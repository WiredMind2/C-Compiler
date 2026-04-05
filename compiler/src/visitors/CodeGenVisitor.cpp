#include "CodeGenVisitor.h"

#include <cassert>
#include <limits>
#include <string>
#include <vector>
#include <functional>
#include <set>
#include <stdexcept>
#include <cctype>
#include <optional>
#include <cstdint>

static int64_t parse_char_literal_text_from_text(const std::string &text) {
    // text expected like '\'a\'' or '\'\\n\''
    if (text.size() < 2 || text.front() != '\'' || text.back() != '\'') throw std::runtime_error("invalid char literal");
    if (text[1] == '\\') {
        if (text.size() < 4) throw std::runtime_error("unterminated escape in char literal");
        char esc = text[2];
        switch (esc) {
            case 'n': return '\n';
            case 't': return '\t';
            case 'r': return '\r';
            case '\\': return '\\';
            case '\'': return '\'';
            case '0': return '\0';
            default: return esc;
        }
    } else {
        return static_cast<unsigned char>(text[1]);
    }
}

// Parse a `case_constant` parse node produced by the grammar. Returns nullopt on error.
static std::optional<int64_t> parse_case_constant(ifccParser::Case_constantContext *cc) {
    if (!cc) return std::nullopt;
    std::string s = cc->getText();
    if (s.empty()) return std::nullopt;

    // Char literal (possibly signed): e.g. 'a' or -'a'
    if (s[0] == '\'' || (s.size() > 1 && (s[0] == '+' || s[0] == '-') && s[1] == '\'')) {
        try {
            if (s[0] == '+' || s[0] == '-') {
                int sign = (s[0] == '-') ? -1 : 1;
                std::string inner = s.substr(1);
                int64_t v = parse_char_literal_text_from_text(inner);
                return sign * v;
            } else {
                return parse_char_literal_text_from_text(s);
            }
        } catch (...) { return std::nullopt; }
    }

    // Numeric constants: stoll with base 0 supports hex and decimal
    try {
        long long v = std::stoll(s, nullptr, 0);
        return static_cast<int64_t>(v);
    } catch (...) {
        return std::nullopt;
    }
}

#include "../ir/IRInstr.h"
#include "CodeGenArithmetic.h"


antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx) {
    for (auto stmt : ctx->statement()) {
        this->visit(stmt);
    }
    if (!cfg->get_function("main")) {
        throw std::runtime_error("Program does not contain a 'main' function.");
    }
    // No implicit function declarations: calls to undeclared functions are rejected
    return "0";
}

antlrcpp::Any CodeGenVisitor::visitReturn_stmt(ifccParser::Return_stmtContext *ctx) {
    auto *bb = cfg->current_bb;

    // when returning something, as opposed to "return ;"
    if (ctx->expr() != nullptr) {
        StackParam var = any_cast_to_stack_param_or_throw_on_nullptr(this->visit(ctx->expr()));

        const string current_function_name = cfg->getCurrentFunction();
        CFG::FunctionSignature *current_function_signature = cfg->get_function(current_function_name);
        const IRType current_function_return_type = current_function_signature->returnType;

        if (current_function_return_type == IRType::VOID) {
            std::cerr << "Error: 'return' with a value, in function returning void" << std::endl;
            exit(1);
        } else {
            if (var.type != current_function_return_type) {
                bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, var.name, var.type));
                bb->generate_conversion_instruction(Reg::W0, var.type, Reg::W1, current_function_return_type);
                bb->add_IRInstr(new CopyRegInstr(bb, Reg::RET, Reg::W1, current_function_return_type));
            } else {
                bb->add_IRInstr(new LoadStackInstr(bb, Reg::RET, var.name, var.type));
            }
            bb->add_IRInstr(new RetInstr(bb, var.type));
        }
    } else {
        const string current_function_name = cfg->getCurrentFunction();
        CFG::FunctionSignature *current_function_signature = cfg->get_function(current_function_name);
        const IRType current_function_return_type = current_function_signature->returnType;

        if (current_function_return_type != IRType::VOID) {
            std::cerr << "Warning: Cannot return no value for a non void returning function" << std::endl;
        }
        bb->add_IRInstr(new RetInstr(bb, current_function_return_type));
    }

    // Set exit_true to nullptr to indicate this block ends in a return
    bb->exit_true = nullptr;
    bb->exit_false = nullptr;
    // Marking block as terminated by return by leaving exit_true/exit_false null
    return nullptr;
}

