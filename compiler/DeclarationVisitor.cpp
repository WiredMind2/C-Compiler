#include "DeclarationVisitor.h"

#include "generated/ifccParser.h"

std::any DeclarationVisitor::visitProg(ifccParser::ProgContext *ctx) {
    symbolTable->addScope("main");
    scopeStack.push_back("main");
    // Iterate over all statements in the program
    for (auto stmt : ctx->statement()) {
        this->visit(stmt);
    }
    scopeStack.pop_back();
    return 0;
}

std::any DeclarationVisitor::visitStatement(ifccParser::StatementContext *ctx) {
    return this->visitChildren(ctx);
}

std::any DeclarationVisitor::visitDeclaration(ifccParser::DeclarationContext *ctx) {
    // With the new grammar, VAR returns a vector of TerminalNodes
    for (auto varNode : ctx->VAR()) {
        std::string name = varNode->getText();
        symbolTable->declare(currentScope(), name);
    }
    return 0;
}

std::any DeclarationVisitor::visitDeclaration_assignement(ifccParser::Declaration_assignementContext *ctx) {
    std::string name = ctx->VAR()->getText();
    symbolTable->declare(currentScope(), name);
    return 0;
}
