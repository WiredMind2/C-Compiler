#include "CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx)
{
    for (auto stmt : ctx->statement()) {
        this->visit(stmt);
    }
    return (string)"0";
}

antlrcpp::Any CodeGenVisitor::visitReturn_stmt(ifccParser::Return_stmtContext *ctx)
{
    string var = std::any_cast<string>(this->visit(ctx->expr()));
    cfg->current_bb->add_IRInstr(IRInstr::copy, INT, {"!eax", var});
    return (string)"!eax";
}

antlrcpp::Any CodeGenVisitor::visitExpr(ifccParser::ExprContext *ctx)
{
    return visit(ctx->bitwiseOR());
}

antlrcpp::Any CodeGenVisitor::visitAddition(ifccParser::AdditionContext *ctx)
{
    string left = std::any_cast<string>(this->visit(ctx->additive()));
    string right = std::any_cast<string>(this->visit(ctx->multiplicative()));
    string tmp = cfg->create_new_tempvar(INT);
    cfg->current_bb->add_IRInstr(IRInstr::add, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any CodeGenVisitor::visitSubstraction(ifccParser::SubstractionContext *ctx)
{
    string left = std::any_cast<string>(this->visit(ctx->additive()));
    string right = std::any_cast<string>(this->visit(ctx->multiplicative()));
    string tmp = cfg->create_new_tempvar(INT);
    cfg->current_bb->add_IRInstr(IRInstr::sub, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any CodeGenVisitor::visitMultiplicativeExprRef(ifccParser::MultiplicativeExprRefContext *ctx)
{
    return visit(ctx->multiplicative());
}

antlrcpp::Any CodeGenVisitor::visitMultiplication(ifccParser::MultiplicationContext *ctx)
{
    string left = std::any_cast<string>(this->visit(ctx->multiplicative()));
    string right = std::any_cast<string>(this->visit(ctx->unary()));
    string tmp = cfg->create_new_tempvar(INT);
    cfg->current_bb->add_IRInstr(IRInstr::mul, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any CodeGenVisitor::visitDivision(ifccParser::DivisionContext *ctx)
{
    string left = std::any_cast<string>(this->visit(ctx->multiplicative()));
    string right = std::any_cast<string>(this->visit(ctx->unary()));
    string tmp = cfg->create_new_tempvar(INT);
    cfg->current_bb->add_IRInstr(IRInstr::div, INT, {tmp, left, right});
    return tmp;
}


antlrcpp::Any CodeGenVisitor::visitUnaryPlus(ifccParser::UnaryPlusContext *ctx)
{
    return visit(ctx->primitive());
}

antlrcpp::Any CodeGenVisitor::visitUnaryMinus(ifccParser::UnaryMinusContext *ctx){
    string value = std::any_cast<string>(this->visit(ctx->primitive()));
    string tmp = cfg->create_new_tempvar(INT);
    cfg->current_bb->addIRInstr(IRInstr::sub, INT, {tmp, 0, value});
    return tmp;
}

antlrcpp::Any CodeGenVisitor::visitUnaryNot(ifccParser::UnaryNotContext *ctx){
    string tmp = cfg->create_new_tempvar(INT);
    cfg->current_bb->addIRInstr(IRInstr::bit_not, INT, {tmp, value});
    return tmp;
}

antlrcpp::Any CodeGenVisitor::visitVariable(ifccParser::VariableContext *ctx)
{
    return (string)ctx->VAR()->getText();
}

antlrcpp::Any CodeGenVisitor::visitConstant(ifccParser::ConstantContext *ctx)
{
    string val = ctx->CONST()->getText();
    string tmp = cfg->create_new_tempvar(INT);
    cfg->current_bb->add_IRInstr(IRInstr::ldconst, INT, {tmp, val});
    return tmp;
}

antlrcpp::Any CodeGenVisitor::visitDeclaration(ifccParser::DeclarationContext *ctx)
{
    string var = ctx->VAR()->getText();
    cfg->add_to_symbol_table(var, INT);
    return var;
}

antlrcpp::Any CodeGenVisitor::visitAssignment(ifccParser::AssignmentContext *ctx)
{
    string var = ctx->VAR()->getText();
    string val = std::any_cast<string>(this->visit(ctx->expr()));
    cfg->current_bb->add_IRInstr(IRInstr::copy, INT, {var, val});
    return var;
}

antlrcpp::Any CodeGenVisitor::visitDeclaration_assignement(ifccParser::Declaration_assignementContext *ctx)
{
    string var = ctx->VAR()->getText();
    cfg->add_to_symbol_table(var, INT);
    string val = std::any_cast<string>(this->visit(ctx->expr()));
    cfg->current_bb->add_IRInstr(IRInstr::copy, INT, {var, val});
    return var;
}
