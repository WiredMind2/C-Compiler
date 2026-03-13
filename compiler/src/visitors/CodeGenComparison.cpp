#include "../CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitEquals(CodeGenVisitor *visitor, ifccParser::EqualsContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->equality()));
    string right = std::any_cast<string>(visitor->visit(ctx->relational()));
    string res = visitor->cfg->current_bb->create_new_tempvar(Type::INT);
    visitor->cfg->current_bb->add_IRInstr(IRInstr::cmp_eq, INT, {res, left, right});
    return res;
}

antlrcpp::Any visitDifferent(CodeGenVisitor *visitor, ifccParser::DifferentContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->equality()));
    string right = std::any_cast<string>(visitor->visit(ctx->relational()));
    string res = visitor->cfg->current_bb->create_new_tempvar(Type::INT);
    visitor->cfg->current_bb->add_IRInstr(IRInstr::cmp_ne, INT, {res, left, right});
    return res;
}
