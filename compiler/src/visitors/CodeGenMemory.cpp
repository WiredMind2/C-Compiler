#include "CodeGenMemory.h"
#include "CodeGenVisitor.h"

antlrcpp::Any visitConstant(CodeGenVisitor* visitor, ifccParser::ConstantContext *ctx)
{
    int64_t val = stol(ctx->CONST()->getText());
    BasicBlock *bb = visitor->getCFG()->current_bb;

    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::INT32, val));
    string tmp = bb->create_new_tempvar(INT);
    bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, IRType::INT32));
    return tmp;
}

antlrcpp::Any visitVariable(CodeGenVisitor* visitor, ifccParser::VariableContext *ctx)
{
    // Return the variable name; callers (arithmetic, return, …) will load it
    return ctx->VAR()->getText();
}

antlrcpp::Any visitDeclaration(CodeGenVisitor* visitor, ifccParser::DeclarationContext *ctx)
{
    for (auto varNode : ctx->VAR())
        visitor->getCFG()->current_bb->add_var_to_symbol_table(varNode->getText(), INT);
    return 0;
}

antlrcpp::Any visitAssignment(CodeGenVisitor* visitor, ifccParser::AssignmentContext *ctx)
{
    string var = ctx->VAR()->getText();
    string src = std::any_cast<string>(visitor->visit(ctx->expr()));
    auto* bb = visitor->getCFG()->current_bb;

    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, src));
    bb->add_IRInstr(new StoreStackInstr(bb, var, Reg::W0));
    return var;
}

antlrcpp::Any visitDeclaration_assignement(CodeGenVisitor* visitor, ifccParser::Declaration_assignementContext *ctx)
{
    string var = ctx->VAR()->getText();
    visitor->getCFG()->current_bb->add_var_to_symbol_table(var, INT);
    string src = std::any_cast<string>(visitor->visit(ctx->expr()));
    auto* bb = visitor->getCFG()->current_bb;

    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, src));
    bb->add_IRInstr(new StoreStackInstr(bb, var, Reg::W0));
    return var;
}
