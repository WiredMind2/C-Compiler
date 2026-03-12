//
// Created by dupic on 06/03/2026.
//

#include "CodeGenFunction.h"
#include "CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitFunction_definition(CodeGenVisitor* visitor, ifccParser::Function_definitionContext *ctx)
{
    std::string func_name = ctx->VAR()->getText();
    std::vector<Type> paramTypes;
    std::vector<std::string> paramNames;

    for (auto param : ctx->var_declarations_function()) {
        paramTypes.push_back(INT);
        paramNames.push_back(param->VAR()->getText());
    }

    visitor->getCFG()->create_function_entry(func_name, INT, paramTypes, paramNames);
    if (ctx->scope())
        visitor->visit(ctx->scope());
    return 0;
}

antlrcpp::Any visitFunction_declaration(CodeGenVisitor* visitor, ifccParser::Function_declarationContext *ctx)
{
    std::string func_name = ctx->VAR()->getText();
    std::vector<Type> paramTypes;
    std::vector<std::string> paramNames;

    for (auto param : ctx->var_declarations_function()) {
        paramTypes.push_back(INT);
        paramNames.push_back(param->VAR()->getText());
    }

    visitor->getCFG()->add_function(func_name, INT, paramTypes, paramNames);
    return 0;
}

antlrcpp::Any visitFunctionCall(CodeGenVisitor* visitor, ifccParser::Function_callContext *ctx)
{
    std::string func_name = ctx->VAR()->getText();
    auto* bb = visitor->getCFG()->current_bb;

    // Argument registers in order
    static const Reg argRegs[] = {
        Reg::ARG0_32, Reg::ARG1_32, Reg::ARG2_32,
        Reg::ARG3_32, Reg::ARG4_32, Reg::ARG5_32
    };

    // Evaluate each argument expression (returns a stack var name),
    // then load it into the matching ARGn register
    auto args = ctx->expr();
    std::vector<Reg> usedArgRegs;
    for (int i = 0; i < (int)args.size() && i < 6; i++) {
        std::string argVar = std::any_cast<std::string>(visitor->visit(args[i]));
        bb->add_IRInstr(new LoadStackInstr(bb, argRegs[i], argVar));
        usedArgRegs.push_back(argRegs[i]);
    }

    // Result goes into RET_32 (W0_32 / %eax)
    std::string resultTmp = bb->create_new_tempvar(INT);
    bb->add_IRInstr(new CallInstr(bb, func_name, Reg::RET_32, usedArgRegs));
    // Store return value from RET_32 → stack slot
    bb->add_IRInstr(new StoreStackInstr(bb, resultTmp, Reg::RET_32));

    return resultTmp;
}
