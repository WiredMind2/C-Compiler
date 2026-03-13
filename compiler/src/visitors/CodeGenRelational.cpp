#include "../CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitSmallerStrictThan(CodeGenVisitor* visitor, ifccParser::SmallerStrictThanContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->relational()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    string res = visitor->getCFG()->getCurrentBB()->create_new_tempvar(Type::INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::cmp_lt, INT, {res, left, right});
    return res;
}

antlrcpp::Any visitGreaterStrictThan(CodeGenVisitor* visitor, ifccParser::GreaterStrictThanContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->relational()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    string res = visitor->getCFG()->getCurrentBB()->create_new_tempvar(Type::INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::cmp_gt, INT, {res, left, right});
    return res;
}

antlrcpp::Any visitSmallerThan(CodeGenVisitor* visitor, ifccParser::SmallerThanContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->relational()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    string res = visitor->getCFG()->getCurrentBB()->create_new_tempvar(Type::INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::cmp_le, INT, {res, left, right});
    return res;
}

antlrcpp::Any visitGreaterThan(CodeGenVisitor* visitor, ifccParser::GreaterThanContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->relational()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    string res = visitor->getCFG()->getCurrentBB()->create_new_tempvar(Type::INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::cmp_ge, INT, {res, left, right});
    return res;
}

antlrcpp::Any visitRelationalExprRef(CodeGenVisitor* visitor, ifccParser::RelationalExprRefContext *ctx)
{
    return visitor->visit(ctx->additive());
}
