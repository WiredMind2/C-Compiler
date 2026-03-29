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
    StackParam value = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->unary()));
    return visitor->getCFG()->current_bb->emit_unop<BitNotInstr>(value);
}

antlrcpp::Any visitUnaryBitNot(CodeGenVisitor* visitor, ifccParser::UnaryBitNotContext *ctx)
{
    StackParam value = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->unary()));
    return visitor->getCFG()->current_bb->emit_unop<BitNotInstr>(value);
}

antlrcpp::Any visitShiftExprRef(CodeGenVisitor* visitor, ifccParser::ShiftExprRefContext *ctx)
{
    return visitor->visit(ctx->additive());
}

antlrcpp::Any visitShiftLeft(CodeGenVisitor* visitor, ifccParser::ShiftLeftContext *ctx)
{
    StackParam left  = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->shift()));
    StackParam right = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->additive()));
    return visitor->getCFG()->current_bb->emit_binop<ShlInstr>(left, right);
}

antlrcpp::Any visitShiftRight(CodeGenVisitor* visitor, ifccParser::ShiftRightContext *ctx)
{
    StackParam left  = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->shift()));
    StackParam right = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->additive()));
    return visitor->getCFG()->current_bb->emit_binop<ShrInstr>(left, right);
}
