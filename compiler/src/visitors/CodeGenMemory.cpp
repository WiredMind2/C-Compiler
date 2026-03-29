#include "CodeGenMemory.h"

#include "CodeGenVisitor.h"

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
            auto* bb = visitor->getCFG()->current_bb;

            if (src.type != type) {
                bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, src.name, src.type));
                bb->generate_conversion_instruction(Reg::W0, src.type, Reg::W1, type);
                bb->add_IRInstr(new StoreStackInstr(bb, var, Reg::W1, type));
            } else {
                bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, src.name, type));
                bb->add_IRInstr(new StoreStackInstr(bb, var, Reg::W0, type));
            }

            bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, src.name, type));
            bb->add_IRInstr(new StoreStackInstr(bb, var, Reg::W0, type));
        }
    }
    return nullptr;
}

antlrcpp::Any visitAssignment(CodeGenVisitor* visitor, ifccParser::AssignmentContext *ctx) {
    auto* bb = visitor->getCFG()->current_bb;
    string var = ctx->VAR()->getText();
    IRType varType = bb->get_var_type(var);

    StackParam src = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(ctx->compoundAssignment()));
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, src.name, varType));
    bb->add_IRInstr(new StoreStackInstr(bb, var, Reg::W0, varType));

    return new StackParam(var, varType);
}