antlrcpp::Any CodeGenVisitor::visitExpr(ifccParser::ExprContext *ctx) { return visit(ctx->sequential()); }

antlrcpp::Any CodeGenVisitor::visitParenthesis(ifccParser::ParenthesisContext *ctx) { return this->visit(ctx->expr()); }

antlrcpp::Any CodeGenVisitor::visitDecimalConstant(ifccParser::DecimalConstantContext *ctx) { return ::visitConstant(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitHexConstant(ifccParser::HexConstantContext *ctx) { return ::visitConstant(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitDoubleConstant(ifccParser::DoubleConstantContext *ctx) { return ::visitDoubleConstant(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitStringConstant(ifccParser::StringConstantContext *ctx) { return ::visitStringConstant(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitCharConstant(ifccParser::CharConstantContext *ctx) { return ::visitCharConstant(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitVariable(ifccParser::VariableContext *ctx) { return ::visitVariable(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitDeclaration_no_semi(ifccParser::Declaration_no_semiContext *ctx) { return ::visitDeclaration_no_semi(this, ctx); }
antlrcpp::Any CodeGenVisitor::visitDeclaration(ifccParser::DeclarationContext *ctx) {
    // delegate to the no-semi handler for shared logic
    if (ctx->declaration_no_semi()) return ::visitDeclaration_no_semi(this, ctx->declaration_no_semi());
    return 0;
}
antlrcpp::Any CodeGenVisitor::visitAssignment(ifccParser::AssignmentContext *ctx) { return ::visitAssignment(this, ctx); }

// Arithmetic expression handlers
antlrcpp::Any CodeGenVisitor::visitMultiplicativeExprRef(ifccParser::MultiplicativeExprRefContext *ctx) { return this->visit(ctx->multiplicative()); }

antlrcpp::Any CodeGenVisitor::visitAddition(ifccParser::AdditionContext *ctx) { return ::visitAddition(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitSubstraction(ifccParser::SubstractionContext *ctx) { return ::visitSubstraction(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitMultiplication(ifccParser::MultiplicationContext *ctx) { return ::visitMultiplication(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitDivision(ifccParser::DivisionContext *ctx) { return ::visitDivision(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitModulo(ifccParser::ModuloContext *ctx) { return ::visitModulo(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitUnaryMinus(ifccParser::UnaryMinusContext *ctx) { return ::visitUnaryMinus(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitUnaryPlus(ifccParser::UnaryPlusContext *ctx) { return ::visitUnaryPlus(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitUnaryNot(ifccParser::UnaryNotContext *ctx) { return ::visitUnaryNot(this, ctx); }
antlrcpp::Any CodeGenVisitor::visitUnaryBitNot(ifccParser::UnaryBitNotContext *ctx) { return ::visitUnaryBitNot(this, ctx); }
antlrcpp::Any CodeGenVisitor::visitDereference(ifccParser::DereferenceContext *ctx) { return ::visitDereference(this, ctx); }
antlrcpp::Any CodeGenVisitor::visitAddressOf(ifccParser::AddressOfContext *ctx) { return ::visitAddressOf(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitPrimitiveExprRef(ifccParser::PrimitiveExprRefContext *ctx) { return this->visit(ctx->primitive()); }

antlrcpp::Any CodeGenVisitor::visitFunctionCall(ifccParser::FunctionCallContext *ctx) { return this->visit(ctx->function_call()); }

antlrcpp::Any CodeGenVisitor::visitArray_subscript(ifccParser::Array_subscriptContext *ctx) { return ::visitArray_subscript(this, ctx); }

// Sequential / compound-assignment pass-throughs
antlrcpp::Any CodeGenVisitor::visitSequentialExprRef(ifccParser::SequentialExprRefContext *ctx) { return this->visit(ctx->compoundAssignment()); }

antlrcpp::Any CodeGenVisitor::visitSequentialRule(ifccParser::SequentialRuleContext *ctx) {
    this->visit(ctx->compoundAssignment());
    return this->visit(ctx->sequential());
}

antlrcpp::Any CodeGenVisitor::visitCompoundAssignmentRef(ifccParser::CompoundAssignmentRefContext *ctx) { return this->visit(ctx->logicalOR()); }

// Bitwise handlers
antlrcpp::Any CodeGenVisitor::visitBitwiseORRef(ifccParser::BitwiseORRefContext *ctx) { return this->visit(ctx->bitwiseXOR()); }

antlrcpp::Any CodeGenVisitor::visitBitwiseORRule(ifccParser::BitwiseORRuleContext *ctx) { return ::visitBitwiseORRule(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitBitwiseXORRef(ifccParser::BitwiseXORRefContext *ctx) { return this->visit(ctx->bitwiseAND()); }

antlrcpp::Any CodeGenVisitor::visitBitwiseXORRule(ifccParser::BitwiseXORRuleContext *ctx) { return ::visitBitwiseXORRule(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitBitwiseANDRef(ifccParser::BitwiseANDRefContext *ctx) { return this->visit(ctx->equality()); }

antlrcpp::Any CodeGenVisitor::visitBitwiseANDRule(ifccParser::BitwiseANDRuleContext *ctx) { return ::visitBitwiseANDRule(this, ctx); }

// Equality expression handlers
antlrcpp::Any CodeGenVisitor::visitEqualityExprRef(ifccParser::EqualityExprRefContext *ctx) { return this->visit(ctx->relational()); }

antlrcpp::Any CodeGenVisitor::visitEquals(ifccParser::EqualsContext *ctx) { return ::visitEquals(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitDifferent(ifccParser::DifferentContext *ctx) { return ::visitDifferent(this, ctx); }

// Relational expression handlers
antlrcpp::Any CodeGenVisitor::visitRelationalExprRef(ifccParser::RelationalExprRefContext *ctx) { return ::visitRelationalExprRef(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitSmallerStrictThan(ifccParser::SmallerStrictThanContext *ctx) { return ::visitSmallerStrictThan(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitGreaterStrictThan(ifccParser::GreaterStrictThanContext *ctx) { return ::visitGreaterStrictThan(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitSmallerThan(ifccParser::SmallerThanContext *ctx) { return ::visitSmallerThan(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitGreaterThan(ifccParser::GreaterThanContext *ctx) { return ::visitGreaterThan(this, ctx); }

// Shift handlers
antlrcpp::Any CodeGenVisitor::visitShiftExprRef(ifccParser::ShiftExprRefContext *ctx) { return this->visit(ctx->additive()); }

antlrcpp::Any CodeGenVisitor::visitShiftLeft(ifccParser::ShiftLeftContext *ctx) { return ::visitShiftLeft(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitShiftRight(ifccParser::ShiftRightContext *ctx) { return ::visitShiftRight(this, ctx); }

// Logical expression handlers
antlrcpp::Any CodeGenVisitor::visitLogicalORRef(ifccParser::LogicalORRefContext *ctx) { return ::visitLogicalORRef(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitLogicalORRule(ifccParser::LogicalORRuleContext *ctx) { return ::visitLogicalORRule(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitLogicalANDRef(ifccParser::LogicalANDRefContext *ctx) { return ::visitLogicalANDRef(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitLogicalANDRule(ifccParser::LogicalANDRuleContext *ctx) { return ::visitLogicalANDRule(this, ctx); }

// Function handlers
antlrcpp::Any CodeGenVisitor::visitFunction_definition(ifccParser::Function_definitionContext *ctx) { return ::visitFunction_definition(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitFunction_declaration(ifccParser::Function_declarationContext *ctx) {
    return ::visitFunction_declaration(this, ctx);
}

antlrcpp::Any CodeGenVisitor::visitFunction_call(ifccParser::Function_callContext *ctx) { return ::visitFunctionCall(this, ctx); }

// Scope handler - handles any { ... } block
antlrcpp::Any CodeGenVisitor::visitScope(ifccParser::ScopeContext *ctx) {
    CFG *cfg = this->cfg;
    BasicBlock *preBB = cfg->current_bb;

    BasicBlock *scopeBB = new BasicBlock(cfg, cfg->new_BB_name());
    scopeBB->functionName = preBB->functionName;
    cfg->add_bb(scopeBB);

    preBB->exit_true = scopeBB;

    cfg->current_bb = scopeBB;
    cfg->getStackBBs().push_back(scopeBB);

    // If we are in a switch (decl_target_bb active), an explicit scope { }
    // creates its own independent scope: we suspend decl_target_bb during this block and restore it on exit.
    BasicBlock *savedDeclTarget = cfg->decl_target_bb;
    cfg->decl_target_bb = nullptr;

    for (auto stmt : ctx->statement()) {
        this->visit(stmt);
    }

    cfg->decl_target_bb = savedDeclTarget;
    cfg->getStackBBs().pop_back();

    BasicBlock *afterBB = new BasicBlock(cfg, cfg->new_BB_name());
    afterBB->functionName = preBB->functionName;
    cfg->add_bb(afterBB);

    BasicBlock *lastBB = cfg->current_bb;
    if (lastBB != nullptr) {
        if (lastBB->exit_true == nullptr && (lastBB->instrs.empty() || dynamic_cast<RetInstr *>(lastBB->instrs.back()) == nullptr)) {
            lastBB->exit_true = afterBB;
        }
    }

    cfg->current_bb = afterBB;
    return 0;
}

antlrcpp::Any CodeGenVisitor::visitStatement(ifccParser::StatementContext *ctx) {
    auto *bb = cfg->current_bb;
    if (bb == nullptr) {
        return nullptr;  // Unreachable code
    }

    // Check if current block already has a return or an unconditional jump (break/continue)
    if (!bb->instrs.empty()) {
        if (dynamic_cast<RetInstr *>(bb->instrs.back()) != nullptr) {
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
antlrcpp::Any CodeGenVisitor::visitCondition(ifccParser::ConditionContext *ctx) {
    CFG *cfg = this->cfg;
    BasicBlock *currentBB = cfg->current_bb;

    // Create blocks for then-branch, else-branch (optional), and merge point
    BasicBlock *thenBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock *elseBB = nullptr;
    BasicBlock *mergeBB = new BasicBlock(cfg, cfg->new_BB_name());

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
    BasicBlock *lastThenBB = cfg->current_bb;
    // Add jump to merge block ONLY if it doesn't already have a return or break/continue
    if (lastThenBB->exit_true == nullptr && (lastThenBB->instrs.empty() || dynamic_cast<RetInstr *>(lastThenBB->instrs.back()) == nullptr)) {
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
        BasicBlock *lastElseBB = cfg->current_bb;
        // Add jump to merge block ONLY if it doesn't already have a return or break/continue
        if (lastElseBB->exit_true == nullptr && (lastElseBB->instrs.empty() || dynamic_cast<RetInstr *>(lastElseBB->instrs.back()) == nullptr)) {
            lastElseBB->exit_true = mergeBB;
        }
    }

    // Continue from merge block - this is where code continues after the if-else
    cfg->current_bb = mergeBB;

    return 0;
}

void CodeGenVisitor::generateLoopBody(ifccParser::ScopeContext *scopeCtx, BasicBlock *bodyBB, BasicBlock *continueTargetBB,
                                      BasicBlock *breakTargetBB) {
    // Store break/continue targets on bodyBB via dedicated fields
    bodyBB->loop_continue_target = continueTargetBB;
    bodyBB->loop_break_target = breakTargetBB;

    cfg->current_bb = bodyBB;
    BasicBlock *oldBreak = cfg->current_break_bb;
    BasicBlock *oldContinue = cfg->current_continue_bb;

    cfg->current_break_bb = breakTargetBB;
    cfg->current_continue_bb = continueTargetBB;

    cfg->getStackBBs().push_back(bodyBB);  // Push loop body onto stack so break/continue can find it
    if (scopeCtx) {
        this->visit(scopeCtx);
    }
    cfg->getStackBBs().pop_back();  // Pop loop body after visiting

    cfg->current_break_bb = oldBreak;
    cfg->current_continue_bb = oldContinue;

    // Automatically jump to condition/update block if not already jumping
    if (cfg->current_bb->exit_true == nullptr &&
        (cfg->current_bb->instrs.empty() || dynamic_cast<RetInstr *>(cfg->current_bb->instrs.back()) == nullptr)) {
        cfg->current_bb->exit_true = continueTargetBB;
    }
}

// While loop handler
antlrcpp::Any CodeGenVisitor::visitWhile_loop(ifccParser::While_loopContext *ctx) {
    CFG *cfg = this->cfg;
    BasicBlock *currentBB = cfg->current_bb;

    // Create blocks for condition check, loop body, and after loop
    BasicBlock *condBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock *bodyBB = new BasicBlock(cfg, cfg->new_BB_name(), true);  // Mark bodyBB as a loop block for break/continue handling
    BasicBlock *afterBB = new BasicBlock(cfg, cfg->new_BB_name());

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

    // Generate code for loop body
    generateLoopBody(ctx->scope(), bodyBB, condBB, afterBB);

    // Continue from after loop
    cfg->current_bb = afterBB;

    return 0;
}

antlrcpp::Any CodeGenVisitor::visitFor_loop(ifccParser::For_loopContext *ctx) {
    CFG *cfg = this->cfg;

    ifccParser::ExprContext *initExpr = nullptr;
    ifccParser::ExprContext *condExpr = nullptr;
    ifccParser::ExprContext *updateExpr = nullptr;

    std::vector<int> semicolonTokenIndices;
    if (!ctx->children.empty()) {
        for (auto *child : ctx->children) {
            auto *terminal = dynamic_cast<antlr4::tree::TerminalNode *>(child);
            if (terminal != nullptr && terminal->getText() == ";") {
                semicolonTokenIndices.push_back(terminal->getSymbol()->getTokenIndex());
            }
        }
    }

    if (semicolonTokenIndices.size() == 2) {
        // if there are exactly 2 semicolons, we can determine the expressions based on their positions
        for (auto *exprCtx : ctx->expr()) {
            int exprStart = exprCtx->getStart()->getTokenIndex();
            if (exprStart < semicolonTokenIndices[1]) {
                // This is the condition expression (between the first and second semicolon)
                condExpr = exprCtx;
            } else {
                // This is the update expression (after the second semicolon)
                updateExpr = exprCtx;
            }
        }
    } else {
        auto exprs = ctx->expr();
        if (exprs.size() > 0) condExpr = exprs[0];
        if (exprs.size() > 1) updateExpr = exprs[1];
    }

    //  init: either a declaration like 'int i = 0' (now in for_init) or an expression
    if (ctx->for_init() && ctx->for_init()->declaration_no_semi()) {
        BasicBlock *oldDeclTarget = cfg->decl_target_bb;
        cfg->decl_target_bb = cfg->current_bb;
        this->visit(ctx->for_init()->declaration_no_semi());
        cfg->decl_target_bb = oldDeclTarget;
    } else if (ctx->for_init() && ctx->for_init()->expr()) {
        this->visit(ctx->for_init()->expr());
    }

    // If init created extra blocks (for example a short-circuit expr), continue from the actual current BB.
    BasicBlock *initEndBB = cfg->current_bb;

    // Then create CFG blocks: cond -> body -> update -> cond, and after for exit.
    BasicBlock *condBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock *bodyBB = new BasicBlock(cfg, cfg->new_BB_name(), true);
    BasicBlock *updateBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock *afterBB = new BasicBlock(cfg, cfg->new_BB_name());

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
        BasicBlock *condEndBB = cfg->current_bb;
        condEndBB->test_var_name = condResult.name;
        condEndBB->exit_true = bodyBB;
        condEndBB->exit_false = afterBB;
    } else {
        // for(;;) or missing middle expression means an always-true loop.
        condBB->exit_true = bodyBB;
    }

    // body
    generateLoopBody(ctx->scope(), bodyBB, updateBB, afterBB);

    // update
    cfg->current_bb = updateBB;
    if (updateExpr != nullptr) {
        this->visit(updateExpr);
    }
    if (updateBB->exit_true == nullptr && (updateBB->instrs.empty() || dynamic_cast<RetInstr *>(updateBB->instrs.back()) == nullptr)) {
        updateBB->exit_true = condBB;
    }

    // continue after loop
    cfg->current_bb = afterBB;

    return 0;
}

antlrcpp::Any CodeGenVisitor::visitBreak_stmt(ifccParser::Break_stmtContext *ctx) { return ::visitBreak_stmt(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitContinue_stmt(ifccParser::Continue_stmtContext *ctx) { return ::visitContinue_stmt(this, ctx); }

antlrcpp::Any CodeGenVisitor::visitSwitch_stmt(ifccParser::Switch_stmtContext *ctx) {
    CFG *cfg = this->cfg;
    BasicBlock *preBB = cfg->current_bb;
    BasicBlock *endBB = new BasicBlock(cfg, cfg->new_BB_name());
    cfg->add_bb(endBB);

    // Save old break context and update it
    BasicBlock *oldBreak = cfg->current_break_bb;
    cfg->current_break_bb = endBB;

    // Selector evaluation
    StackParam selector = any_cast_to_stack_param_or_throw_on_nullptr(this->visit(ctx->expr()));

    // Variable allocation management
    BasicBlock *oldDeclTarget = cfg->decl_target_bb;
    cfg->decl_target_bb = preBB;

    // Cases collection
    struct CaseInfo {
        int64_t val; // compile-time evaluated value when available
        BasicBlock *bb;
        ifccParser::Case_blockContext *ctx;
    };
    std::vector<CaseInfo> cases;
    BasicBlock *defaultBB = nullptr;
    ifccParser::Default_blockContext *defaultCtx = nullptr;

    // Helper: parse a case_constant parse node. Grammar guarantees format is
    // optional '+'/'-' followed by DEC_CONST, HEX_CONST or CHAR_CONST (no spaces/paren).
    auto parse_case_constant = [&](ifccParser::Case_constantContext *cc) -> std::optional<int64_t> {
        if (!cc) return std::nullopt;
        std::string s = cc->getText();
        if (s.empty()) return std::nullopt;

        // Char literal (possibly with leading sign)
        if (s[0] == '\'' || (s.size() > 1 && (s[0] == '+' || s[0] == '-') && s[1] == '\'')) {
            try {
                // If signed char like -'a', strip sign and apply it afterwards
                if (s[0] == '+' || s[0] == '-') {
                    int sign = (s[0] == '-') ? -1 : 1;
                    std::string inner = s.substr(1);
                    int64_t v = parse_char_literal_text_from_text(inner);
                    return sign * v;
                } else {
                    return parse_char_literal_text_from_text(s);
                }
            } catch (...) { return std::nullopt; }
        }

        // For numeric constants (decimal or hex), std::stoll with base 0 handles 0x prefix
        try {
            long long v = std::stoll(s, nullptr, 0);
            return static_cast<int64_t>(v);
        } catch (...) {
            return std::nullopt;
        }
    };

    std::set<int64_t> seen_cases;
    for (int i = 0; i < ctx->case_block().size(); ++i) {
        auto comp = ctx->case_block(i);
        auto maybe = parse_case_constant(comp->case_constant());
        if (!maybe.has_value()) {
            std::cerr << "Error: invalid case label constant: '" << comp->case_constant()->getText() << "'" << std::endl;
            exit(1);
        }
        int64_t val = *maybe;
        if (seen_cases.count(val)) {
            std::cerr << "Error: duplicate case value '" << val << "' in switch statement." << std::endl;
            exit(1);
        }
        seen_cases.insert(val);
        BasicBlock *cbb = new BasicBlock(cfg, cfg->new_BB_name());
        cfg->add_bb(cbb);
        cases.push_back({val, cbb, comp});
    }
    if (ctx->default_block()) {
        defaultBB = new BasicBlock(cfg, cfg->new_BB_name());
        cfg->add_bb(defaultBB);
        defaultCtx = ctx->default_block();
    }

    // Selection logic
    BasicBlock *currentCheckBB = preBB;
    for (size_t i = 0; i < cases.size(); ++i) {
        BasicBlock *targetBB = cases[i].bb;
        auto *caseCtx = cases[i].ctx;

        // Evaluate the case label expression starting from the current check block.
        cfg->current_bb = currentCheckBB;
        BasicBlock *afterEvalBB = cfg->current_bb;

        string tmpCompResult = afterEvalBB->create_new_tempvar(IRType::INT32);
        afterEvalBB->add_IRInstr(new LoadStackInstr(afterEvalBB, Reg::W0, selector.name, selector.type));
        // Load literal constant for the case value (no runtime evaluation allowed)
        afterEvalBB->add_IRInstr(new LdConstInstr(afterEvalBB, Reg::W1, IRType::INT32, cases[i].val));
        afterEvalBB->add_IRInstr(new CmpEqInstr(afterEvalBB, Reg::W2, Reg::W0, Reg::W1, selector.type));
        afterEvalBB->add_IRInstr(new StoreStackInstr(afterEvalBB, tmpCompResult, Reg::W2, IRType::INT32));

        afterEvalBB->test_var_name = tmpCompResult;
        afterEvalBB->exit_true = targetBB;

        if (i < cases.size() - 1 || defaultBB != nullptr) {
            BasicBlock *nextCheck = new BasicBlock(cfg, cfg->new_BB_name());
            cfg->add_bb(nextCheck);
            afterEvalBB->exit_false = nextCheck;
            currentCheckBB = nextCheck;
        } else {
            afterEvalBB->exit_false = endBB;
        }
    }

    if (defaultBB) {
        currentCheckBB->exit_true = defaultBB;
    }

    // Body generation
    // We need to visit all blocks in order they appear in the source to handle fallthrough
    for (auto child : ctx->children) {
        auto *caseComp = dynamic_cast<ifccParser::Case_blockContext *>(child);
        auto *defComp = dynamic_cast<ifccParser::Default_blockContext *>(child);

        BasicBlock *compBB = nullptr;
        std::vector<ifccParser::StatementContext *> statements;

        if (caseComp) {
            for (auto &ci : cases)
                if (ci.ctx == caseComp) {
                    compBB = ci.bb;
                    break;
                }
            statements = caseComp->statement();
        } else if (defComp) {
            compBB = defaultBB;
            statements = defComp->statement();
        } else
            continue;

        cfg->current_bb = compBB;
        for (auto *stmt : statements) {
            this->visit(stmt);
        }

        if (cfg->current_bb != nullptr) {
            BasicBlock *lastBB = cfg->current_bb;
            if (lastBB->exit_true == nullptr && (lastBB->instrs.empty() || dynamic_cast<RetInstr *>(lastBB->instrs.back()) == nullptr)) {
                BasicBlock *nextCompBB = endBB;
                bool foundCurrent = false;
                for (auto *innerChild : ctx->children) {
                    if (foundCurrent) {
                        auto *innerCase = dynamic_cast<ifccParser::Case_blockContext *>(innerChild);
                        auto *innerDef = dynamic_cast<ifccParser::Default_blockContext *>(innerChild);
                        if (innerCase) {
                            for (auto &ci : cases)
                                if (ci.ctx == innerCase) {
                                    nextCompBB = ci.bb;
                                    break;
                                }
                            break;
                        } else if (innerDef) {
                            nextCompBB = defaultBB;
                            break;
                        }
                    }
                    if (innerChild == child) foundCurrent = true;
                }
                lastBB->exit_true = nextCompBB;
            }
        }
    }

    cfg->current_bb = endBB;
    cfg->decl_target_bb = oldDeclTarget;
    cfg->current_break_bb = oldBreak;
    return nullptr;
}

antlrcpp::Any CodeGenVisitor::visitAddAssignment(ifccParser::AddAssignmentContext *ctx) { return ::visitAddAssignment(this, ctx); }
antlrcpp::Any CodeGenVisitor::visitSubAssignment(ifccParser::SubAssignmentContext *ctx) { return ::visitSubAssignment(this, ctx); }
antlrcpp::Any CodeGenVisitor::visitMulAssignment(ifccParser::MulAssignmentContext *ctx) { return ::visitMulAssignment(this, ctx); }
antlrcpp::Any CodeGenVisitor::visitDivAssignment(ifccParser::DivAssignmentContext *ctx) { return ::visitDivAssignment(this, ctx); }
antlrcpp::Any CodeGenVisitor::visitPreIncrement(ifccParser::PreIncrementContext *ctx) { return ::visitPreIncrement(this, ctx); }
antlrcpp::Any CodeGenVisitor::visitPreDecrement(ifccParser::PreDecrementContext *ctx) { return ::visitPreDecrement(this, ctx); }
antlrcpp::Any CodeGenVisitor::visitPostIncrement(ifccParser::PostIncrementContext *ctx) { return ::visitPostIncrement(this, ctx); }
antlrcpp::Any CodeGenVisitor::visitPostDecrement(ifccParser::PostDecrementContext *ctx) { return ::visitPostDecrement(this, ctx); }
// Do while loop handler
antlrcpp::Any CodeGenVisitor::visitDo_while_loop(ifccParser::Do_while_loopContext *ctx) {
    CFG *cfg = this->cfg;
    BasicBlock *currentBB = cfg->current_bb;

    // Create blocks for loop body, condition check, and after loop
    BasicBlock *bodyBB = new BasicBlock(cfg, cfg->new_BB_name(), true);  // Mark bodyBB as a loop block for break/continue handling
    BasicBlock *condBB = new BasicBlock(cfg, cfg->new_BB_name());
    BasicBlock *afterBB = new BasicBlock(cfg, cfg->new_BB_name());

    // Add blocks to CFG
    cfg->add_bb(bodyBB);
    cfg->add_bb(condBB);
    cfg->add_bb(afterBB);

    // Set up current block to unconditionally jump to loop body initially
    currentBB->exit_true = bodyBB;

    // Generate code for loop body
    generateLoopBody(ctx->scope(), bodyBB, condBB, afterBB);

    // Set up condition block
    cfg->current_bb = condBB;

    // The do-while grammar guarantees expr() is always present; treat null as a hard error.
    assert(ctx->expr() && "do-while condition expression must not be null");
    StackParam condResult = any_cast_to_stack_param_or_throw_on_nullptr(this->visit(ctx->expr()));

    // Condition result determines whether to loop again or exit
    condBB->test_var_name = condResult.name;
    condBB->exit_true = bodyBB;
    condBB->exit_false = afterBB;

    // Set the current block to the block after the loop
    cfg->current_bb = afterBB;

    return 0;
}
