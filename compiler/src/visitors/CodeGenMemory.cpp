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

antlrcpp::Any visitChar_constant(CodeGenVisitor* visitor, ifccParser::Char_constantContext *ctx)
{
    // Parse character literal: 'x' or '\n', '\t', '\\', '\'' etc.
    std::string text = ctx->CHAR_CONST()->getText();
    // text includes the surrounding quotes, e.g. "'A'" or "'\n'"
    int64_t val = 0;
    if (text.size() >= 3 && text[1] == '\\') {
        // Escape sequence
        switch (text[2]) {
            case 'n':  val = '\n'; break;
            case 't':  val = '\t'; break;
            case 'r':  val = '\r'; break;
            case '0':  val = '\0'; break;
            case '\\': val = '\\'; break;
            case '\'': val = '\''; break;
            case '"':  val = '"';  break;
            default:   val = text[2]; break;
        }
    } else if (text.size() >= 3) {
        val = (unsigned char)text[1];
    }

    BasicBlock *bb = visitor->getCFG()->current_bb;
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::INT32, val));
    string tmp = bb->create_new_tempvar(IRType::INT32);
    bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, IRType::INT32));
    return StackParam(tmp, IRType::INT32);
}


{
    string name = ctx->VAR()->getText();
    IRType t = visitor->getCFG()->current_bb->get_var_type(name);
    return StackParam(name, t);
}

antlrcpp::Any visitDeclaration_list(CodeGenVisitor* visitor, ifccParser::Declaration_listContext *ctx)
{
    // Handle multiple variable declarations: int x, y, z;
    IRType type = irtype_from_string(ctx->type_specifier()->getText());
    for (auto varNode : ctx->VAR())
        visitor->getCFG()->current_bb->add_var_to_symbol_table(varNode->getText(), type);
    return 0;
}

antlrcpp::Any visitVar_decl(CodeGenVisitor* visitor, ifccParser::Var_declContext *ctx)
{
    // Handle single variable declaration: int x;
    string var = ctx->VAR()->getText();
    IRType type = irtype_from_string(ctx->type_specifier()->getText());
    visitor->getCFG()->current_bb->add_var_to_symbol_table(var, type);
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
