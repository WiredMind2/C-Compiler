#include "CodeGenArithmetic.h"
#include "../CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitAddition(CodeGenVisitor* visitor, ifccParser::AdditionContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->additive()));
    string right = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    string tmp = visitor->getCFG()->create_new_tempvar(INT);
    visitor->getCFG()->current_bb->add_IRInstr(IRInstr::add, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitSubstraction(CodeGenVisitor* visitor, ifccParser::SubstractionContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->additive()));
    string right = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    string tmp = visitor->getCFG()->create_new_tempvar(INT);
    visitor->getCFG()->current_bb->add_IRInstr(IRInstr::sub, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitMultiplication(CodeGenVisitor* visitor, ifccParser::MultiplicationContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    string right = std::any_cast<string>(visitor->visit(ctx->unary()));
    string tmp = visitor->getCFG()->create_new_tempvar(INT);
    visitor->getCFG()->current_bb->add_IRInstr(IRInstr::mul, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitDivision(CodeGenVisitor* visitor, ifccParser::DivisionContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    string right = std::any_cast<string>(visitor->visit(ctx->unary()));
    string tmp = visitor->getCFG()->create_new_tempvar(INT);
    visitor->getCFG()->current_bb->add_IRInstr(IRInstr::div, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitUnaryPlus(CodeGenVisitor* visitor, ifccParser::UnaryPlusContext *ctx)
{
    return visitor->visit(ctx->primitive());
}

antlrcpp::Any visitUnaryMinus(CodeGenVisitor* visitor, ifccParser::UnaryMinusContext *ctx)
{
    string value = std::any_cast<string>(visitor->visit(ctx->primitive()));
    string tmp = visitor->getCFG()->create_new_tempvar(INT);
    visitor->getCFG()->current_bb->add_IRInstr(IRInstr::sub, INT, {tmp, "0", value});
    return tmp;
}
