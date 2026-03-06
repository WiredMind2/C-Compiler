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

antlrcpp::Any CodeGenVisitor::visitBitwiseORRule(ifccParser::BitwiseORRuleContext *ctx)
{
    string left = std::any_cast<string>(this->visit(ctx->bitwiseOR()));
    string right = std::any_cast<string>(this->visit(ctx->bitwiseXOR()));
    string tmp = cfg->create_new_tempvar(INT);
    cfg->current_bb->add_IRInstr(IRInstr::bit_xor, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any CodeGenVisitor::visitBitwiseXORRule(ifccParser::BitwiseXORRuleContext *ctx)
{
    string left = std::any_cast<string>(this->visit(ctx->bitwiseXOR()));
    string right = std::any_cast<string>(this->visit(ctx->bitwiseAND()));
    string tmp = cfg->create_new_tempvar(INT);
    cfg->current_bb->add_IRInstr(IRInstr::bit_xor, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any CodeGenVisitor::visitBitwiseANDRule(ifccParser::BitwiseANDRuleContext *ctx)
{
    string left = std::any_cast<string>(this->visit(ctx->bitwiseAND()));
    string right = std::any_cast<string>(this->visit(ctx->additive()));
    string tmp = cfg->create_new_tempvar(INT);
    cfg->current_bb->add_IRInstr(IRInstr::bit_and, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any CodeGenVisitor::visitEquals(ifccParser::EqualsContext *ctx)
{
    string left = std::any_cast<string>(this->visit(ctx->equality()));
    string right = std::any_cast<string>(this->visit(ctx->additive()));
    string tmp = cfg->create_new_tempvar(INT);
    cfg->current_bb->add_IRInstr(IRInstr::cmp_eq, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any CodeGenVisitor::visitDifferent(ifccParser::DifferentContext *ctx)
{
    string left = std::any_cast<string>(this->visit(ctx->equality()));
    string right = std::any_cast<string>(this->visit(ctx->additive()));
    string tmp = cfg->create_new_tempvar(INT);
    cfg->current_bb->add_IRInstr(IRInstr::cmp_eq, INT, {tmp, left, right});
    // TODO: Implement "not equal" - could do XOR with 1 or use cmp_ne if available
    return tmp;
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
    cfg->current_bb->add_IRInstr(IRInstr::sub, INT, {tmp, "0", value});
    return tmp;
}

antlrcpp::Any CodeGenVisitor::visitUnaryNot(ifccParser::UnaryNotContext *ctx){
    string value = std::any_cast<string>(this->visit(ctx->primitive()));
    string tmp = cfg->create_new_tempvar(INT);
    cfg->current_bb->add_IRInstr(IRInstr::bit_not, INT, {tmp, value});
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
    // Variable already declared in DeclarationVisitor, just get its offset
    int offset = symbolTable->getOffset("main", var);
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
    // Variable already declared in DeclarationVisitor, just get its offset
    int offset = symbolTable->getOffset("main", var);
    string val = std::any_cast<string>(this->visit(ctx->expr()));
    cfg->current_bb->add_IRInstr(IRInstr::copy, INT, {var, val});
    return var;
}
