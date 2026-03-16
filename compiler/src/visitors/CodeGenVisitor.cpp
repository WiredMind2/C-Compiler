#include "CodeGenVisitor.h"
#include "CodeGenArithmetic.h"
#include "../ir/IRInstr.h"

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
    bb->add_IRInstr(new RetInstr(bb, var.type));
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

antlrcpp::Any CodeGenVisitor::visitUnaryNot(ifccParser::UnaryNotContext *ctx)
{
    return ::visitUnaryNot(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitPrimitiveExprRef(ifccParser::PrimitiveExprRefContext *ctx)
{
    return this->visit(ctx->primitive());
}

antlrcpp::Any CodeGenVisitor::visitFunctionCall(ifccParser::FunctionCallContext *ctx)
{
    return this->visit(ctx->function_call());
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
        // If the statement was a return, we should ideally stop visiting,
        // but for now we follow the same logic as before.
        // The issue is that visitCondition/visitWhile_loop also need to know.
    }
    return 0;
}

antlrcpp::Any CodeGenVisitor::visitStatement(ifccParser::StatementContext *ctx)
{
    auto* bb = cfg->current_bb;
    // Check if current block already has a return
    if (!bb->instrs.empty()) {
        if (dynamic_cast<RetInstr*>(bb->instrs.back()) != nullptr) {
            // Already returned, skip this statement to avoid unreachable code
            // that might mess up our CFG jump logic
            return nullptr;
        }
    }
    return visitChildren(ctx);
}

// Condition handler - handles if statements
antlrcpp::Any CodeGenVisitor::visitCondition(ifccParser::ConditionContext *ctx)
{
    CFG* cfg = this->cfg;
    BasicBlock* currentBB = cfg->current_bb;
    
    // Create blocks for then-branch, else-branch (optional), and merge point
    BasicBlock* thenBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* elseBB = nullptr;
    BasicBlock* mergeBB = new BasicBlock(cfg, cfg->new_BB_name());
    
    // If there's an else clause, create else block
    if (ctx->else_block()) {
        elseBB = new BasicBlock(cfg, cfg->new_BB_name());
    }
    
    // Add the new blocks to CFG
    cfg->add_bb(thenBB);
    if (elseBB) cfg->add_bb(elseBB);
    cfg->add_bb(mergeBB);
    
    // Generate code for the condition expression
    // Visit the expression which should leave result in a register
    StackParam condResult("!tmp0", IRType::INT32);
    if (ctx->expr()) {
        condResult = std::any_cast<StackParam>(this->visit(ctx->expr()));
    }
    
    // Set up the control flow from current block
    // The condition result determines which branch to take
    currentBB->test_var_name = condResult.name;
    if (elseBB) {
        // if-else: conditional jump to then or else
        currentBB->exit_true = thenBB;
        currentBB->exit_false = elseBB;
    } else {
        // if without else: conditional jump to then, fall through to merge
        // For conditional jump we need exit_false to point to merge
        currentBB->exit_true = thenBB;
        currentBB->exit_false = mergeBB;
    }
    
    // Now generate code for the then-branch
    cfg->current_bb = thenBB;
    if (ctx->scope()) {
        // Visit the scope (the if-body)
        this->visit(ctx->scope());
    }
    // Add jump to merge block at end of then ONLY if it doesn't already have a return
    if (thenBB->instrs.empty() || dynamic_cast<RetInstr*>(thenBB->instrs.back()) == nullptr) {
        thenBB->exit_true = mergeBB;
    }
    
    // If there's an else clause
    if (elseBB && ctx->else_block()) {
        cfg->current_bb = elseBB;
        // Handle else_block which can be either scope or another condition
        auto elseBlock = ctx->else_block();
        if (elseBlock->scope()) {
            this->visit(elseBlock->scope());
        } else if (elseBlock->condition()) {
            this->visit(elseBlock->condition());
        }
        // Add jump to merge block at end of else ONLY if it doesn't already have a return
        if (elseBB->instrs.empty() || dynamic_cast<RetInstr*>(elseBB->instrs.back()) == nullptr) {
            elseBB->exit_true = mergeBB;
        }
    }
    
    // Continue from merge block - this is where code continues after the if-else
    cfg->current_bb = mergeBB;
    
    return 0;
}

// While loop handler
antlrcpp::Any CodeGenVisitor::visitWhile_loop(ifccParser::While_loopContext *ctx)
{
    CFG* cfg = this->cfg;
    BasicBlock* currentBB = cfg->current_bb;

    // Create blocks for condition check, loop body, and after loop
    BasicBlock* condBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* bodyBB = new BasicBlock(cfg, cfg->new_BB_name(), true); // Mark bodyBB as a loop block for break/continue handling
    BasicBlock* afterBB = new BasicBlock(cfg, cfg->new_BB_name());

    // Add blocks to CFG
    cfg->add_bb(condBB);
    cfg->add_bb(bodyBB);
    cfg->add_bb(afterBB);

    // Set up current block to unconditionally jump to condition
    currentBB->exit_true = condBB;

    // Set up condition block
    cfg->current_bb = condBB;
    StackParam condResult("!tmp0", IRType::INT32);
    if (ctx->expr()) {
        condResult = std::any_cast<StackParam>(this->visit(ctx->expr()));
    }

    // Condition result determines whether to enter body or exit
    condBB->test_var_name = condResult.name;
    condBB->exit_true = bodyBB;
    condBB->exit_false = afterBB;

    // Generate code for loop body
    cfg->current_bb = bodyBB;
    if (ctx->scope()) {
        this->visit(ctx->scope());
    }

    // Keep explicit loop targets on the loop body block:
    // - exit_true: continue target (re-check condition)
    // - exit_false: break target (after loop)
    bodyBB->exit_true = condBB;
    bodyBB->exit_false = afterBB;

    // Continue from after loop
    cfg->current_bb = afterBB;

    return 0;
}

antlrcpp::Any CodeGenVisitor::visitBreak_stmt(ifccParser::Break_stmtContext *ctx)
{
    (void)ctx;

    // Find nearest enclosing loop block and jump to its break target.
    BasicBlock* currentBB = cfg->current_bb;
    BasicBlock* targetBB = nullptr;
    vector<BasicBlock*> bbStack = cfg->getStackBBs();
    for (auto it = bbStack.rbegin(); it != bbStack.rend(); ++it) {
        BasicBlock* bb = *it;
        if (bb->is_loop) {
            targetBB = bb->exit_false;
            break;
        }
    }

    if (targetBB) {
        currentBB->exit_true = targetBB;
    } else {
        cerr << "Error: 'break' statement not within a loop." << endl;
        exit(1);
    }
    return nullptr;
}

antlrcpp::Any CodeGenVisitor::visitContinue_stmt(ifccParser::Continue_stmtContext *ctx)
{
    (void)ctx;

    // Find nearest enclosing loop block and jump to its continue target.
    BasicBlock* currentBB = cfg->current_bb;
    BasicBlock* targetBB = nullptr;
    vector<BasicBlock*> bbStack = cfg->getStackBBs();
    for (auto it = bbStack.rbegin(); it != bbStack.rend(); ++it) {
        BasicBlock* bb = *it;
        if (bb->is_loop) {
            targetBB = bb->exit_true;
            break;
        }
    }

    if (targetBB) {
        currentBB->exit_true = targetBB;
    } else {
        cerr << "Error: 'continue' statement not within a loop." << endl;
        exit(1);
    }
    return nullptr;
}
