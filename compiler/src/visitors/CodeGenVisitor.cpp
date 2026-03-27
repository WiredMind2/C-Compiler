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
    auto* bb = cfg->current_bb;

    // when returning something, as opposed to "return ;"
    if (!ctx->expr().empty()) {
        StackParam var = any_cast_to_stack_param_or_throw_on_nullptr(this->visit(ctx->expr()[0]));

        const string current_function_name = cfg->getCurrentFunction();
        CFG::FunctionSignature* current_function_signature = cfg->get_function(current_function_name);
        const IRType current_function_return_type = current_function_signature->returnType;

        if (var.type != current_function_return_type) {
            bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, var.name, var.type));
            bb->generate_conversion_instruction(Reg::W0, var.type, Reg::W1, current_function_return_type);
            bb->add_IRInstr(new CopyRegInstr(bb, Reg::RET, Reg::W1, current_function_return_type));
        } else {
            bb->add_IRInstr(new LoadStackInstr(bb, Reg::RET, var.name, var.type));
        }
        bb->add_IRInstr(new RetInstr(bb, var.type));
    }
    // empty return
    else {
        const string current_function_name = cfg->getCurrentFunction();
        CFG::FunctionSignature* current_function_signature = cfg->get_function(current_function_name);
        const IRType current_function_return_type = current_function_signature->returnType;

        if (current_function_return_type != IRType::VOID) {
            std::cerr << "Cannot return no value for a non void returning function" << std::endl;
            exit(1);
        }
        bb->add_IRInstr(new RetInstr(bb, IRType::VOID));
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
    // Check if current block already has a return or an unconditional jump (break/continue)
    if (!bb->instrs.empty()) {
        if (dynamic_cast<RetInstr*>(bb->instrs.back()) != nullptr) {
            // Already returned, skip this statement to avoid unreachable code
            return nullptr;
        }
    }
    // Also check if the block is already closed with an unconditional branch (exit_true set but no exit_false)
    if (bb->exit_true != nullptr && bb->exit_false == nullptr) {
        return nullptr;
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
        condResult = any_cast_to_stack_param_or_throw_on_nullptr(this->visit(ctx->expr()));
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
    if (ctx->statement()) {
        // Visit the scope (the if-body)
        this->visit(ctx->statement());
    }
    // Get the last BB after visiting the then scope (may differ from thenBB if there are nested ifs)
    BasicBlock* lastThenBB = cfg->current_bb;
    // Add jump to merge block ONLY if it doesn't already have a return or break/continue
    if (lastThenBB->exit_true == nullptr &&
        (lastThenBB->instrs.empty() || dynamic_cast<RetInstr*>(lastThenBB->instrs.back()) == nullptr)) {
        lastThenBB->exit_true = mergeBB;
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
        // Get the last BB after visiting the else scope
        BasicBlock* lastElseBB = cfg->current_bb;
        // Add jump to merge block ONLY if it doesn't already have a return or break/continue
        if (lastElseBB->exit_true == nullptr &&
            (lastElseBB->instrs.empty() || dynamic_cast<RetInstr*>(lastElseBB->instrs.back()) == nullptr)) {
            lastElseBB->exit_true = mergeBB;
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
        condResult = any_cast_to_stack_param_or_throw_on_nullptr(this->visit(ctx->expr()));
    }

    // Condition result determines whether to enter body or exit
    condBB->test_var_name = condResult.name;
    condBB->exit_true = bodyBB;
    condBB->exit_false = afterBB;

    // Store break/continue targets on bodyBB via dedicated fields (not exit_true/exit_false,
    // which will be overwritten if the body starts with an if-statement).
    bodyBB->loop_continue_target = condBB;
    bodyBB->loop_break_target    = afterBB;

    // Generate code for loop body
    cfg->current_bb = bodyBB;
    cfg->getStackBBs().push_back(bodyBB);  // Push loop body onto stack so break/continue can find it
    if (ctx->scope()) {
        this->visit(ctx->scope());
    }
    cfg->getStackBBs().pop_back();  // Pop loop body after visiting

    // cfg->current_bb is now the last BB generated inside the loop body
    // (could be bodyBB itself, or a mergeBB from a nested if).
    // Only set exit_true (back to condBB) if this BB hasn't already been
    // redirected by a break/continue statement.
    BasicBlock* lastBodyBB = cfg->current_bb;
    if (lastBodyBB->exit_true == nullptr) {
        lastBodyBB->exit_true = condBB;
    }

    // Continue from after loop
    cfg->current_bb = afterBB;

    return 0;
}

// For loop handler
antlrcpp::Any CodeGenVisitor::visitFor_loop(ifccParser::For_loopContext *ctx)
{
    CFG* cfg = this->cfg;

    ifccParser::ExprContext* initExpr = nullptr;
    ifccParser::ExprContext* condExpr = nullptr;
    ifccParser::ExprContext* updateExpr = nullptr;

    std::vector<int> semicolonTokenIndices;
    if (!ctx->children.empty()) {
        for (auto* child : ctx->children) {
            auto* terminal = dynamic_cast<antlr4::tree::TerminalNode*>(child);
            if (terminal != nullptr && terminal->getText() == ";") {
                semicolonTokenIndices.push_back(terminal->getSymbol()->getTokenIndex());
            }
        }
    }

    if (semicolonTokenIndices.size() == 2) {
        // if there are exactly 2 semicolons, we can determine the expressions based on their positions
        for (auto* exprCtx : ctx->expr()) {
            int exprStart = exprCtx->getStart()->getTokenIndex();
            if (exprStart < semicolonTokenIndices[0]) {
                // This is the initialization expression (before the first semicolon)
                initExpr = exprCtx;
            } else if (exprStart < semicolonTokenIndices[1]) {
                // This is the condition expression (between the first and second semicolon)
                condExpr = exprCtx;
            } else {
                // This is the update expression (after the second semicolon)
                updateExpr = exprCtx;
            }
        }
    } else {
        auto exprs = ctx->expr();
        if (exprs.size() > 0) initExpr = exprs[0];
        if (exprs.size() > 1) condExpr = exprs[1];
        if (exprs.size() > 2) updateExpr = exprs[2];
    }

    //  init
    if (initExpr != nullptr) {
        this->visit(initExpr);
    }

    // If init created extra blocks (for example a short-circuit expr), continue from the actual current BB.
    BasicBlock* initEndBB = cfg->current_bb;

    // Then create CFG blocks: cond -> body -> update -> cond, and after for exit.
    BasicBlock* condBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* bodyBB = new BasicBlock(cfg, cfg->new_BB_name(), true);
    BasicBlock* updateBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* afterBB = new BasicBlock(cfg, cfg->new_BB_name());

    cfg->add_bb(condBB);
    cfg->add_bb(bodyBB);
    cfg->add_bb(updateBB);
    cfg->add_bb(afterBB);

    initEndBB->exit_true = condBB;

    // condition
    cfg->current_bb = condBB;
    if (condExpr != nullptr) {
        StackParam condResult = any_cast_to_stack_param_or_throw_on_nullptr(this->visit(condExpr));
        // Get the last block after visiting condition (may have changed due to short-circuit booleans)
        BasicBlock* condEndBB = cfg->current_bb;
        condEndBB->test_var_name = condResult.name;
        condEndBB->exit_true = bodyBB;
        condEndBB->exit_false = afterBB;
    } else {
        // for(;;) or missing middle expression means an always-true loop.
        condBB->exit_true = bodyBB;
    }

    // body
    bodyBB->loop_continue_target = updateBB;
    bodyBB->loop_break_target = afterBB;

    cfg->current_bb = bodyBB;
    cfg->getStackBBs().push_back(bodyBB);
    if (ctx->scope()) {
        this->visit(ctx->scope());
    }
    cfg->getStackBBs().pop_back();

    BasicBlock* lastBodyBB = cfg->current_bb;
    if (lastBodyBB->exit_true == nullptr &&
        (lastBodyBB->instrs.empty() || dynamic_cast<RetInstr*>(lastBodyBB->instrs.back()) == nullptr)) {
        lastBodyBB->exit_true = updateBB;
    }

    // update
    cfg->current_bb = updateBB;
    if (updateExpr != nullptr) {
        this->visit(updateExpr);
    }
    if (updateBB->exit_true == nullptr &&
        (updateBB->instrs.empty() || dynamic_cast<RetInstr*>(updateBB->instrs.back()) == nullptr)) {
        updateBB->exit_true = condBB;
    }

    // continue after loop
    cfg->current_bb = afterBB;

    return 0;
}

antlrcpp::Any CodeGenVisitor::visitBreak_stmt(ifccParser::Break_stmtContext *ctx)
{
    (void)ctx;

    // Find nearest enclosing loop block and jump to its break target.
    BasicBlock* currentBB = cfg->current_bb;
    BasicBlock* targetBB = nullptr;
    std::vector<BasicBlock*> bbStack = cfg->getStackBBs();
    for (auto it = bbStack.rbegin(); it != bbStack.rend(); ++it) {
        BasicBlock* bb = *it;
        if (bb->is_loop) {
            targetBB = bb->loop_break_target;
            break;
        }
    }

    if (targetBB) {
        currentBB->exit_true = targetBB;
    } else {
        std::cerr << "Error: 'break' statement not within a loop." << std::endl;
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
    std::vector<BasicBlock*> bbStack = cfg->getStackBBs();
    for (auto it = bbStack.rbegin(); it != bbStack.rend(); ++it) {
        BasicBlock* bb = *it;
        if (bb->is_loop) {
            targetBB = bb->loop_continue_target;
            break;
        }
    }

    if (targetBB) {
        currentBB->exit_true = targetBB;
    } else {
        std::cerr << "Error: 'continue' statement not within a loop." << std::endl;
        exit(1);
    }
    return nullptr;
}
