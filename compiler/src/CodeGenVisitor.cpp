#include "CodeGenVisitor.h"
#include "visitors/CodeGenArithmetic.h"

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx)
{
    // If there is no statement, or if we want to ensure main is registered,
    // we should check if a function named "main" has been visited.
    // However, our grammar allows both function definitions and global statements.
    
    for (auto stmt : ctx->statement()) {
        this->visit(stmt);
    }

    // Ensure "main" is registered as a function if it was created as a fallback BB
    if (cfg->getFunctions().empty() && cfg->getBBs().size() > 0 && cfg->getBBs()[0]->label == "main") {
        cfg->add_function("main", INT, {}, {});
    }
    
    std::cerr << "BBs: " << cfg->getBBs().size() << std::endl; 
    for(auto bb : cfg->getBBs()) { 
        std::cerr << "BB " << bb->label << " has " << bb->instrs.size() << " instructions" << std::endl; 
    } 
    return "0";
}

antlrcpp::Any CodeGenVisitor::visitReturn_stmt(ifccParser::Return_stmtContext *ctx)
{
    string var = std::any_cast<string>(this->visit(ctx->expr()));
    cfg->getCurrentBB()->add_IRInstr(IRInstr::copy, INT, {"!eax", var});
    // Add return epilogue instructions
    cfg->getCurrentBB()->add_IRInstr(IRInstr::ret, INT, {});
    return "!eax";
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
            string left = std::any_cast<string>(this->visit(ctx->equality()));
            string right = std::any_cast<string>(this->visit(ctx->relational()));
            string result = cfg->getCurrentBB()->create_new_tempvar(INT);
            cfg->getCurrentBB()->add_IRInstr(IRInstr::cmp_eq, INT, {result, left, right});
            return result;
        }

        antlrcpp::Any CodeGenVisitor::visitDifferent(ifccParser::DifferentContext *ctx)
        {
            string left = std::any_cast<string>(this->visit(ctx->equality()));
            string right = std::any_cast<string>(this->visit(ctx->relational()));
            string result = cfg->getCurrentBB()->create_new_tempvar(INT);
            cfg->getCurrentBB()->add_IRInstr(IRInstr::cmp_ne, INT, {result, left, right});
            return result;
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

// Condition handler - handles if-else and else-if chains
antlrcpp::Any CodeGenVisitor::visitCondition(ifccParser::ConditionContext *ctx)
{
    CFG* cfg = this->cfg;
    
    // Evaluate the condition expression in the CURRENT basic block
    string testVar = std::any_cast<string>(this->visit(ctx->expr()));
    
    // Create basic blocks for the if statement branches and continuation
    BasicBlock* thenBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* mergeBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* elseBB = nullptr;
    
    // Connect currentBB to thenBB/elseBB/mergeBB
    BasicBlock* currentBB = cfg->getCurrentBB();
    
    if (ctx->else_block()) {
        elseBB = new BasicBlock(cfg, cfg->new_BB_name());
        currentBB->setTerminator(new TerminatorInstr(currentBB, testVar, thenBB, elseBB));
    } else {
        currentBB->setTerminator(new TerminatorInstr(currentBB, testVar, thenBB, mergeBB));
    }
    
    // Visit then block
    cfg->add_bb(thenBB);
    cfg->push_bb(thenBB);
    this->visit(ctx->scope());
    if (!cfg->getCurrentBB()->hasTerminator()) {
        cfg->getCurrentBB()->setTerminator(new TerminatorInstr(cfg->getCurrentBB(), mergeBB));
    }
    cfg->pop_bb();
    
    // Visit else block if present
    if (ctx->else_block()) {
        cfg->add_bb(elseBB);
        cfg->push_bb(elseBB);
        ifccParser::Else_blockContext* elseBlock = ctx->else_block();
        if (elseBlock->condition()) {
            this->visit(elseBlock->condition());
            // Recursive condition visit might have its own merge block.
            // But it MUST eventually jump to OUR mergeBB if it doesn't return.
            if (!cfg->getCurrentBB()->hasTerminator()) {
                cfg->getCurrentBB()->setTerminator(new TerminatorInstr(cfg->getCurrentBB(), mergeBB));
            }
        } else {
            this->visit(elseBlock->scope());
        }
        
        if (!cfg->getCurrentBB()->hasTerminator()) {
            cfg->getCurrentBB()->setTerminator(new TerminatorInstr(cfg->getCurrentBB(), mergeBB));
        }
        cfg->pop_bb();
    }
    
    // Continue in merge block
    cfg->add_bb(mergeBB);
    cfg->current_bb = mergeBB; // Set as default for next instructions
     cfg->getStackBBs().push_back(mergeBB); // Make it the current one on the stack
     
     
     return 0;
 }

antlrcpp::Any CodeGenVisitor::visitWhile_loop(ifccParser::While_loopContext *ctx)
{
    CFG* cfg = this->cfg;
    BasicBlock* currentBB = cfg->getCurrentBB();

    // Create basic blocks for the while loop: test, body, exit
    BasicBlock* testBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* bodyBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* exitBB = new BasicBlock(cfg, cfg->new_BB_name());

    // From current block, we go to the test block first (check condition before entering loop)
    currentBB->setTerminator(new TerminatorInstr(currentBB, testBB));

    // Test block: evaluate condition and branch
    cfg->add_bb(testBB);
    cfg->push_bb(testBB);
    string testVar = std::any_cast<string>(this->visit(ctx->expr())); // Visit the condition expression
    if (!cfg->getCurrentBB()->hasTerminator()) {
        cfg->getCurrentBB()->setTerminator(new TerminatorInstr(cfg->getCurrentBB(), testVar, bodyBB, exitBB));
    }
    cfg->pop_bb();

    // Body block: execute the body and then jump back to test
    cfg->add_bb(bodyBB);
    cfg->push_bb(bodyBB);
    this->visit(ctx->scope()); // Visit the body
    if (!cfg->getCurrentBB()->hasTerminator()) {
        cfg->getCurrentBB()->setTerminator(new TerminatorInstr(cfg->getCurrentBB(), testBB));
    }
    cfg->pop_bb();

    // Exit block: continue after the loop
    cfg->add_bb(exitBB);
    cfg->current_bb = exitBB; // Set as default for next instructions
    cfg->getStackBBs().push_back(exitBB); // Make it the current one on the stack

    return 0;
}
