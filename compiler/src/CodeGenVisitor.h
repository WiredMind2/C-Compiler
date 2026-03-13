#pragma once

#include "antlr4-runtime.h"
#include "generated/ifccBaseVisitor.h"
#include "IR.h"
#include "visitors/CodeGenArithmetic.h"
#include "visitors/CodeGenBitwise.h"
#include "visitors/CodeGenComparison.h"
#include "visitors/CodeGenMemory.h"
#include "visitors/CodeGenFunction.h"
#include "visitors/CodeGenRelational.h"
#include "visitors/CodeGenLogical.h"
#include <map>
#include <string>

// Other function are declared in the visitors/ folder

class CodeGenVisitor : public ifccBaseVisitor {
public:
        CodeGenVisitor(TargetArch arch) {
                cfg = new CFG(arch);
        }

        virtual antlrcpp::Any visitProg(ifccParser::ProgContext *ctx) override;
        virtual antlrcpp::Any visitReturn_stmt(ifccParser::Return_stmtContext *ctx) override;

        virtual antlrcpp::Any visitExpr(ifccParser::ExprContext *ctx) override;

        virtual antlrcpp::Any visitParenthesis(ifccParser::ParenthesisContext *ctx) override;

        virtual antlrcpp::Any visitConstant(ifccParser::ConstantContext *ctx) override;

        virtual antlrcpp::Any visitVariable(ifccParser::VariableContext *ctx) override;

        virtual antlrcpp::Any visitDeclaration_list(ifccParser::Declaration_listContext *ctx) override;
        virtual antlrcpp::Any visitVar_decl(ifccParser::Var_declContext *ctx) override;
        virtual antlrcpp::Any visitVar_decl_with_init(ifccParser::Var_decl_with_initContext *ctx) override;
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
        virtual antlrcpp::Any visitPrimitiveExprRef(ifccParser::PrimitiveExprRefContext *ctx) override;

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
        
        // Condition handler
        virtual antlrcpp::Any visitCondition(ifccParser::ConditionContext *ctx) override;
        // While loop handler
        virtual antlrcpp::Any visitWhile_loop(ifccParser::While_loopContext *ctx) override;

        CFG *getCFG() { return cfg; }

        CFG *cfg;
};
