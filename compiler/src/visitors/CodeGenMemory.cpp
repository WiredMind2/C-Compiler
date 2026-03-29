#include "CodeGenMemory.h"

#include "CodeGenVisitor.h"

namespace {

StackParam load_var_value(CodeGenVisitor* visitor, const string& var, IRType type) {
    auto* bb = visitor->getCFG()->current_bb;
    string tmp = bb->create_new_tempvar(type);
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, var, type));
    bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, type));
    return StackParam(tmp, type);
}

StackParam assign_to_var(CodeGenVisitor* visitor, const string& var, IRType varType, const StackParam& src) {
    auto* bb = visitor->getCFG()->current_bb;
    if (src.type != varType) {
        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, src.name, src.type));
        bb->generate_conversion_instruction(Reg::W0, src.type, Reg::W1, varType);
        bb->add_IRInstr(new StoreStackInstr(bb, var, Reg::W1, varType));
    } else {
        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, src.name, varType));
        bb->add_IRInstr(new StoreStackInstr(bb, var, Reg::W0, varType));
    }
    return StackParam(var, varType);
}

StackParam one_of_type(CodeGenVisitor* visitor, IRType type) {
    auto* bb = visitor->getCFG()->current_bb;
    if (type == IRType::INT8) {
        bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::INT8, static_cast<int64_t>(1)));
        string oneName = bb->create_new_tempvar(IRType::INT8);
        bb->add_IRInstr(new StoreStackInstr(bb, oneName, Reg::W0, IRType::INT8));
        return StackParam(oneName, IRType::INT8);
    }

    if (type == IRType::FLOAT64) {
        bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::FLOAT64, 1.0));
        string oneName = bb->create_new_tempvar(IRType::FLOAT64);
        bb->add_IRInstr(new StoreStackInstr(bb, oneName, Reg::W0, IRType::FLOAT64));
        return StackParam(oneName, IRType::FLOAT64);
    }

    // Default to INT32 for unsupported integer-like cases.
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::INT32, static_cast<int64_t>(1)));
    string oneName = bb->create_new_tempvar(IRType::INT32);
    bb->add_IRInstr(new StoreStackInstr(bb, oneName, Reg::W0, IRType::INT32));
    return StackParam(oneName, IRType::INT32);
}

template<class BinInstr>
StackParam apply_compound(CodeGenVisitor* visitor, const string& var, const StackParam& rhs) {
    auto* bb = visitor->getCFG()->current_bb;
    IRType varType = bb->get_var_type(var);
    StackParam lhs(var, varType);
    StackParam value = bb->emit_binop<BinInstr>(lhs, rhs);
    return assign_to_var(visitor, var, varType, value);
}

} // namespace

antlrcpp::Any visitConstant(CodeGenVisitor* visitor, ifccParser::ConstantContext* ctx) {
    int64_t val = stol(ctx->CONST()->getText());
    BasicBlock* bb = visitor->getCFG()->current_bb;

    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::INT32, val));
    string tmp = bb->create_new_tempvar(IRType::INT32);
    bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, IRType::INT32));
    return StackParam(tmp, IRType::INT32);
}

antlrcpp::Any visitDouble_constant(CodeGenVisitor* visitor, ifccParser::Double_constantContext* ctx) {
    double val = stod(ctx->DOUBLE_CONST()->getText());
    BasicBlock* bb = visitor->getCFG()->current_bb;

    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::FLOAT64, val));
    string tmp = bb->create_new_tempvar(IRType::FLOAT64);
    bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, IRType::FLOAT64));
    return StackParam(tmp, IRType::FLOAT64);
}

antlrcpp::Any visitChar_constant(CodeGenVisitor* visitor, ifccParser::Char_constantContext *ctx)
{
    string text = ctx->CHAR_CONST()->getText(); // e.g. 'a' or '\n'
    int8_t val;
    if (text[1] == '\\') {
        switch (text[2]) {
            case 'n':  val = '\n'; break;
            case 't':  val = '\t'; break;
            case 'r':  val = '\r'; break;
            case '0':  val = '\0'; break;
            case '\\': val = '\\'; break;
            case '\'': val = '\''; break;
            case 'a':  val = '\a'; break;
            case 'b':  val = '\b'; break;
            case 'f':  val = '\f'; break;
            case 'v':  val = '\v'; break;
            default:   val = static_cast<int8_t>(text[2]); break;
        }
    } else {
        val = static_cast<int8_t>(text[1]);
    }
    BasicBlock *bb = visitor->getCFG()->current_bb;

    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::INT8, static_cast<int64_t>(val)));
    string tmp = bb->create_new_tempvar(IRType::INT8);
    bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, IRType::INT8));
    return StackParam(tmp, IRType::INT8);
}

