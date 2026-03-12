#include "CodeGenRelational.h"
#include "../CodeGenVisitor.h"
#include "../type.h"

antlrcpp::Any visitSmallerStrictThan(CodeGenVisitor* visitor, ifccParser::SmallerStrictThanContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->relational()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    Type leftType = visitor->getCFG()->getCurrentBB()->get_var_type(left);
    Type rightType = visitor->getCFG()->getCurrentBB()->get_var_type(right);
    if (leftType != rightType) {
        std::cerr << "type not identical. Not supported right now" << std::endl;
        exit(1);
    }
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::cmp_lt, leftType, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitGreaterStrictThan(CodeGenVisitor* visitor, ifccParser::GreaterStrictThanContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->relational()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    Type leftType = visitor->getCFG()->getCurrentBB()->get_var_type(left);
    Type rightType = visitor->getCFG()->getCurrentBB()->get_var_type(right);
    if (leftType != rightType) {
        std::cerr << "type not identical. Not supported right now" << std::endl;
        exit(1);
    }
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::cmp_gt, leftType, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitSmallerThan(CodeGenVisitor* visitor, ifccParser::SmallerThanContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->relational()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    Type leftType = visitor->getCFG()->getCurrentBB()->get_var_type(left);
    Type rightType = visitor->getCFG()->getCurrentBB()->get_var_type(right);
    if (leftType != rightType) {
        std::cerr << "type not identical. Not supported right now" << std::endl;
        exit(1);
    }    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::cmp_le, leftType, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitGreaterThan(CodeGenVisitor* visitor, ifccParser::GreaterThanContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->relational()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    Type leftType = visitor->getCFG()->getCurrentBB()->get_var_type(left);
    Type rightType = visitor->getCFG()->getCurrentBB()->get_var_type(right);
    if (leftType != rightType) {
        std::cerr << "type not identical. Not supported right now" << std::endl;
        exit(1);
    }    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::cmp_ge, leftType, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitRelationalExprRef(CodeGenVisitor* visitor, ifccParser::RelationalExprRefContext *ctx)
{
    return visitor->visit(ctx->additive());
}
