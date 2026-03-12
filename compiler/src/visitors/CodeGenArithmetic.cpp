#include "CodeGenArithmetic.h"
#include "../CodeGenVisitor.h"
#include "../type.h"
#include <iostream>

antlrcpp::Any visitAddition(CodeGenVisitor* visitor, ifccParser::AdditionContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->additive()));
    string right = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    Type leftType = visitor->getCFG()->getCurrentBB()->get_var_type(left);
    Type rightType = visitor->getCFG()->getCurrentBB()->get_var_type(right);
    if (leftType != rightType) {
        std::cerr << "type not identical. Not supported right now" << std::endl;
        exit(1);
    }
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(leftType);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::add, leftType, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitSubstraction(CodeGenVisitor* visitor, ifccParser::SubstractionContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->additive()));
    string right = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    Type leftType = visitor->getCFG()->getCurrentBB()->get_var_type(left);
    Type rightType = visitor->getCFG()->getCurrentBB()->get_var_type(right);
    if (leftType != rightType) {
        std::cerr << "type not identical. Not supported right now" << std::endl;
        exit(1);
    }    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(leftType);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::sub, leftType, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitMultiplication(CodeGenVisitor* visitor, ifccParser::MultiplicationContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    string right = std::any_cast<string>(visitor->visit(ctx->unary()));
    Type leftType = visitor->getCFG()->getCurrentBB()->get_var_type(left);
    Type rightType = visitor->getCFG()->getCurrentBB()->get_var_type(right);
    if (leftType != rightType) {
        std::cerr << "type not identical. Not supported right now" << std::endl;
        exit(1);
    }    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(leftType);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::mul, leftType, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitDivision(CodeGenVisitor* visitor, ifccParser::DivisionContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    string right = std::any_cast<string>(visitor->visit(ctx->unary()));
    Type leftType = visitor->getCFG()->getCurrentBB()->get_var_type(left);
    Type rightType = visitor->getCFG()->getCurrentBB()->get_var_type(right);
    if (leftType != rightType) {
        std::cerr << "type not identical. Not supported right now" << std::endl;
        exit(1);
    }    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(leftType);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::div, leftType, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitModulo(CodeGenVisitor* visitor, ifccParser::ModuloContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    string right = std::any_cast<string>(visitor->visit(ctx->unary()));
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::cmp_mod, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitUnaryPlus(CodeGenVisitor* visitor, ifccParser::UnaryPlusContext *ctx)
{
    return visitor->visit(ctx->primitive());
}

antlrcpp::Any visitUnaryMinus(CodeGenVisitor* visitor, ifccParser::UnaryMinusContext *ctx)
{
    string value = std::any_cast<string>(visitor->visit(ctx->primitive()));
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::sub, INT, {tmp, "0", value});
    return tmp;
}
