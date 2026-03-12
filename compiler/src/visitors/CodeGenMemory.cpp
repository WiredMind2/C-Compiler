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
    return (string)ctx->VAR()->getText();
}

antlrcpp::Any visitDouble_constant(CodeGenVisitor* visitor, ifccParser::Double_constantContext *ctx)
{
    string val = ctx->DOUBLE_CONST()->getText();
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(DOUBLE);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::ldconst, DOUBLE, {tmp, val});
    return tmp;
}

antlrcpp::Any visitDeclaration_list(CodeGenVisitor* visitor, ifccParser::Declaration_listContext *ctx)
{
    // Handle multiple variable declarations: int x, y, z;
    Type type = type_from_string(ctx->type_specifier()->getText());
    for (auto varCtx : ctx->VAR()) {
        string var = varCtx->getText();
        visitor->getCFG()->getCurrentBB()->add_var_to_symbol_table(var, type);
    }
    return 0;
}

antlrcpp::Any visitVar_decl(CodeGenVisitor* visitor, ifccParser::Var_declContext *ctx)
{
    // Handle single variable declaration: int x;
    string var = ctx->VAR()->getText();
    Type type = type_from_string(ctx->type_specifier()->getText());
    visitor->getCFG()->getCurrentBB()->add_var_to_symbol_table(var, type);
    return 0;
}

antlrcpp::Any visitVar_decl_with_init(CodeGenVisitor* visitor, ifccParser::Var_decl_with_initContext *ctx)
{
    // Handle declaration with initialization: int x = expr;
    string var = ctx->VAR()->getText();
    Type type = type_from_string(ctx->type_specifier()->getText());
    visitor->getCFG()->getCurrentBB()->add_var_to_symbol_table(var, type);
    string val = std::any_cast<string>(visitor->visit(ctx->expr()));
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::copy, type, {var, val});
    return var;
}

antlrcpp::Any visitAssignment(CodeGenVisitor* visitor, ifccParser::AssignmentContext *ctx)
{
    string var = ctx->VAR()->getText();
    string val = std::any_cast<string>(visitor->visit(ctx->compoundAssignment()));
    Type type  =  visitor->getCFG()->getCurrentBB()->get_var_type(var);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::copy, type, {var, val});
    return var;
}
