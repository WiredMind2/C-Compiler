#include "CodeGenMemory.h"
#include "../CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitConstant(CodeGenVisitor* visitor, ifccParser::ConstantContext *ctx)
{
    string val = ctx->CONST()->getText();
    string tmp = visitor->getCFG()->create_new_tempvar(INT);
    visitor->getCFG()->current_bb->add_IRInstr(IRInstr::ldconst, INT, {tmp, val});
    return tmp;
}

antlrcpp::Any visitVariable(CodeGenVisitor* visitor, ifccParser::VariableContext *ctx)
{
    return (string)ctx->VAR()->getText();
}

antlrcpp::Any visitDeclaration(CodeGenVisitor* visitor, ifccParser::DeclarationContext *ctx)
{
    // With the new grammar, VAR returns a vector of TerminalNodes
    for (auto varNode : ctx->VAR()) {
        string var = varNode->getText();
        // Variable already declared in DeclarationVisitor, get its offset
        int offset = visitor->getSymbolTable()->getOffset("main", var);
        // Add to CFG's symbol table for code generation
        visitor->getCFG()->add_to_symbol_table(var, INT);
    }
    return 0;
}

antlrcpp::Any visitAssignment(CodeGenVisitor* visitor, ifccParser::AssignmentContext *ctx)
{
    string var = ctx->VAR()->getText();
    string val = std::any_cast<string>(visitor->visit(ctx->expr()));
    visitor->getCFG()->current_bb->add_IRInstr(IRInstr::copy, INT, {var, val});
    return var;
}

antlrcpp::Any visitDeclaration_assignement(CodeGenVisitor* visitor, ifccParser::Declaration_assignementContext *ctx)
{
    string var = ctx->VAR()->getText();
    // Variable already declared in DeclarationVisitor, just get its offset
    int offset = visitor->getSymbolTable()->getOffset("main", var);
    string val = std::any_cast<string>(visitor->visit(ctx->expr()));
    visitor->getCFG()->current_bb->add_IRInstr(IRInstr::copy, INT, {var, val});
    return var;
}
