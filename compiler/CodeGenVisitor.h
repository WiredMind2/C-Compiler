#pragma once

#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include "IR.h"
#include "SymbolTable.h"
#include "visitors/CodeGenArithmetic.h"
#include "visitors/CodeGenBitwise.h"
#include "visitors/CodeGenComparison.h"
#include "visitors/CodeGenMemory.h"
#include <map>
#include <string>

// Other function are declared in the visitors/ folder

class CodeGenVisitor : public ifccBaseVisitor
{
public:
        CodeGenVisitor(SymbolTable* symbolTable) : symbolTable(symbolTable) {
                cfg = new CFG();
        }
        virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
        virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;

        virtual antlrcpp::Any visitExpr(ifccParser::ExprContext *ctx) override;

        virtual antlrcpp::Any visitParenthesis(ifccParser::ParenthesisContext *ctx) override;

        virtual antlrcpp::Any visitConstant(ifccParser::ConstantContext *ctx) override;

        virtual antlrcpp::Any visitVariable(ifccParser::VariableContext *ctx) override;

        virtual antlrcpp::Any visitDeclaration(ifccParser::DeclarationContext *ctx) override;
        virtual antlrcpp::Any visitDeclaration_assignement(ifccParser::Declaration_assignementContext *ctx) override;
        virtual antlrcpp::Any visitAssignment(ifccParser::AssignmentContext *ctx) override;

        // Arithmetic expression handlers
        virtual antlrcpp::Any visitMultiplicativeExprRef(ifccParser::MultiplicativeExprRefContext *ctx) override;
        virtual antlrcpp::Any visitAddition(ifccParser::AdditionContext *ctx) override;
        virtual antlrcpp::Any visitSubstraction(ifccParser::SubstractionContext *ctx) override;
        virtual antlrcpp::Any visitMultiplication(ifccParser::MultiplicationContext *ctx) override;
        virtual antlrcpp::Any visitDivision(ifccParser::DivisionContext *ctx) override;
        virtual antlrcpp::Any visitUnaryMinus(ifccParser::UnaryMinusContext *ctx) override;
        virtual antlrcpp::Any visitUnaryPlus(ifccParser::UnaryPlusContext *ctx) override;
        virtual antlrcpp::Any visitPrimitiveExprRef(ifccParser::PrimitiveExprRefContext *ctx) override;

        CFG* getCFG() { return cfg; }
        SymbolTable* getSymbolTable() { return symbolTable; }

private:
        CFG* cfg;
        SymbolTable* symbolTable;
};
