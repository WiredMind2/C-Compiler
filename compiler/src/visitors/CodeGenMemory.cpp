#include "CodeGenMemory.h"
#include "CodeGenVisitor.h"

antlrcpp::Any visitConstant(CodeGenVisitor* visitor, ifccParser::ConstantContext *ctx)
{
    int64_t val = stol(ctx->CONST()->getText());
    BasicBlock *bb = visitor->getCFG()->current_bb;

    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::INT32, val));
    string tmp = bb->create_new_tempvar(IRType::INT32);
    bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, IRType::INT32));
    return StackParam(tmp, IRType::INT32);
}

antlrcpp::Any visitDouble_constant(CodeGenVisitor* visitor, ifccParser::Double_constantContext *ctx)
{
    double val = stod(ctx->DOUBLE_CONST()->getText());
    BasicBlock *bb = visitor->getCFG()->current_bb;

    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::FLOAT64, val));
    string tmp = bb->create_new_tempvar(IRType::FLOAT64);
    bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, IRType::FLOAT64));
    return StackParam(tmp, IRType::FLOAT64);
}

antlrcpp::Any visitVariable(CodeGenVisitor* visitor, ifccParser::VariableContext *ctx)
{
    string name = ctx->VAR()->getText();
    IRType t = visitor->getCFG()->current_bb->get_var_type(name);
    return StackParam(name, t);
}

antlrcpp::Any visitVar_decl_list(CodeGenVisitor* visitor, ifccParser::Var_decl_listContext *ctx)
{
    // Handle multiple variable declarations: int x, y, z;
    IRType type = irtype_from_string(ctx->type_specifier()->getText());
    for (auto varNode : ctx->VAR())
        visitor->getCFG()->current_bb->add_var_to_symbol_table(varNode->getText(), type);
    return 0;
}

antlrcpp::Any visitVar_decl_with_init(CodeGenVisitor* visitor, ifccParser::Var_decl_with_initContext *ctx)
{
    // Handle declaration with initialization: int x = expr;
    string var = ctx->VAR()->getText();
    IRType type = irtype_from_string(ctx->type_specifier()->getText());
    visitor->getCFG()->current_bb->add_var_to_symbol_table(var, type);

    StackParam src = std::any_cast<StackParam>(visitor->visit(ctx->expr()));
    auto* bb = visitor->getCFG()->current_bb;
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, src.name, type));
    bb->add_IRInstr(new StoreStackInstr(bb, var, Reg::W0, type));
    return StackParam(var, type);
}
antlrcpp::Any visitArray_decl_list(CodeGenVisitor* visitor, ifccParser::Array_decl_listContext *ctx)
{
    IRType type = irtype_from_string(ctx->type_specifier()->getText());
    BasicBlock *bb = visitor->getCFG()->current_bb;
    int i = 0;
    for (auto varNode : ctx->VAR()) {
        int64_t val = stol(ctx->CONST().at(i)->getText());

        bb->add_var_to_symbol_table(varNode->getText(), type);
        int stack_offset = bb->allocate_bytes_on_symbol_table(val * irtype_size(type));

        // Store the address of the stack allocated space on the variable varNode
        bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::INT32, static_cast<int64_t>(stack_offset)));
        bb->add_IRInstr(new StoreStackInstr(bb, varNode->getText(), Reg::W0, IRType::INT32));

        i++;
    }
    return 0;
}

antlrcpp::Any visitArray_decl_with_init(CodeGenVisitor* visitor, ifccParser::Array_decl_with_initContext *ctx)
{
    // TODO
}



antlrcpp::Any visitAssignment(CodeGenVisitor* visitor, ifccParser::AssignmentContext *ctx)
{
    string var = ctx->VAR()->getText();
    StackParam src = std::any_cast<StackParam>(visitor->visit(ctx->compoundAssignment()));
    auto* bb = visitor->getCFG()->current_bb;
    IRType type = bb->get_var_type(var);
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, src.name, type));
    bb->add_IRInstr(new StoreStackInstr(bb, var, Reg::W0, type));
    return StackParam(var, type);
}


antlrcpp::Any visitDereference(CodeGenVisitor* visitor, ifccParser::DereferenceContext *ctx) {
    // TODO
}
antlrcpp::Any visitAddressOf(CodeGenVisitor* visitor, ifccParser::AddressOfContext *ctx) {
    // TODO
}


antlrcpp::Any visitArray_subscript(CodeGenVisitor* visitor, ifccParser::Array_subscriptContext *ctx) {
    // TODO
}
