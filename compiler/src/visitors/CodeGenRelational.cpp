#include "CodeGenRelational.h"
#include "../CodeGenVisitor.h"

antlrcpp::Any visitSmallerStrictThan(CodeGenVisitor* visitor, ifccParser::SmallerStrictThanContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->relational()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::cmp_lt, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitGreaterStrictThan(CodeGenVisitor* visitor, ifccParser::GreaterStrictThanContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->relational()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::cmp_gt, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitSmallerThan(CodeGenVisitor* visitor, ifccParser::SmallerThanContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->relational()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::cmp_le, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitGreaterThan(CodeGenVisitor* visitor, ifccParser::GreaterThanContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->relational()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::cmp_ge, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitRelationalExprRef(CodeGenVisitor* visitor, ifccParser::RelationalExprRefContext *ctx)
{
    return visitor->visit(ctx->additive());
}
