#include "CodeGenArithmetic.h"
#include "CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitAddition(CodeGenVisitor* visitor, ifccParser::AdditionContext *ctx)
{
    StackParam left  = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->additive()));
    StackParam right = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->multiplicative()));
    auto* bb = visitor->getCFG()->current_bb;

    // Special case for Pointer arithmetic
    if (left.type == IRType::POINTER && right.type == IRType::INT32) {
        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, right.name, IRType::INT32));
        int elemSize = irtype_size(IRType::INT32);
        if (bb->cfg->has_array_element_type(left.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(left.name));
        bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
        bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));

        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, left.name, IRType::POINTER));
        bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
        string tmp = bb->create_new_tempvar(IRType::POINTER);
        bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, IRType::POINTER));
        return StackParam(tmp, IRType::POINTER);
    }
    if (left.type == IRType::INT32 && right.type == IRType::POINTER) {
        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, left.name, IRType::INT32));
        int elemSize = irtype_size(IRType::INT32);
        if (bb->cfg->has_array_element_type(right.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(right.name));
        bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
        bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));

        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, right.name, IRType::POINTER));
        bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
        string tmp = bb->create_new_tempvar(IRType::POINTER);
        bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, IRType::POINTER));
        return StackParam(tmp, IRType::POINTER);
    }

    if (left.type != right.type) {
        std::cerr << "type not identical. Not supported right now: " << static_cast<int>(left.type) << " vs " << static_cast<int>(right.type) << std::endl;
        exit(1);
    }
    return bb->emit_binop<AddInstr>(left, right);
}

antlrcpp::Any visitSubstraction(CodeGenVisitor* visitor, ifccParser::SubstractionContext *ctx)
{
    StackParam left  = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->additive()));
    StackParam right = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->multiplicative()));
    auto* bb = visitor->getCFG()->current_bb;

    // Special case for Pointer arithmetic
    if (left.type == IRType::POINTER && right.type == IRType::INT32) {
        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, right.name, IRType::INT32));
        int elemSize = irtype_size(IRType::INT32);
        if (bb->cfg->has_array_element_type(left.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(left.name));
        bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
        bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));

        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, left.name, IRType::POINTER));
        bb->add_IRInstr(new SubInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
        string tmp = bb->create_new_tempvar(IRType::POINTER);
        bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, IRType::POINTER));
        return StackParam(tmp, IRType::POINTER);
    }

    if (left.type != right.type) {
        std::cerr << "type not identical. Not supported right now: " << static_cast<int>(left.type) << " vs " << static_cast<int>(right.type) << std::endl;
        exit(1);
    }
    return bb->emit_binop<SubInstr>(left, right);
}

antlrcpp::Any visitMultiplication(CodeGenVisitor* visitor, ifccParser::MultiplicationContext *ctx)
{
    StackParam left  = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->multiplicative()));
    StackParam right = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->unary()));
    return visitor->getCFG()->current_bb->emit_binop<MulInstr>(left, right);
}

antlrcpp::Any visitDivision(CodeGenVisitor* visitor, ifccParser::DivisionContext *ctx)
{
    StackParam left  = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->multiplicative()));
    StackParam right = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->unary()));
    return visitor->getCFG()->current_bb->emit_binop<DivInstr>(left, right);
}

antlrcpp::Any visitModulo(CodeGenVisitor* visitor, ifccParser::ModuloContext *ctx)
{
    StackParam left  = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->multiplicative()));
    StackParam right = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->unary()));
    return visitor->getCFG()->current_bb->emit_binop<ModInstr>(left, right);
}

antlrcpp::Any visitUnaryPlus(CodeGenVisitor* visitor, ifccParser::UnaryPlusContext *ctx)
{
    return visitor->visit(ctx->unary());
}

antlrcpp::Any visitUnaryMinus(CodeGenVisitor* visitor, ifccParser::UnaryMinusContext *ctx)
{
    StackParam value = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->unary()));
    auto* bb = visitor->getCFG()->current_bb;
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::INT32, static_cast<int64_t>(0)));
    string zname = bb->create_new_tempvar(IRType::INT32);
    bb->add_IRInstr(new StoreStackInstr(bb, zname, Reg::W0, IRType::INT32));
    StackParam zero(zname, IRType::INT32);
    return bb->emit_binop<SubInstr>(zero, value);
}
