#include "../CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitBitwiseORRule(CodeGenVisitor *visitor, ifccParser::BitwiseORRuleContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->bitwiseOR()));
    string right = std::any_cast<string>(visitor->visit(ctx->bitwiseXOR()));
    string res = visitor->cfg->current_bb->create_new_tempvar(Type::INT);
    visitor->cfg->current_bb->add_IRInstr(IRInstr::bit_or, INT, {res, left, right});
    return res;
}

antlrcpp::Any visitBitwiseXORRule(CodeGenVisitor *visitor, ifccParser::BitwiseXORRuleContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->bitwiseXOR()));
    string right = std::any_cast<string>(visitor->visit(ctx->bitwiseAND()));
    string res = visitor->cfg->current_bb->create_new_tempvar(Type::INT);
    visitor->cfg->current_bb->add_IRInstr(IRInstr::bit_xor, INT, {res, left, right});
    return res;
}

antlrcpp::Any visitBitwiseANDRule(CodeGenVisitor *visitor, ifccParser::BitwiseANDRuleContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->bitwiseAND()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    string res = visitor->cfg->current_bb->create_new_tempvar(Type::INT);
    visitor->cfg->current_bb->add_IRInstr(IRInstr::bit_and, INT, {res, left, right});
    return res;
}

antlrcpp::Any visitUnaryNot(CodeGenVisitor *visitor, ifccParser::UnaryNotContext *ctx)
{
    string value = std::any_cast<string>(visitor->visit(ctx->primitive()));
    string res = visitor->cfg->current_bb->create_new_tempvar(Type::INT);
    visitor->cfg->current_bb->add_IRInstr(IRInstr::bit_not, INT, {res, value});
    return res;
}
