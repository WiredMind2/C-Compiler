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
    return visitor->getCFG()->current_bb->emit_unop<BitNotInstr>(value);
}
