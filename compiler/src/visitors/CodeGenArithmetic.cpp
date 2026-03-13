#include "CodeGenArithmetic.h"
#include "../CodeGenVisitor.h"

antlrcpp::Any visitAddition(CodeGenVisitor* visitor, ifccParser::AdditionContext *ctx) 
{
    string left = std::any_cast<string>(visitor->visit(ctx->additive()));
    string right = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(Type::INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::add, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitSubstraction(CodeGenVisitor* visitor, ifccParser::SubstractionContext *ctx) 
{
    string left = std::any_cast<string>(visitor->visit(ctx->additive()));
    string right = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(Type::INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::sub, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitMultiplication(CodeGenVisitor* visitor, ifccParser::MultiplicationContext *ctx) 
{
    string left = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    string right = std::any_cast<string>(visitor->visit(ctx->unary()));
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(Type::INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::mul, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitDivision(CodeGenVisitor* visitor, ifccParser::DivisionContext *ctx) 
{
    string left = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    string right = std::any_cast<string>(visitor->visit(ctx->unary()));
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(Type::INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::div, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitModulo(CodeGenVisitor* visitor, ifccParser::ModuloContext *ctx) 
{
    string left = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    string right = std::any_cast<string>(visitor->visit(ctx->unary()));
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(Type::INT);
    // Basic implementation of modulo if IR supports it, else need workaround
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::cmp_mod, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitUnaryMinus(CodeGenVisitor* visitor, ifccParser::UnaryMinusContext *ctx) 
{
    string val = std::any_cast<string>(visitor->visit(ctx->primitive()));
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(Type::INT);
    // 0 - val
    string zero = visitor->getCFG()->getCurrentBB()->create_new_tempvar(Type::INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::ldconst, INT, {zero, "0"});
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::sub, INT, {tmp, zero, val});
    return tmp;
}

antlrcpp::Any visitUnaryPlus(CodeGenVisitor* visitor, ifccParser::UnaryPlusContext *ctx) 
{
    return visitor->visit(ctx->primitive());
}

antlrcpp::Any visitMultiplicativeExprRef(CodeGenVisitor* visitor, ifccParser::MultiplicativeExprRefContext *ctx) 
{
    return visitor->visit(ctx->multiplicative());
}

antlrcpp::Any visitUnaryExprRef(CodeGenVisitor* visitor, ifccParser::UnaryExprRefContext *ctx) 
{
    return visitor->visit(ctx->unary());
}
