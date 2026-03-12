#include "CodeGenComparison.h"
#include "CodeGenVisitor.h"

antlrcpp::Any visitEquals(CodeGenVisitor* visitor, ifccParser::EqualsContext *ctx)
{
    string left  = std::any_cast<string>(visitor->visit(ctx->equality()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    return visitor->getCFG()->current_bb->emit_binop<CmpEqInstr>(left, right);
}

antlrcpp::Any visitDifferent(CodeGenVisitor* visitor, ifccParser::DifferentContext *ctx)
{
    string left  = std::any_cast<string>(visitor->visit(ctx->equality()));
    string right = std::any_cast<string>(visitor->visit(ctx->additive()));
    auto* bb = visitor->getCFG()->current_bb;
    // cmp_eq, then XOR result with 1 to negate
    string eq  = bb->emit_binop<CmpEqInstr>(left, right);
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0_32, TypedConst(IRType::INT32, static_cast<int64_t>(1))));
    string one = bb->create_new_tempvar(INT);
    bb->add_IRInstr(new StoreStackInstr(bb, one, Reg::W0_32, IRType::INT32));
    return bb->emit_binop<BitXorInstr>(eq, one);
}
