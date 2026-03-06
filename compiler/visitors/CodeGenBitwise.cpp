#include "CodeGenBitwise.h"
#include "../CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitBitwiseORRule(CodeGenVisitor* visitor, ifccParser::BitwiseORRuleContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->bitwiseOR()));
    string right = std::any_cast<string>(visitor->visit(ctx->bitwiseXOR()));
    string tmp = visitor->getCFG()->create_new_tempvar(INT);
    visitor->getCFG()->current_bb->add_IRInstr(IRInstr::bit_xor, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitBitwiseXORRule(CodeGenVisitor* visitor, ifccParser::BitwiseXORRuleContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->bitwiseXOR()));
    string right = std::any_cast<string>(visitor->visit(ctx->bitwiseAND()));
    string tmp = visitor->getCFG()->create_new_tempvar(INT);
    visitor->getCFG()->current_bb->add_IRInstr(IRInstr::bit_xor, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitBitwiseANDRule(CodeGenVisitor* visitor, ifccParser::BitwiseANDRuleContext *ctx)
{
    string left = std::any_cast<string>(visitor->visit(ctx->bitwiseAND()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    string tmp = visitor->getCFG()->create_new_tempvar(INT);
    visitor->getCFG()->current_bb->add_IRInstr(IRInstr::bit_and, INT, {tmp, left, right});
    return tmp;
}

antlrcpp::Any visitUnaryNot(CodeGenVisitor* visitor, ifccParser::UnaryNotContext *ctx)
{
    string value = std::any_cast<string>(visitor->visit(ctx->primitive()));
    string tmp = visitor->getCFG()->create_new_tempvar(INT);
    visitor->getCFG()->current_bb->add_IRInstr(IRInstr::bit_not, INT, {tmp, value});
    return tmp;
}
