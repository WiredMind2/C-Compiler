#include "CodeGenMemory.h"
#include "../CodeGenVisitor.h"

antlrcpp::Any visitConstant(CodeGenVisitor* visitor, ifccParser::ConstantContext *ctx)
{
    string val = ctx->CONST()->getText();
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::ldconst, INT, {tmp, val});
    return tmp;
}

antlrcpp::Any visitVariable(CodeGenVisitor* visitor, ifccParser::VariableContext *ctx)
{
    string var = ctx->VAR()->getText();
    // Create a temporary and copy the variable into it
    // This ensures that the variable is accessible in the current basic block's context
    // if it needs to be used by a terminator during assembly generation.
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::copy, INT, {tmp, var});
    return tmp;
}

antlrcpp::Any visitDeclaration_list(CodeGenVisitor* visitor, ifccParser::Declaration_listContext *ctx)
{
    // Handle multiple variable declarations: int x, y, z;
    for (auto varCtx : ctx->VAR()) {
        string var = varCtx->getText();
        visitor->getCFG()->getCurrentBB()->add_var_to_symbol_table(var, INT);
    }
    return 0;
}

antlrcpp::Any visitVar_decl(CodeGenVisitor* visitor, ifccParser::Var_declContext *ctx)
{
    // Handle single variable declaration: int x;
    string var = ctx->VAR()->getText();
    visitor->getCFG()->getCurrentBB()->add_var_to_symbol_table(var, INT);
    return 0;
}

antlrcpp::Any visitVar_decl_with_init(CodeGenVisitor* visitor, ifccParser::Var_decl_with_initContext *ctx)
{
    // Handle declaration with initialization: int x = expr;
    string var = ctx->VAR()->getText();
    visitor->getCFG()->getCurrentBB()->add_var_to_symbol_table(var, INT);
    string val = std::any_cast<string>(visitor->visit(ctx->expr()));
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::copy, INT, {var, val});
    return var;
}

antlrcpp::Any visitAssignment(CodeGenVisitor* visitor, ifccParser::AssignmentContext *ctx)
{
    string var = ctx->VAR()->getText();
    string val = std::any_cast<string>(visitor->visit(ctx->compoundAssignment()));
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::copy, INT, {var, val});
    return var;
}
