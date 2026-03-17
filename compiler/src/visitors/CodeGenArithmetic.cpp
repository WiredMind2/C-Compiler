#include "CodeGenArithmetic.h"
#include "CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitAddition(CodeGenVisitor* visitor, ifccParser::AdditionContext *ctx)
{
    StackParam left  = std::any_cast<StackParam>(visitor->visit(ctx->additive()));
    StackParam right = std::any_cast<StackParam>(visitor->visit(ctx->multiplicative()));
    return visitor->getCFG()->current_bb->emit_binop<AddInstr>(left, right);
}

antlrcpp::Any visitSubstraction(CodeGenVisitor* visitor, ifccParser::SubstractionContext *ctx)
{
    StackParam left  = std::any_cast<StackParam>(visitor->visit(ctx->additive()));
    StackParam right = std::any_cast<StackParam>(visitor->visit(ctx->multiplicative()));
    return visitor->getCFG()->current_bb->emit_binop<SubInstr>(left, right);
}

antlrcpp::Any visitMultiplication(CodeGenVisitor* visitor, ifccParser::MultiplicationContext *ctx)
{
    StackParam left  = std::any_cast<StackParam>(visitor->visit(ctx->multiplicative()));
    StackParam right = std::any_cast<StackParam>(visitor->visit(ctx->unary()));
    return visitor->getCFG()->current_bb->emit_binop<MulInstr>(left, right);
}

antlrcpp::Any visitDivision(CodeGenVisitor* visitor, ifccParser::DivisionContext *ctx)
{
    StackParam left  = std::any_cast<StackParam>(visitor->visit(ctx->multiplicative()));
    StackParam right = std::any_cast<StackParam>(visitor->visit(ctx->unary()));
    return visitor->getCFG()->current_bb->emit_binop<DivInstr>(left, right);
}

antlrcpp::Any visitModulo(CodeGenVisitor* visitor, ifccParser::ModuloContext *ctx)
{
    StackParam left  = std::any_cast<StackParam>(visitor->visit(ctx->multiplicative()));
    StackParam right = std::any_cast<StackParam>(visitor->visit(ctx->unary()));
    return visitor->getCFG()->current_bb->emit_binop<ModInstr>(left, right);
}

antlrcpp::Any visitUnaryPlus(CodeGenVisitor* visitor, ifccParser::UnaryPlusContext *ctx)
{
    return visitor->visit(ctx->primitive());
}

antlrcpp::Any visitUnaryMinus(CodeGenVisitor* visitor, ifccParser::UnaryMinusContext *ctx)
{
    StackParam value = std::any_cast<StackParam>(visitor->visit(ctx->primitive()));
    auto* bb = visitor->getCFG()->current_bb;
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::INT32, static_cast<int64_t>(0)));
    string zname = bb->create_new_tempvar(IRType::INT32);
    bb->add_IRInstr(new StoreStackInstr(bb, zname, Reg::W0, IRType::INT32));
    StackParam zero(zname, IRType::INT32);
    return bb->emit_binop<SubInstr>(zero, value);
}
