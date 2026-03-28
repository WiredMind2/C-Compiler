//
// Created by dupic on 06/03/2026.
//

#include "CodeGenFunction.h"
#include "CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitFunction_definition(CodeGenVisitor* visitor, ifccParser::Function_definitionContext *ctx)
{
    std::string func_name = ctx->VAR()->getText();

    std::vector<IRType> paramTypes;
    std::vector<std::string> paramNames;

    if (ctx->param_list()) {
        for (auto param : ctx->param_list()->param()) {
            paramTypes.push_back(IRType::INT32);
            paramNames.push_back(param->VAR()->getText());
        }
    }

    // Save and restore the former BB around the function body
    BasicBlock* formerBB = visitor->getCFG()->current_bb;

    BasicBlock* entryBB = visitor->getCFG()->create_function_entry(
        func_name, IRType::INT32, paramTypes, paramNames);

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
            paramTypes.push_back(IRType::INT32);
            paramNames.push_back(param->VAR()->getText());
        }
    }

    visitor->getCFG()->add_function(func_name, IRType::INT32, paramTypes, paramNames);
    return 0;
}

antlrcpp::Any visitFunctionCall(CodeGenVisitor* visitor, ifccParser::Function_callContext *ctx) {
    std::string func_name = ctx->VAR()->getText();

    auto funcInfo = visitor->getCFG()->get_function(func_name);

    // Handle standard library functions when not previously declared.
    if (funcInfo == nullptr) {
        if (isFunctionStandardLibrary(visitor, func_name, ctx)) {
            visitor->getCFG()->add_function(
                func_name,
                IRType::INT32,
                std::vector<IRType>(ctx->expr().size(), IRType::INT32),
                std::vector<std::string>());
            funcInfo = visitor->getCFG()->get_function(func_name);
        } else {
            std::cerr << "Error: call to undeclared function '" << func_name << "'." << std::endl;
            exit(1);
        }
    }

    // Check call arity and argument types against declaration/definition.
    int expectedArgs = static_cast<int>(funcInfo->paramTypes.size());
    int actualArgs = static_cast<int>(ctx->expr().size());
    if (actualArgs > expectedArgs) {
        std::cerr << "Error: too many arguments in call to '" << func_name << "'. Expected " << expectedArgs << ", got " << actualArgs << "." << std::endl;
        exit(1);
    }
    for (int i = 0; i < actualArgs; i++) {
        StackParam arg = std::any_cast<StackParam>(visitor->visit(ctx->expr(i)));
        if (arg.type != funcInfo->paramTypes[i]) {
            std::cerr << "Error: argument " << (i + 1) << " of '" << func_name << "' has incompatible type. Expected "
                      << irtype_name(funcInfo->paramTypes[i]) << ", got " << irtype_name(arg.type) << "." << std::endl;
            exit(1);
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
    for (int i = 0; i < static_cast<int>(args.size()) && i < 6; i++) {
        StackParam argVar = std::any_cast<StackParam>(visitor->visit(args[i]));
        bb->add_IRInstr(new LoadStackInstr(bb, argRegs[i], argVar.name, argVar.type));
        usedArgRegs.push_back(argRegs[i]);
    }

    // Result stored in RET, then saved to a temp stack slot.
    std::string resultTmp = bb->create_new_tempvar(IRType::INT32);
    bb->add_IRInstr(new CallInstr(bb, func_name, Reg::RET, usedArgRegs));
    bb->add_IRInstr(new StoreStackInstr(bb, resultTmp, Reg::RET));

    return StackParam(resultTmp, IRType::INT32);
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
            return ctx->expr().empty();

        case StandardLibraryFunction::PUTCHAR:
        case StandardLibraryFunction::MALLOC: {
            if (ctx->expr().size() != 1) {
                return 0;
            }

            StackParam arg = std::any_cast<StackParam>(visitor->visit(ctx->expr(0)));
            return arg.type == IRType::INT32;
        }

        default:
            return 0;
    }
}