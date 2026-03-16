#include "CodeGenVisitor.h"
#include "CodeGenArithmetic.h"

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx)
{
    for (auto stmt : ctx->statement()) {
        this->visit(stmt);
    }
    std::cerr << "BBs: " << cfg->getBBs().size() << std::endl; for(auto bb : cfg->getBBs()) { std::cerr << "BB " << bb->label << " has " << bb->instrs.size() << " instructions" << std::endl; } return "0";
}

antlrcpp::Any CodeGenVisitor::visitReturn_stmt(ifccParser::Return_stmtContext *ctx)
{
    StackParam var = std::any_cast<StackParam>(this->visit(ctx->expr()));
    auto* bb = cfg->current_bb;
    if (var.type == IRType::FLOAT64) {
        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, var.name, IRType::FLOAT64));
        bb->add_IRInstr(new FToIInstr(bb, Reg::RET, Reg::W0));
    } else {
        bb->add_IRInstr(new LoadStackInstr(bb, Reg::RET, var.name, var.type));
    }
    return nullptr;
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

antlrcpp::Any CodeGenVisitor::visitDouble_constant(ifccParser::Double_constantContext* ctx) {
    return ::visitDouble_constant(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitVariable(ifccParser::VariableContext *ctx)
{
    return ::visitVariable(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitVar_decl_list(ifccParser::Var_decl_listContext *ctx)
{
    return ::visitVar_decl_list(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitVar_decl_with_init(ifccParser::Var_decl_with_initContext *ctx)
{
    return ::visitVar_decl_with_init(this, ctx);
}
antlrcpp::Any CodeGenVisitor::visitArray_decl_list(ifccParser::Array_decl_listContext *ctx)
{
    return ::visitArray_decl_list(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitArray_decl_with_init(ifccParser::Array_decl_with_initContext *ctx)
{
    return ::visitArray_decl_with_init(this, ctx);
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

antlrcpp::Any CodeGenVisitor::visitUnaryNot(ifccParser::UnaryNotContext *ctx)
{
    return ::visitUnaryNot(this, ctx);
}
antlrcpp::Any CodeGenVisitor::visitDereference(ifccParser::DereferenceContext *ctx)
{
    return ::visitDereference(this, ctx);
}
antlrcpp::Any CodeGenVisitor::visitAddressOf(ifccParser::AddressOfContext *ctx)
{
    return ::visitAddressOf(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitPrimitiveExprRef(ifccParser::PrimitiveExprRefContext *ctx)
{
    return this->visit(ctx->primitive());
}

antlrcpp::Any CodeGenVisitor::visitFunctionCall(ifccParser::FunctionCallContext *ctx)
{
    return this->visit(ctx->function_call());
}


antlrcpp::Any CodeGenVisitor::visitArray_subscript(ifccParser::Array_subscriptContext *ctx) {
    return ::visitArray_subscript(this, ctx);
}

// Sequential / compound-assignment pass-throughs
antlrcpp::Any CodeGenVisitor::visitSequentialExprRef(ifccParser::SequentialExprRefContext *ctx)
{
    return this->visit(ctx->compoundAssignment());
}

antlrcpp::Any CodeGenVisitor::visitSequentialRule(ifccParser::SequentialRuleContext *ctx)
{
    this->visit(ctx->compoundAssignment());
    return this->visit(ctx->sequential());
}

antlrcpp::Any CodeGenVisitor::visitCompoundAssignmentRef(ifccParser::CompoundAssignmentRefContext *ctx)
{
    return this->visit(ctx->logicalOR());
}

// Bitwise handlers
antlrcpp::Any CodeGenVisitor::visitBitwiseORRef(ifccParser::BitwiseORRefContext *ctx)
{
    return this->visit(ctx->bitwiseXOR());
}

antlrcpp::Any CodeGenVisitor::visitBitwiseORRule(ifccParser::BitwiseORRuleContext *ctx)
{
    return ::visitBitwiseORRule(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitBitwiseXORRef(ifccParser::BitwiseXORRefContext *ctx)
{
    return this->visit(ctx->bitwiseAND());
}

antlrcpp::Any CodeGenVisitor::visitBitwiseXORRule(ifccParser::BitwiseXORRuleContext *ctx)
{
    return ::visitBitwiseXORRule(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitBitwiseANDRef(ifccParser::BitwiseANDRefContext *ctx)
{
    return this->visit(ctx->equality());
}

antlrcpp::Any CodeGenVisitor::visitBitwiseANDRule(ifccParser::BitwiseANDRuleContext *ctx)
{
    return ::visitBitwiseANDRule(this, ctx);
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
