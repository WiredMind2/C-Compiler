#include "CodeGenComparison.h"
#include "CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitEquals(CodeGenVisitor* visitor, ifccParser::EqualsContext *ctx)
{
    StackParam left  = std::any_cast<StackParam>(visitor->visit(ctx->equality()));
    StackParam right = std::any_cast<StackParam>(visitor->visit(ctx->relational()));
    if (left.type != right.type) {
        std::cerr << "type not identical. Not supported right now" << std::endl;
        exit(1);
    }
    return visitor->getCFG()->current_bb->emit_cmp_binop<CmpEqInstr>(left, right);
}

antlrcpp::Any visitDifferent(CodeGenVisitor* visitor, ifccParser::DifferentContext *ctx)
{
    StackParam left  = std::any_cast<StackParam>(visitor->visit(ctx->equality()));
    StackParam right = std::any_cast<StackParam>(visitor->visit(ctx->relational()));
    if (left.type != right.type) {
        std::cerr << "type not identical. Not supported right now" << std::endl;
        exit(1);
    }
    auto* bb = visitor->getCFG()->current_bb;
    StackParam eq = bb->emit_cmp_binop<CmpEqInstr>(left, right);
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::INT32, static_cast<int64_t>(1)));
    string oname = bb->create_new_tempvar(IRType::INT32);
    bb->add_IRInstr(new StoreStackInstr(bb, oname, Reg::W0, IRType::INT32));
    StackParam one(oname, IRType::INT32);
    return bb->emit_binop<BitXorInstr>(eq, one);
}
