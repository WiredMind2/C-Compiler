#include "CodeGenArithmetic.h"
#include "CodeGenVisitor.h"

antlrcpp::Any visitAddition(CodeGenVisitor* visitor, ifccParser::AdditionContext *ctx)
{
    string left  = std::any_cast<string>(visitor->visit(ctx->additive()));
    string right = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    return visitor->getCFG()->current_bb->emit_binop<AddInstr>(left, right);
}

antlrcpp::Any visitSubstraction(CodeGenVisitor* visitor, ifccParser::SubstractionContext *ctx)
{
    string left  = std::any_cast<string>(visitor->visit(ctx->additive()));
    string right = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    return visitor->getCFG()->current_bb->emit_binop<SubInstr>(left, right);
}

antlrcpp::Any visitMultiplication(CodeGenVisitor* visitor, ifccParser::MultiplicationContext *ctx)
{
    string left  = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    string right = std::any_cast<string>(visitor->visit(ctx->unary()));
    return visitor->getCFG()->current_bb->emit_binop<MulInstr>(left, right);
}

antlrcpp::Any visitDivision(CodeGenVisitor* visitor, ifccParser::DivisionContext *ctx)
{
    string left  = std::any_cast<string>(visitor->visit(ctx->multiplicative()));
    string right = std::any_cast<string>(visitor->visit(ctx->unary()));
    return visitor->getCFG()->current_bb->emit_binop<DivInstr>(left, right);
}

antlrcpp::Any visitUnaryPlus(CodeGenVisitor* visitor, ifccParser::UnaryPlusContext *ctx)
{
    return visitor->visit(ctx->primitive());
}

antlrcpp::Any visitUnaryMinus(CodeGenVisitor* visitor, ifccParser::UnaryMinusContext *ctx)
{
    string value = std::any_cast<string>(visitor->visit(ctx->primitive()));
    // Negate: sub(0, value)
    auto* bb = visitor->getCFG()->current_bb;
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::INT32, static_cast<int64_t>(0)));
    string zero = bb->create_new_tempvar(INT);
    bb->add_IRInstr(new StoreStackInstr(bb, zero, Reg::W0, IRType::INT32));
    return bb->emit_binop<SubInstr>(zero, value);
}
