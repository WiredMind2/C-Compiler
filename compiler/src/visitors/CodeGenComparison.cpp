#include "CodeGenComparison.h"
#include "../CodeGenVisitor.h"
#include "../type.h"

antlrcpp::Any visitEquals(CodeGenVisitor* visitor, ifccParser::EqualsContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->equality()));
    string right = std::any_cast<string>(visitor->visit(ctx->relational()));
    Type leftType = visitor->getCFG()->getCurrentBB()->get_var_type(left);
    Type rightType = visitor->getCFG()->getCurrentBB()->get_var_type(right);
    if (leftType != rightType) {
        std::cerr << "type not identical. Not supported right now" << std::endl;
        exit(1);
    }
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::cmp_eq, leftType, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitDifferent(CodeGenVisitor* visitor, ifccParser::DifferentContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->equality()));
    string right = std::any_cast<string>(visitor->visit(ctx->relational()));
    Type leftType = visitor->getCFG()->getCurrentBB()->get_var_type(left);
    Type rightType = visitor->getCFG()->getCurrentBB()->get_var_type(right);
    if (leftType != rightType) {
        std::cerr << "type not identical. Not supported right now" << std::endl;
        exit(1);
    }
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::cmp_eq, leftType, {tmp, left, right});
    // TODO: Implement "not equal" - could do XOR with 1 or use cmp_ne if available
    return tmp;
}
