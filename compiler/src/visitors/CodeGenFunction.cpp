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

antlrcpp::Any visitFunctionCall(CodeGenVisitor* visitor, ifccParser::Function_callContext *ctx)
{
    std::string func_name = ctx->VAR()->getText();

    // Check that the function has been declared or defined
    if (visitor->getCFG()->get_function(func_name) == nullptr) {
        std::cerr << "Error: implicit declaration of function '" << func_name << "'" << std::endl;
        exit(1);
    }

    auto* bb = visitor->getCFG()->current_bb;

    static const Reg argRegs[] = {
        Reg::ARG0, Reg::ARG1, Reg::ARG2,
        Reg::ARG3, Reg::ARG4, Reg::ARG5
    };

    // Evaluate each argument, load into ARGn register
    auto args = ctx->expr();
    std::vector<Reg> usedArgRegs;
    for (int i = 0; i < (int)args.size() && i < 6; i++) {
        StackParam argVar = std::any_cast<StackParam>(visitor->visit(args[i]));
        bb->add_IRInstr(new LoadStackInstr(bb, argRegs[i], argVar.name, argVar.type));
        usedArgRegs.push_back(argRegs[i]);
    }

    // Result stored in RET (W0), then saved to a temp stack slot
    std::string resultTmp = bb->create_new_tempvar(IRType::INT32);
    bb->add_IRInstr(new CallInstr(bb, func_name, Reg::RET, usedArgRegs));
    bb->add_IRInstr(new StoreStackInstr(bb, resultTmp, Reg::RET));

    return StackParam(resultTmp, IRType::INT32);
}
