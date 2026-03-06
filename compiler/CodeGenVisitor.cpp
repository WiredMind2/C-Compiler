#include "CodeGenVisitor.h"
#include "visitors/CodeGenArithmetic.h"
#include "visitors/CodeGenBitwise.h"
#include "visitors/CodeGenComparison.h"
#include "visitors/CodeGenMemory.h"
#include <iostream>

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx)
{
    for (auto stmt : ctx->statement()) {
        this->visit(stmt);
    }
    return (string)"0";
}

antlrcpp::Any CodeGenVisitor::visitReturn_stmt(ifccParser::Return_stmtContext *ctx)
{
    string var = std::any_cast<string>(this->visit(ctx->expr()));
    cfg->current_bb->add_IRInstr(IRInstr::copy, INT, {"!eax", var});
    return (string)"!eax";
}

antlrcpp::Any CodeGenVisitor::visitExpr(ifccParser::ExprContext *ctx)
{
    return visit(ctx->bitwiseOR());
}

antlrcpp::Any CodeGenVisitor::visitParenthesis(ifccParser::ParenthesisContext *ctx)
{
    return this->visit(ctx->expr());
}

antlrcpp::Any CodeGenVisitor::visitConstant(ifccParser::ConstantContext *ctx)
{
    // Get the constant value text (e.g., "42")
    string constValue = ctx->getText();
    
    // Create a temporary variable to hold the constant
    string tmp = cfg->create_new_tempvar(INT);
    
    // Add IR instruction to load the constant
    cfg->current_bb->add_IRInstr(IRInstr::ldconst, INT, {tmp, constValue});
    
    return tmp;
}

antlrcpp::Any CodeGenVisitor::visitVariable(ifccParser::VariableContext *ctx)
{
    // Return the variable name as-is
    return ctx->getText();
}

antlrcpp::Any CodeGenVisitor::visitDeclaration(ifccParser::DeclarationContext *ctx)
{
    // With the new grammar, VAR returns a vector of TerminalNodes
    for (auto varNode : ctx->VAR()) {
        std::string var = varNode->getText();
        // Add variable to CFG's symbol table for code generation
        cfg->add_to_symbol_table(var, INT);
    }
    return 0;
}

antlrcpp::Any CodeGenVisitor::visitDeclaration_assignement(ifccParser::Declaration_assignementContext *ctx)
{
    std::string var = ctx->VAR()->getText();
    // Add variable to CFG's symbol table for code generation
    cfg->add_to_symbol_table(var, INT);
    // Visit the expression and generate copy instruction
    std::string val = std::any_cast<std::string>(this->visit(ctx->expr()));
    cfg->current_bb->add_IRInstr(IRInstr::copy, INT, {var, val});
    return var;
}

antlrcpp::Any CodeGenVisitor::visitAssignment(ifccParser::AssignmentContext *ctx)
{
    std::string var = ctx->VAR()->getText();
    std::string val = std::any_cast<std::string>(this->visit(ctx->expr()));
    cfg->current_bb->add_IRInstr(IRInstr::copy, INT, {var, val});
    return var;
}

// Arithmetic expression handlers
antlrcpp::Any CodeGenVisitor::visitMultiplicativeExprRef(ifccParser::MultiplicativeExprRefContext *ctx)
{
    return this->visit(ctx->multiplicative());
}

antlrcpp::Any CodeGenVisitor::visitAddition(ifccParser::AdditionContext *ctx)
{
    return ::visitAddition(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitSubstraction(ifccParser::SubstractionContext *ctx)
{
    return ::visitSubstraction(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitMultiplication(ifccParser::MultiplicationContext *ctx)
{
    return ::visitMultiplication(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitDivision(ifccParser::DivisionContext *ctx)
{
    return ::visitDivision(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitUnaryMinus(ifccParser::UnaryMinusContext *ctx)
{
    return ::visitUnaryMinus(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitUnaryPlus(ifccParser::UnaryPlusContext *ctx)
{
    return ::visitUnaryPlus(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitPrimitiveExprRef(ifccParser::PrimitiveExprRefContext *ctx)
{
    return this->visit(ctx->primitive());
}
