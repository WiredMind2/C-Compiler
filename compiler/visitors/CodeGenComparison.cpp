#include "CodeGenComparison.h"
#include "../CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitEquals(CodeGenVisitor* visitor, ifccParser::EqualsContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->equality()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    string tmp = visitor->getCFG()->create_new_tempvar(INT);
    visitor->getCFG()->current_bb->add_IRInstr(IRInstr::cmp_eq, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitDifferent(CodeGenVisitor* visitor, ifccParser::DifferentContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->equality()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    string tmp = visitor->getCFG()->create_new_tempvar(INT);
    visitor->getCFG()->current_bb->add_IRInstr(IRInstr::cmp_eq, INT, {tmp, left, right});
    // TODO: Implement "not equal" - could do XOR with 1 or use cmp_ne if available
    return tmp;
}
