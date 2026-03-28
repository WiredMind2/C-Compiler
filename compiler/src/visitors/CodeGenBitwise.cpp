#include "CodeGenBitwise.h"
#include "CodeGenVisitor.h"

antlrcpp::Any visitBitwiseORRule(CodeGenVisitor* visitor, ifccParser::BitwiseORRuleContext *ctx)
{
    StackParam left  = std::any_cast<StackParam>(visitor->visit(ctx->bitwiseOR()));
    StackParam right = std::any_cast<StackParam>(visitor->visit(ctx->bitwiseXOR()));
    return visitor->getCFG()->current_bb->emit_binop<BitOrInstr>(left, right);
}

antlrcpp::Any visitBitwiseXORRule(CodeGenVisitor* visitor, ifccParser::BitwiseXORRuleContext *ctx)
{
    StackParam left  = std::any_cast<StackParam>(visitor->visit(ctx->bitwiseXOR()));
    StackParam right = std::any_cast<StackParam>(visitor->visit(ctx->bitwiseAND()));
    return visitor->getCFG()->current_bb->emit_binop<BitXorInstr>(left, right);
}

antlrcpp::Any visitBitwiseANDRule(CodeGenVisitor* visitor, ifccParser::BitwiseANDRuleContext *ctx)
{
    StackParam left  = std::any_cast<StackParam>(visitor->visit(ctx->bitwiseAND()));
    StackParam right = std::any_cast<StackParam>(visitor->visit(ctx->additive()));
    return visitor->getCFG()->current_bb->emit_binop<BitAndInstr>(left, right);
}

antlrcpp::Any visitUnaryNot(CodeGenVisitor* visitor, ifccParser::UnaryNotContext *ctx)
{
    StackParam value = std::any_cast<StackParam>(visitor->visit(ctx->unary()));
    return visitor->getCFG()->current_bb->emit_unop<BitNotInstr>(value);
}
