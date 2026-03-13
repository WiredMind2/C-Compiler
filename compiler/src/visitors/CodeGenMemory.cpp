#include "CodeGenMemory.h"
#include "../CodeGenVisitor.h"

antlrcpp::Any visitConstant(CodeGenVisitor* visitor, ifccParser::ConstantContext *ctx) {
    string val = ctx->CONST()->getText();
    string tmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(Type::INT);
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::ldconst, INT, {tmp, val});
    return tmp;
}

antlrcpp::Any visitVariable(CodeGenVisitor* visitor, ifccParser::VariableContext *ctx) {
    string var = ctx->VAR()->getText();
    return var;
}

antlrcpp::Any visitDeclaration_list(CodeGenVisitor* visitor, ifccParser::Declaration_listContext *ctx) {
    for (auto varCtx : ctx->VAR()) {
        string var = varCtx->getText();
        BasicBlock* entryBB = visitor->getCFG()->getBBs()[0]; // Fallback
        if (!visitor->getCFG()->get_functions().empty()) {
            entryBB = visitor->getCFG()->get_functions().back().entryBB;
        }
        entryBB->add_var_to_symbol_table(var, INT);
    }
    return 0;
}

antlrcpp::Any visitVar_decl(CodeGenVisitor* visitor, ifccParser::Var_declContext *ctx) {
    string var = ctx->VAR()->getText();
    BasicBlock* entryBB = visitor->getCFG()->getBBs()[0]; // Fallback
    if (!visitor->getCFG()->get_functions().empty()) {
        entryBB = visitor->getCFG()->get_functions().back().entryBB;
    }
    entryBB->add_var_to_symbol_table(var, INT);
    return 0;
}

antlrcpp::Any visitVar_decl_with_init(CodeGenVisitor* visitor, ifccParser::Var_decl_with_initContext *ctx) {
    string var = ctx->VAR()->getText();
    BasicBlock* entryBB = visitor->getCFG()->getBBs()[0]; // Fallback
    if (!visitor->getCFG()->get_functions().empty()) {
        entryBB = visitor->getCFG()->get_functions().back().entryBB;
    }
    entryBB->add_var_to_symbol_table(var, INT);
    string val = std::any_cast<string>(visitor->visit(ctx->expr()));
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::copy, INT, {var, val});
    return var;
}

antlrcpp::Any visitAssignment(CodeGenVisitor* visitor, ifccParser::AssignmentContext *ctx) {
    string var = ctx->VAR()->getText();
    string val = std::any_cast<string>(visitor->visit(ctx->compoundAssignment()));
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::copy, INT, {var, val});
    return var;
}
