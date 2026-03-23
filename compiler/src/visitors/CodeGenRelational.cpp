#include "CodeGenRelational.h"
#include "CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitSmallerStrictThan(CodeGenVisitor* visitor, ifccParser::SmallerStrictThanContext *ctx)
{
    StackParam left  = std::any_cast<StackParam>(visitor->visit(ctx->relational()));
    StackParam right = std::any_cast<StackParam>(visitor->visit(ctx->additive()));
    return visitor->getCFG()->current_bb->emit_cmp_binop<CmpLtInstr>(left, right);
}

antlrcpp::Any visitGreaterStrictThan(CodeGenVisitor* visitor, ifccParser::GreaterStrictThanContext *ctx)
{
    StackParam left  = std::any_cast<StackParam>(visitor->visit(ctx->relational()));
    StackParam right = std::any_cast<StackParam>(visitor->visit(ctx->additive()));
    return visitor->getCFG()->current_bb->emit_cmp_binop<CmpGtInstr>(left, right);
}

antlrcpp::Any visitSmallerThan(CodeGenVisitor* visitor, ifccParser::SmallerThanContext *ctx)
{
    StackParam left  = std::any_cast<StackParam>(visitor->visit(ctx->relational()));
    StackParam right = std::any_cast<StackParam>(visitor->visit(ctx->additive()));
    return visitor->getCFG()->current_bb->emit_cmp_binop<CmpLeInstr>(left, right);
}

antlrcpp::Any visitGreaterThan(CodeGenVisitor* visitor, ifccParser::GreaterThanContext *ctx)
{
    StackParam left  = std::any_cast<StackParam>(visitor->visit(ctx->relational()));
    StackParam right = std::any_cast<StackParam>(visitor->visit(ctx->additive()));
    return visitor->getCFG()->current_bb->emit_cmp_binop<CmpGeInstr>(left, right);
}

antlrcpp::Any visitRelationalExprRef(CodeGenVisitor* visitor, ifccParser::RelationalExprRefContext *ctx)
{
    return visitor->visit(ctx->additive());
}
