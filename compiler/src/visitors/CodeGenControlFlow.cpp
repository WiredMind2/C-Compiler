#include "CodeGenControlFlow.h"
#include "CodeGenVisitor.h"
#include "../ir/IR.h"

antlrcpp::Any visitCondition(CodeGenVisitor* visitor, ifccParser::ConditionContext *ctx) {
    CFG* cfg = visitor->getCFG();
    BasicBlock* before_if_bb = cfg->current_bb;

    // Evaluate condition
    StackParam cond_res = std::any_cast<StackParam>(visitor->visit(ctx->expr()));
    before_if_bb->test_var_name = cond_res.name;

    BasicBlock* then_bb = new BasicBlock(cfg, cfg->new_BB_name());
    cfg->add_bb(then_bb);

    BasicBlock* else_bb = nullptr;
    if (ctx->else_block()) {
        else_bb = new BasicBlock(cfg, cfg->new_BB_name());
        cfg->add_bb(else_bb);
    }

    BasicBlock* merge_bb = new BasicBlock(cfg, cfg->new_BB_name());
    cfg->add_bb(merge_bb);

    // Set control flow from parent block
    before_if_bb->exit_true = then_bb;
    before_if_bb->exit_false = else_bb ? else_bb : merge_bb;

    // Generate THEN block
    cfg->current_bb = then_bb;
    visitor->visit(ctx->statement());
    // If the last block in the then-branch doesn't already end in a return or an unconditional jump,
    // add a jump to the merge block.
    if (cfg->current_bb != nullptr) {
        BasicBlock* lastThen = cfg->current_bb;
        if (lastThen->exit_true == nullptr &&
            (lastThen->instrs.empty() || dynamic_cast<RetInstr*>(lastThen->instrs.back()) == nullptr)) {
            lastThen->exit_true = merge_bb;
        }
    }

    // Generate ELSE block (optional)
    if (else_bb) {
        cfg->current_bb = else_bb;
        visitor->visit(ctx->else_block());
        if (cfg->current_bb != nullptr) {
            BasicBlock* lastElse = cfg->current_bb;
            if (lastElse->exit_true == nullptr &&
                (lastElse->instrs.empty() || dynamic_cast<RetInstr*>(lastElse->instrs.back()) == nullptr)) {
                lastElse->exit_true = merge_bb;
            }
        }
    }

    cfg->current_bb = merge_bb;
    return nullptr;
}

antlrcpp::Any visitWhile_loop(CodeGenVisitor* visitor, ifccParser::While_loopContext *ctx) {
    CFG* cfg = visitor->getCFG();
    BasicBlock* before_while_bb = cfg->current_bb;

    BasicBlock* test_bb = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock* end_bb = new BasicBlock(cfg, cfg->new_BB_name());
    cfg->add_bb(test_bb);
    cfg->add_bb(end_bb);

    // Save previous loop contexts
    BasicBlock* oldBreak = cfg->current_break_bb;
    BasicBlock* oldContinue = cfg->current_continue_bb;
    cfg->current_break_bb = end_bb;
    cfg->current_continue_bb = test_bb;

    // The block before the while jumps to the test block
    before_while_bb->exit_true = test_bb;

    // Evaluate condition
    cfg->current_bb = test_bb;
    StackParam cond_res = std::any_cast<StackParam>(visitor->visit(ctx->expr()));
    test_bb->test_var_name = cond_res.name;

    // Create a block for the body
    BasicBlock* body_bb = new BasicBlock(cfg, cfg->new_BB_name());
    cfg->add_bb(body_bb);

    // Set the test block's exits
    test_bb->exit_true = body_bb;   // if true, go to body
    test_bb->exit_false = end_bb;   // if false, go to end

    // Generate the body
    cfg->current_bb = body_bb;
    visitor->visit(ctx->scope());

    // Restore loop contexts
    cfg->current_break_bb = oldBreak;
    cfg->current_continue_bb = oldContinue;

    // After the body, if the last generated BB didn't already redirect control (return/break/continue),
    // set it to jump back to the test block.
    if (cfg->current_bb != nullptr) {
        BasicBlock* lastBody = cfg->current_bb;
        if (lastBody->exit_true == nullptr &&
            (lastBody->instrs.empty() || dynamic_cast<RetInstr*>(lastBody->instrs.back()) == nullptr)) {
            lastBody->exit_true = test_bb;
        }
    }

    // After the loop, we continue from end_bb
    cfg->current_bb = end_bb;

    return nullptr;
}

antlrcpp::Any visitBreak_stmt(CodeGenVisitor* visitor, ifccParser::Break_stmtContext *ctx) {
    CFG* cfg = visitor->getCFG();
    if (!cfg->current_break_bb) {
        std::cerr << "Error: 'break' statement not within a loop or switch." << std::endl;
        exit(1);
    }
    cfg->current_bb->exit_true = cfg->current_break_bb;
    cfg->current_bb = nullptr;
    return nullptr;
}

antlrcpp::Any visitContinue_stmt(CodeGenVisitor* visitor, ifccParser::Continue_stmtContext *ctx) {
    CFG* cfg = visitor->getCFG();
    if (!cfg->current_continue_bb) {
        std::cerr << "Error: 'continue' statement not within a loop." << std::endl;
        exit(1);
    }
    cfg->current_bb->exit_true = cfg->current_continue_bb;
    cfg->current_bb = nullptr;
    return nullptr;
}
