#include "CodeGenLogical.h"
#include "CodeGenVisitor.h"

antlrcpp::Any visitLogicalORRule(CodeGenVisitor* visitor, ifccParser::LogicalORRuleContext *ctx)
{
    StackParam left  = std::any_cast<StackParam>(visitor->visit(ctx->logicalOR()));
    StackParam right = std::any_cast<StackParam>(visitor->visit(ctx->logicalAND()));
    return visitor->getCFG()->current_bb->emit_binop<LogicalOrInstr>(left, right);
}

antlrcpp::Any visitLogicalANDRule(CodeGenVisitor* visitor, ifccParser::LogicalANDRuleContext *ctx)
{
    StackParam left  = std::any_cast<StackParam>(visitor->visit(ctx->logicalAND()));
    StackParam right = std::any_cast<StackParam>(visitor->visit(ctx->bitwiseOR()));
    return visitor->getCFG()->current_bb->emit_binop<LogicalAndInstr>(left, right);
}

antlrcpp::Any visitLogicalORRef(CodeGenVisitor* visitor, ifccParser::LogicalORRefContext *ctx)
{
    return visitor->visit(ctx->logicalAND());
}

antlrcpp::Any visitLogicalANDRef(CodeGenVisitor* visitor, ifccParser::LogicalANDRefContext *ctx)
{
    return visitor->visit(ctx->bitwiseOR());
}
