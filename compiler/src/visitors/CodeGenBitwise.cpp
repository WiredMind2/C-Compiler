#include "CodeGenBitwise.h"
#include "CodeGenVisitor.h"

antlrcpp::Any visitBitwiseORRule(CodeGenVisitor* visitor, ifccParser::BitwiseORRuleContext *ctx)
{
    StackParam left  = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->bitwiseOR()));
    StackParam right = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->bitwiseXOR()));
    return visitor->getCFG()->current_bb->emit_binop<BitOrInstr>(left, right);
}

antlrcpp::Any visitBitwiseXORRule(CodeGenVisitor* visitor, ifccParser::BitwiseXORRuleContext *ctx)
{
    StackParam left  = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->bitwiseXOR()));
    StackParam right = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->bitwiseAND()));
    return visitor->getCFG()->current_bb->emit_binop<BitXorInstr>(left, right);
}

antlrcpp::Any visitBitwiseANDRule(CodeGenVisitor* visitor, ifccParser::BitwiseANDRuleContext *ctx)
{
    StackParam left  = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->bitwiseAND()));
    StackParam right = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->additive()));
    return visitor->getCFG()->current_bb->emit_binop<BitAndInstr>(left, right);
}

antlrcpp::Any visitUnaryNot(CodeGenVisitor* visitor, ifccParser::UnaryNotContext *ctx)
{
    StackParam value = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->primitive()));
    auto* bb = visitor->getCFG()->current_bb;
    // !x  <=>  (x == 0)
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, value.type, static_cast<int64_t>(0)));
    string zname = bb->create_new_tempvar(value.type);
    bb->add_IRInstr(new StoreStackInstr(bb, zname, Reg::W0, value.type));
    StackParam zero(zname, value.type);
    return bb->emit_cmp_binop<CmpEqInstr>(value, zero);
}