antlrcpp::Any visitVariable(CodeGenVisitor* visitor, ifccParser::VariableContext *ctx)
{
    string name = ctx->VAR()->getText();
    IRType t = visitor->getCFG()->current_bb->get_var_type(name);
    return StackParam(name, t);
}

antlrcpp::Any visitDeclaration(CodeGenVisitor* visitor, ifccParser::DeclarationContext* ctx) {
    IRType type = irtype_from_string(ctx->type_specifier()->getText());
    auto* bb = visitor->getCFG()->current_bb;
    for (auto assignement_ctx : ctx->declaration_instance()) {
        string var = assignement_ctx->VAR()->getText();

        bb->add_var_to_symbol_table(var, type);

        if (assignement_ctx->expr()) {
            StackParam src = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(assignement_ctx->expr()));
            assign_to_var(visitor, var, type, src);
        }
    }
    return nullptr;
}

antlrcpp::Any visitAssignment(CodeGenVisitor* visitor, ifccParser::AssignmentContext *ctx) {
    auto* bb = visitor->getCFG()->current_bb;
    string var = ctx->VAR()->getText();
    IRType varType = bb->get_var_type(var);

    StackParam src = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->compoundAssignment()));
    return assign_to_var(visitor, var, varType, src);
}

antlrcpp::Any visitAddAssignment(CodeGenVisitor* visitor, ifccParser::AddAssignmentContext *ctx) {
    string var = ctx->VAR()->getText();
    StackParam rhs = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->compoundAssignment()));
    return apply_compound<AddInstr>(visitor, var, rhs);
}

antlrcpp::Any visitSubAssignment(CodeGenVisitor* visitor, ifccParser::SubAssignmentContext *ctx) {
    string var = ctx->VAR()->getText();
    StackParam rhs = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->compoundAssignment()));
    return apply_compound<SubInstr>(visitor, var, rhs);
}

antlrcpp::Any visitPreIncrement(CodeGenVisitor* visitor, ifccParser::PreIncrementContext *ctx) {
    auto* bb = visitor->getCFG()->current_bb;
    string var = ctx->VAR()->getText();
    IRType varType = bb->get_var_type(var);
    StackParam one = one_of_type(visitor, varType);
    return apply_compound<AddInstr>(visitor, var, one);
}

antlrcpp::Any visitPreDecrement(CodeGenVisitor* visitor, ifccParser::PreDecrementContext *ctx) {
    auto* bb = visitor->getCFG()->current_bb;
    string var = ctx->VAR()->getText();
    IRType varType = bb->get_var_type(var);
    StackParam one = one_of_type(visitor, varType);
    return apply_compound<SubInstr>(visitor, var, one);
}

antlrcpp::Any visitPostIncrement(CodeGenVisitor* visitor, ifccParser::PostIncrementContext *ctx) {
    auto* bb = visitor->getCFG()->current_bb;
    string var = ctx->VAR()->getText();
    IRType varType = bb->get_var_type(var);

    StackParam oldValue = load_var_value(visitor, var, varType);
    StackParam one = one_of_type(visitor, varType);
    apply_compound<AddInstr>(visitor, var, one);
    return oldValue;
}

antlrcpp::Any visitPostDecrement(CodeGenVisitor* visitor, ifccParser::PostDecrementContext *ctx) {
    auto* bb = visitor->getCFG()->current_bb;
    string var = ctx->VAR()->getText();
    IRType varType = bb->get_var_type(var);

    StackParam oldValue = load_var_value(visitor, var, varType);
    StackParam one = one_of_type(visitor, varType);
    apply_compound<SubInstr>(visitor, var, one);
    return oldValue;
}


