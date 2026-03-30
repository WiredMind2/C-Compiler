#include "CodeGenRelational.h"

#include <iostream>

#include "CodeGenVisitor.h"

antlrcpp::Any visitSmallerStrictThan(CodeGenVisitor* visitor, ifccParser::SmallerStrictThanContext* ctx) {
    StackParam left = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->relational()));
    StackParam right = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->shift()));
    return visitor->getCFG()->current_bb->emit_cmp_binop<CmpLtInstr>(left, right);
}

antlrcpp::Any visitGreaterStrictThan(CodeGenVisitor* visitor, ifccParser::GreaterStrictThanContext* ctx) {
    StackParam left = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->relational()));
    StackParam right = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->shift()));
    return visitor->getCFG()->current_bb->emit_cmp_binop<CmpGtInstr>(left, right);
}

antlrcpp::Any visitSmallerThan(CodeGenVisitor* visitor, ifccParser::SmallerThanContext* ctx) {
    StackParam left = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->relational()));
    StackParam right = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->shift()));
    return visitor->getCFG()->current_bb->emit_cmp_binop<CmpLeInstr>(left, right);
}

antlrcpp::Any visitGreaterThan(CodeGenVisitor* visitor, ifccParser::GreaterThanContext* ctx) {
    StackParam left = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->relational()));
    StackParam right = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->shift()));
    return visitor->getCFG()->current_bb->emit_cmp_binop<CmpGeInstr>(left, right);
}

antlrcpp::Any visitRelationalExprRef(CodeGenVisitor* visitor, ifccParser::RelationalExprRefContext* ctx) { return visitor->visit(ctx->shift()); }
