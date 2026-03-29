#pragma once

#include "antlr4-runtime.h"
#include "../generated/ifccBaseVisitor.h"
#include "../ir/IR.h"
#include "utils.h"
#include "CodeGenArithmetic.h"
#include "CodeGenBitwise.h"
#include "CodeGenComparison.h"
#include "CodeGenMemory.h"
#include "CodeGenFunction.h"
#include "CodeGenRelational.h"
#include "CodeGenLogical.h"
#include <map>
#include <string>

// Other function are declared in the visitors/ folder

class CodeGenVisitor : public ifccBaseVisitor {
public:
    CodeGenVisitor(TargetArch arch, bool include_stdio = false, bool include_stdlib = false) {
        cfg = new CFG(arch);
        this->include_stdio = include_stdio;
        this->include_stdlib = include_stdlib;
    }

    virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
    virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;

    virtual antlrcpp::Any visitExpr(ifccParser::ExprContext *ctx) override;

    virtual antlrcpp::Any visitParenthesis(ifccParser::ParenthesisContext *ctx) override;

    virtual antlrcpp::Any visitConstant(ifccParser::ConstantContext *ctx) override;
    virtual antlrcpp::Any visitDouble_constant(ifccParser::Double_constantContext *ctx) override;
    virtual antlrcpp::Any visitChar_constant(ifccParser::Char_constantContext *ctx) override;

    virtual antlrcpp::Any visitVariable(ifccParser::VariableContext *ctx) override;

    virtual antlrcpp::Any visitDeclaration(ifccParser::DeclarationContext *ctx) override;
    virtual antlrcpp::Any visitAssignment(ifccParser::AssignmentContext *ctx) override;

    // Arithmetic expression handlers
    virtual antlrcpp::Any visitMultiplicativeExprRef(ifccParser::MultiplicativeExprRefContext *ctx) override;
    virtual antlrcpp::Any visitAddition(ifccParser::AdditionContext *ctx) override;
    virtual antlrcpp::Any visitSubstraction(ifccParser::SubstractionContext *ctx) override;
    virtual antlrcpp::Any visitMultiplication(ifccParser::MultiplicationContext *ctx) override;
    virtual antlrcpp::Any visitDivision(ifccParser::DivisionContext *ctx) override;
    virtual antlrcpp::Any visitModulo(ifccParser::ModuloContext *ctx) override;
    virtual antlrcpp::Any visitUnaryMinus(ifccParser::UnaryMinusContext *ctx) override;
    virtual antlrcpp::Any visitUnaryPlus(ifccParser::UnaryPlusContext *ctx) override;
    virtual antlrcpp::Any visitUnaryNot(ifccParser::UnaryNotContext *ctx) override;
    virtual antlrcpp::Any visitPrimitiveExprRef(ifccParser::PrimitiveExprRefContext *ctx) override;
    virtual antlrcpp::Any visitFunctionCall(ifccParser::FunctionCallContext *ctx) override;

    // Sequential / compound-assignment pass-throughs
    virtual antlrcpp::Any visitSequentialExprRef(ifccParser::SequentialExprRefContext *ctx) override;
    virtual antlrcpp::Any visitSequentialRule(ifccParser::SequentialRuleContext *ctx) override;
    virtual antlrcpp::Any visitCompoundAssignmentRef(ifccParser::CompoundAssignmentRefContext *ctx) override;

    // Bitwise handlers
    virtual antlrcpp::Any visitBitwiseORRef(ifccParser::BitwiseORRefContext *ctx) override;
    virtual antlrcpp::Any visitBitwiseORRule(ifccParser::BitwiseORRuleContext *ctx) override;
    virtual antlrcpp::Any visitBitwiseXORRef(ifccParser::BitwiseXORRefContext *ctx) override;
    virtual antlrcpp::Any visitBitwiseXORRule(ifccParser::BitwiseXORRuleContext *ctx) override;
    virtual antlrcpp::Any visitBitwiseANDRef(ifccParser::BitwiseANDRefContext *ctx) override;
    virtual antlrcpp::Any visitBitwiseANDRule(ifccParser::BitwiseANDRuleContext *ctx) override;

    // Equality expression handlers
    virtual antlrcpp::Any visitEqualityExprRef(ifccParser::EqualityExprRefContext *ctx) override;
    virtual antlrcpp::Any visitEquals(ifccParser::EqualsContext *ctx) override;
    virtual antlrcpp::Any visitDifferent(ifccParser::DifferentContext *ctx) override;

    // Relational expression handlers
    virtual antlrcpp::Any visitRelationalExprRef(ifccParser::RelationalExprRefContext *ctx) override;
    virtual antlrcpp::Any visitSmallerStrictThan(ifccParser::SmallerStrictThanContext *ctx) override;
    virtual antlrcpp::Any visitGreaterStrictThan(ifccParser::GreaterStrictThanContext *ctx) override;
    virtual antlrcpp::Any visitSmallerThan(ifccParser::SmallerThanContext *ctx) override;
    virtual antlrcpp::Any visitGreaterThan(ifccParser::GreaterThanContext *ctx) override;

    // Logical expression handlers
    virtual antlrcpp::Any visitLogicalORRef(ifccParser::LogicalORRefContext *ctx) override;
    virtual antlrcpp::Any visitLogicalORRule(ifccParser::LogicalORRuleContext *ctx) override;
    virtual antlrcpp::Any visitLogicalANDRef(ifccParser::LogicalANDRefContext *ctx) override;
    virtual antlrcpp::Any visitLogicalANDRule(ifccParser::LogicalANDRuleContext *ctx) override;

    // Function handlers
    virtual antlrcpp::Any visitFunction_definition(ifccParser::Function_definitionContext *ctx) override;
    virtual antlrcpp::Any visitFunction_declaration(ifccParser::Function_declarationContext *ctx) override;
    virtual antlrcpp::Any visitFunction_call(ifccParser::Function_callContext *ctx) override;

    // Scope handler
    virtual antlrcpp::Any visitScope(ifccParser::ScopeContext *ctx) override;

    // Statement handler
    virtual antlrcpp::Any visitStatement(ifccParser::StatementContext *ctx) override;

    // Control flow handlers
    virtual antlrcpp::Any visitCondition(ifccParser::ConditionContext *ctx) override;
    virtual antlrcpp::Any visitWhile_loop(ifccParser::While_loopContext *ctx) override;
    virtual antlrcpp::Any visitFor_loop(ifccParser::For_loopContext *ctx) override;
    virtual antlrcpp::Any visitBreak_stmt(ifccParser::Break_stmtContext *ctx) override;
    virtual antlrcpp::Any visitContinue_stmt(ifccParser::Continue_stmtContext *ctx) override;

    CFG *getCFG() { return cfg; }

private:
    CFG *cfg;
    bool include_stdio = false;
    bool include_stdlib = false;
public:
    bool has_stdio() const { return include_stdio; }
    bool has_stdlib() const { return include_stdlib; }
};
