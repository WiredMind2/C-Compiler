#include "CodeGenLogical.h"
#include "../CodeGenVisitor.h"

antlrcpp::Any visitLogicalORRule(CodeGenVisitor* visitor, ifccParser::LogicalORRuleContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->logicalOR()));
    string right = std::any_cast<string>(visitor->visit(ctx->logicalAND()));
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::logical_or, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitLogicalANDRule(CodeGenVisitor* visitor, ifccParser::LogicalANDRuleContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->logicalAND()));
    string right = std::any_cast<string>(visitor->visit(ctx->bitwiseOR()));
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::logical_and, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitLogicalORRef(CodeGenVisitor* visitor, ifccParser::LogicalORRefContext *ctx)
{
    return visitor->visit(ctx->logicalAND());
}

antlrcpp::Any visitLogicalANDRef(CodeGenVisitor* visitor, ifccParser::LogicalANDRefContext *ctx)
{
    return visitor->visit(ctx->bitwiseOR());
}
