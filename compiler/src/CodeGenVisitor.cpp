#include "CodeGenVisitor.h"
#include "visitors/CodeGenArithmetic.h"

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx)
{
    for (auto stmt : ctx->statement()) {
        this->visit(stmt);
    }
    std::cerr << "BBs: " << cfg->getBBs().size() << std::endl; for(auto bb : cfg->getBBs()) { std::cerr << "BB " << bb->label << " has " << bb->instrs.size() << " instructions" << std::endl; } return "0";
}

antlrcpp::Any CodeGenVisitor::visitReturn_stmt(ifccParser::Return_stmtContext *ctx)
{
    string returned_expression = std::any_cast<string>(this->visit(ctx->expr()));
    Type exprType = cfg->current_bb->get_var_type(returned_expression);

    string fonction_name = cfg->getBBs()[0]->label;
    CFG::FunctionSignature* function_signature = cfg->get_function(fonction_name);  // Get the function signature (use to get return type)
    Type returnType = (function_signature != nullptr) ? function_signature->returnType : INT; // Return type (default to INT if not found)

    const string registerToUse = returnType == Type::DOUBLE ? "!xmm0" : "!eax";
    cfg->current_bb->add_IRInstr(IRInstr::copy, exprType, {registerToUse, returned_expression});
    return registerToUse;
}

antlrcpp::Any CodeGenVisitor::visitExpr(ifccParser::ExprContext *ctx)
{
    return visit(ctx->sequential());
}

antlrcpp::Any CodeGenVisitor::visitParenthesis(ifccParser::ParenthesisContext *ctx)
{
    return this->visit(ctx->expr());
}

antlrcpp::Any CodeGenVisitor::visitConstant(ifccParser::ConstantContext *ctx)
{
    return ::visitConstant(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitVariable(ifccParser::VariableContext *ctx)
{
    return ::visitVariable(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitDeclaration_list(ifccParser::Declaration_listContext *ctx)
{
    return ::visitDeclaration_list(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitVar_decl(ifccParser::Var_declContext *ctx)
{
    return ::visitVar_decl(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitVar_decl_with_init(ifccParser::Var_decl_with_initContext *ctx)
{
    return ::visitVar_decl_with_init(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitAssignment(ifccParser::AssignmentContext *ctx)
{
    return ::visitAssignment(this, ctx);
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

antlrcpp::Any CodeGenVisitor::visitModulo(ifccParser::ModuloContext *ctx)
{
    return ::visitModulo(this, ctx);
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

// Equality expression handlers
antlrcpp::Any CodeGenVisitor::visitEqualityExprRef(ifccParser::EqualityExprRefContext *ctx)
{
    return this->visit(ctx->relational());
}

antlrcpp::Any CodeGenVisitor::visitEquals(ifccParser::EqualsContext *ctx)
{
    return ::visitEquals(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitDifferent(ifccParser::DifferentContext *ctx)
{
    return ::visitDifferent(this, ctx);
}

// Relational expression handlers
antlrcpp::Any CodeGenVisitor::visitRelationalExprRef(ifccParser::RelationalExprRefContext *ctx)
{
    return ::visitRelationalExprRef(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitSmallerStrictThan(ifccParser::SmallerStrictThanContext *ctx)
{
    return ::visitSmallerStrictThan(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitGreaterStrictThan(ifccParser::GreaterStrictThanContext *ctx)
{
    return ::visitGreaterStrictThan(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitSmallerThan(ifccParser::SmallerThanContext *ctx)
{
    return ::visitSmallerThan(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitGreaterThan(ifccParser::GreaterThanContext *ctx)
{
    return ::visitGreaterThan(this, ctx);
}

// Logical expression handlers
antlrcpp::Any CodeGenVisitor::visitLogicalORRef(ifccParser::LogicalORRefContext *ctx)
{
    return ::visitLogicalORRef(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitLogicalORRule(ifccParser::LogicalORRuleContext *ctx)
{
    return ::visitLogicalORRule(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitLogicalANDRef(ifccParser::LogicalANDRefContext *ctx)
{
    return ::visitLogicalANDRef(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitLogicalANDRule(ifccParser::LogicalANDRuleContext *ctx)
{
    return ::visitLogicalANDRule(this, ctx);
}

// Function handlers
antlrcpp::Any CodeGenVisitor::visitFunction_definition(ifccParser::Function_definitionContext *ctx)
{
    return ::visitFunction_definition(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitFunction_declaration(ifccParser::Function_declarationContext *ctx)
{
    return ::visitFunction_declaration(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitFunction_call(ifccParser::Function_callContext *ctx)
{
    return ::visitFunctionCall(this, ctx);
}

// Scope handler - handles any { ... } block
antlrcpp::Any CodeGenVisitor::visitScope(ifccParser::ScopeContext *ctx)
{
    // Visit all statements in the scope
    for (auto stmt : ctx->statement()) {
        this->visit(stmt);
    }
    return 0;
}
