//
// Created by dupic on 06/03/2026.
//

#include "CodeGenFunction.h"
#include "CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitFunction_definition(CodeGenVisitor* visitor, ifccParser::Function_definitionContext *ctx)
{
    IRType return_type = irtype_from_string(ctx->type_specifier()->getText());
    std::string func_name = ctx->VAR()->getText();

    std::vector<IRType> paramTypes;
    std::vector<std::string> paramNames;

    if (ctx->param_list()) {
        for (auto param : ctx->param_list()->param()) {
            IRType type = irtype_from_string(param->type_specifier()->getText());
            paramTypes.push_back(type);
            paramNames.push_back(param->VAR()->getText());
        }
    }

    // Save and restore the former BB around the function body
    BasicBlock* formerBB = visitor->getCFG()->current_bb;

    BasicBlock* entryBB = visitor->getCFG()->create_function_entry(
        func_name, return_type, paramTypes, paramNames);

    // Push the entry BB onto the scope stack
    visitor->getCFG()->getStackBBs().push_back(entryBB);

    if (ctx->scope())
        visitor->visit(ctx->scope());

    visitor->getCFG()->getStackBBs().pop_back();
    visitor->getCFG()->current_bb = formerBB;

    return 0;
}

antlrcpp::Any visitFunction_declaration(CodeGenVisitor* visitor, ifccParser::Function_declarationContext *ctx)
{
    // TODO : handle types
    std::string func_name = ctx->VAR()->getText();

    std::vector<IRType> paramTypes;
    std::vector<std::string> paramNames;

    if (ctx->param_list()) {
        for (auto param : ctx->param_list()->param()) {
            IRType type = irtype_from_string(param->type_specifier()->getText());
            paramTypes.push_back(type);
            paramNames.push_back(param->VAR()->getText());
        }
    }

    visitor->getCFG()->add_function(func_name, IRType::INT32, paramTypes, paramNames);
    return 0;
}

antlrcpp::Any visitFunctionCall(CodeGenVisitor* visitor, ifccParser::Function_callContext *ctx) {
    std::string func_name = ctx->VAR()->getText();

    auto function_signature = visitor->getCFG()->get_function(func_name);

    // Handle standard library functions when not previously declared.
    if (function_signature == nullptr) {
        if (isFunctionStandardLibrary(visitor, func_name, ctx)) {
            visitor->getCFG()->add_function(
                func_name,
                IRType::INT32,
                std::vector<IRType>(ctx->expr().size(), IRType::INT32),
                std::vector<std::string>());
            function_signature = visitor->getCFG()->get_function(func_name);
        } else {
            std::cerr << "Error: call to undeclared function '" << func_name << "'." << std::endl;
            exit(1);
        }
    }

    // Check call arity and argument types against declaration/definition.
    int expectedArgs = static_cast<int>(function_signature->paramTypes.size());
    int actualArgs = static_cast<int>(ctx->expr().size());
    if (actualArgs > expectedArgs) {
        std::cerr << "Error: too many arguments in call to '" << func_name << "'. Expected " << expectedArgs << ", got " << actualArgs << "." << std::endl;
        exit(1);
    }

    auto isIntegral = [](IRType t) {
        return t == IRType::INT8 || t == IRType::INT32 || t == IRType::INT64;
    };
    auto isFloat = [](IRType t) {
        return t == IRType::FLOAT32 || t == IRType::FLOAT64;
    };

    for (int i = 0; i < actualArgs; i++) {
        StackParam arg = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->expr(i)));
        IRType expected = function_signature->paramTypes[i];
        IRType actual = arg.type;
        if (actual != expected) {
            // Allow implicit numeric conversions (integral promotions / float promotions)
            if (!(isIntegral(actual) && isIntegral(expected)) && !(isFloat(actual) && isFloat(expected))) {
                std::cerr << "Error: argument " << (i + 1) << " of '" << func_name << "' has incompatible type. Expected "
                          << irtype_name(expected) << ", got " << irtype_name(actual) << "." << std::endl;
                exit(1);
            }
            // otherwise compatible via implicit conversion
        }
    }

    auto* bb = visitor->getCFG()->current_bb;

    static const Reg argRegs[] = {
        Reg::ARG0, Reg::ARG1, Reg::ARG2,
        Reg::ARG3, Reg::ARG4, Reg::ARG5
    };

    // Evaluate each argument, load into ARGn register.
    auto args = ctx->expr();
    std::vector<Reg> usedArgRegs;
    std::vector<IRType> usedArgTypes;
    for (int i = 0; i < static_cast<int>(args.size()) && i < 6; i++) {
        StackParam argVar = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(args[i]));
        bb->add_IRInstr(new LoadStackInstr(bb, argRegs[i], argVar.name, argVar.type));
        // Insert conversion to expected parameter type when needed (e.g. char -> int)
        IRType expectedType = IRType::INT32;
        if (function_signature && static_cast<int>(function_signature->paramTypes.size()) > i) {
            expectedType = function_signature->paramTypes[i];
        }
        if (argVar.type != expectedType) {
            bb->generate_conversion_instruction(argRegs[i], argVar.type, argRegs[i], expectedType);
        }
        usedArgRegs.push_back(argRegs[i]);
        usedArgTypes.push_back(expectedType);
    }

    // Generate the call instruction
    bb->add_IRInstr(new CallInstr(bb, func_name, Reg::RET, usedArgRegs, usedArgTypes));

    // Get the signature again (maybe it was added above because it is a standard library function)
    func_name = ctx->VAR()->getText();
    function_signature = visitor->getCFG()->get_function(func_name);

    if (function_signature->returnType == IRType::VOID) {
        return {};
    }

    // Result stored in RET, then saved to a temp stack slot.
    std::string resultTmp = bb->create_new_tempvar(function_signature->returnType);
    bb->add_IRInstr(new StoreStackInstr(bb, resultTmp, Reg::RET, function_signature->returnType));

    return StackParam(resultTmp, function_signature->returnType);
}


StandardLibraryFunction getStandardLibraryFunction(const std::string& func_name) {
    if (func_name == "getchar") {
        return StandardLibraryFunction::GETCHAR;
    } else if (func_name == "putchar") {
        return StandardLibraryFunction::PUTCHAR;
    } else if (func_name == "malloc") {
        return StandardLibraryFunction::MALLOC;
    } else {
        return StandardLibraryFunction::UNKNOWN;
    }
}

int isFunctionStandardLibrary(CodeGenVisitor* visitor, const std::string& func_name, ifccParser::Function_callContext *ctx)
{
    switch (getStandardLibraryFunction(func_name)) {
        case StandardLibraryFunction::GETCHAR:
            // Only consider getchar as standard if <stdio.h> was included
            if (!visitor->has_stdio()) return 0;
            return ctx->expr().empty();

        case StandardLibraryFunction::PUTCHAR: {
            if (ctx->expr().size() != 1) {
                return 0;
            }

            StackParam arg = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->expr(0)));
            // Accept both int and char (integral promotion to int allowed)
            return arg.type == IRType::INT32 || arg.type == IRType::INT8;
        }

        case StandardLibraryFunction::MALLOC: {
            // Only consider malloc as standard if <stdlib.h> was included
            if (!visitor->has_stdlib()) return 0;
            if (ctx->expr().size() != 1) {
                return 0;
            }

            StackParam arg = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->expr(0)));
            return arg.type == IRType::INT32;
        }

        default:
            return 0;
    }
}