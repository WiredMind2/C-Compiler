#include "CodeGenBitwise.h"
#include "CodeGenVisitor.h"

antlrcpp::Any visitBitwiseORRule(CodeGenVisitor* visitor, ifccParser::BitwiseORRuleContext *ctx)
{
    string left  = std::any_cast<string>(visitor->visit(ctx->bitwiseOR()));
    string right = std::any_cast<string>(visitor->visit(ctx->bitwiseXOR()));
    return visitor->getCFG()->current_bb->emit_binop<BitOrInstr>(left, right);
}

antlrcpp::Any visitBitwiseXORRule(CodeGenVisitor* visitor, ifccParser::BitwiseXORRuleContext *ctx)
{
    string left  = std::any_cast<string>(visitor->visit(ctx->bitwiseXOR()));
    string right = std::any_cast<string>(visitor->visit(ctx->bitwiseAND()));
    return visitor->getCFG()->current_bb->emit_binop<BitXorInstr>(left, right);
}

antlrcpp::Any visitBitwiseANDRule(CodeGenVisitor* visitor, ifccParser::BitwiseANDRuleContext *ctx)
{
    string left  = std::any_cast<string>(visitor->visit(ctx->bitwiseAND()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    return visitor->getCFG()->current_bb->emit_binop<BitAndInstr>(left, right);
}

antlrcpp::Any visitUnaryNot(CodeGenVisitor* visitor, ifccParser::UnaryNotContext *ctx)
{
    string value = std::any_cast<string>(visitor->visit(ctx->primitive()));
    return visitor->getCFG()->current_bb->emit_unop<BitNotInstr>(value);
}
