#include "CodeGenComparison.h"
#include "CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitEquals(CodeGenVisitor* visitor, ifccParser::EqualsContext *ctx)
{
    StackParam left  = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->equality()));
    StackParam right = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->relational()));
    return visitor->getCFG()->current_bb->emit_cmp_binop<CmpEqInstr>(left, right);
}

antlrcpp::Any visitDifferent(CodeGenVisitor* visitor, ifccParser::DifferentContext *ctx)
{
    StackParam left  = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->equality()));
    StackParam right = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->relational()));
    auto* bb = visitor->getCFG()->current_bb;
    StackParam eq = bb->emit_cmp_binop<CmpEqInstr>(left, right);
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::INT32, static_cast<int64_t>(1)));
    string oname = bb->create_new_tempvar(IRType::INT32);
    bb->add_IRInstr(new StoreStackInstr(bb, oname, Reg::W0, IRType::INT32));
    StackParam one(oname, IRType::INT32);
    return bb->emit_binop<BitXorInstr>(eq, one);
}
