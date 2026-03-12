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

// Condition handler - handles if-else and else-if chains
antlrcpp::Any CodeGenVisitor::visitCondition(ifccParser::ConditionContext *ctx)
{
    CFG* cfg = this->cfg;
    BasicBlock* currentBB = cfg->getCurrentBB();
    
    // Create basic blocks for the if statement
    BasicBlock* condBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* thenBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* mergeBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* elseBB = nullptr;
    
    // Add all basic blocks to CFG
    cfg->add_bb(condBB);
    cfg->add_bb(thenBB);
    cfg->add_bb(mergeBB);
    
    // Connect currentBB to condBB with JMP (was previously FALLTHROUGH but let's be explicit)
    if (!currentBB->hasTerminator()) {
        currentBB->setTerminator(new TerminatorInstr(currentBB, condBB));
    }
    
    // Push condBB onto stack and evaluate the condition
    cfg->push_bb(condBB);
    string testVar = std::any_cast<string>(this->visit(ctx->expr()));
    condBB->test_var_name = testVar;  // Keep for backward compatibility
    
    // Handle else block if present
    if (ctx->else_block()) {
        elseBB = new BasicBlock(cfg, cfg->new_BB_name());
        cfg->add_bb(elseBB);
        
        ifccParser::Else_blockContext* elseBlock = ctx->else_block();
        if (elseBlock->condition()) {
            // else-if chain
            if (!condBB->hasTerminator()) {
                condBB->setTerminator(new TerminatorInstr(condBB, testVar, thenBB, elseBB));
            }
            
            cfg->push_bb(thenBB);
            this->visit(ctx->scope());
            if (!cfg->getCurrentBB()->hasTerminator()) {
                cfg->getCurrentBB()->setTerminator(new TerminatorInstr(cfg->getCurrentBB(), mergeBB));
            }
            cfg->pop_bb();
            
            cfg->push_bb(elseBB);
            this->visit(elseBlock->condition());
            // The inner visit(condition) will handle its own terminators and potentially merge to ITS mergeBB.
            // But we need to make sure our elseBB (which is the beginning of the next 'if(cond)')
            // eventually reaches OUR mergeBB if it doesn't return.
            // This is tricky with recursive 'if'.
            cfg->pop_bb();
            
            cfg->push_bb(mergeBB);
            
            return 0;
        } else {
            // Simple else
            if (!condBB->hasTerminator()) {
                condBB->setTerminator(new TerminatorInstr(condBB, testVar, thenBB, elseBB));
            }
            
            cfg->push_bb(thenBB);
            this->visit(ctx->scope());
            if (!cfg->getCurrentBB()->hasTerminator()) {
                cfg->getCurrentBB()->setTerminator(new TerminatorInstr(cfg->getCurrentBB(), mergeBB));
            }
            cfg->pop_bb();
            
            cfg->push_bb(elseBB);
            this->visit(elseBlock->scope());
            if (!cfg->getCurrentBB()->hasTerminator()) {
                cfg->getCurrentBB()->setTerminator(new TerminatorInstr(cfg->getCurrentBB(), mergeBB));
            }
            cfg->pop_bb();
        }
    } else {
        // No else
        if (!condBB->hasTerminator()) {
            condBB->setTerminator(new TerminatorInstr(condBB, testVar, thenBB, mergeBB));
        }
        
        cfg->push_bb(thenBB);
        this->visit(ctx->scope());
        if (!cfg->getCurrentBB()->hasTerminator()) {
            cfg->getCurrentBB()->setTerminator(new TerminatorInstr(cfg->getCurrentBB(), mergeBB));
        }
        cfg->pop_bb();
    }
    
    cfg->push_bb(mergeBB);
    
    return 0;
}
