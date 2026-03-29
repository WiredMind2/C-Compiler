#include "CodeGenVisitor.h"
#include "CodeGenArithmetic.h"
#include "../ir/IRInstr.h"
#include <limits>

namespace {
// Simple tokenizer and recursive-descent evaluator for case label expression text.
// Accepts integers, character literals, parentheses, and operators + - * / %.
// Rejects identifiers, floating-point literals and function calls.

struct Token {
    enum Type {NUM, CHAR, PLUS, MINUS, MUL, DIV, MOD, LPAREN, RPAREN, END} type;
    int64_t value;
};

static int64_t parse_char_literal_text(const std::string &s, size_t &i) {
    // s[i] should be '\''
    if (s[i] != '\'') throw std::runtime_error("invalid char literal");
    i++; // skip '
    if (i >= s.size()) throw std::runtime_error("unterminated char literal");
    int64_t v;
    if (s[i] == '\\') {
        i++;
        if (i >= s.size()) throw std::runtime_error("unterminated escape in char literal");
        char esc = s[i++];
        switch (esc) {
            case 'n': v = '\n'; break;
            case 't': v = '\t'; break;
            case 'r': v = '\r'; break;
            case '\\': v = '\\'; break;
            case '\'': v = '\''; break;
            case '0': v = '\0'; break;
            default: throw std::runtime_error("unsupported escape in char literal");
        }
    } else {
        v = static_cast<unsigned char>(s[i++]);
    }
    if (i >= s.size() || s[i] != '\'') throw std::runtime_error("unterminated char literal");
    i++; // closing '
    return v;
}

static std::vector<Token> tokenize_case_text(const std::string &s) {
    std::vector<Token> out;
    size_t i = 0, n = s.size();
    while (i < n) {
        char c = s[i];
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') { i++; continue; }
        if (c == '+') { out.push_back({Token::PLUS, 0}); i++; continue; }
        if (c == '-') { out.push_back({Token::MINUS, 0}); i++; continue; }
        if (c == '*') { out.push_back({Token::MUL, 0}); i++; continue; }
        if (c == '/') { out.push_back({Token::DIV, 0}); i++; continue; }
        if (c == '%') { out.push_back({Token::MOD, 0}); i++; continue; }
        if (c == '(') { out.push_back({Token::LPAREN, 0}); i++; continue; }
        if (c == ')') { out.push_back({Token::RPAREN, 0}); i++; continue; }
        if (c == '\'') {
            int64_t val = parse_char_literal_text(s, i);
            out.push_back({Token::CHAR, val});
            continue;
        }
        if ((c >= '0' && c <= '9')) {
            size_t j = i;
            while (j < n && isdigit((unsigned char)s[j])) j++;
            std::string num = s.substr(i, j - i);
            // reject floating point literals
            if (j < n && s[j] == '.') throw std::runtime_error("floating constant not allowed in case label");
            int64_t v = std::stoll(num);
            out.push_back({Token::NUM, v});
            i = j;
            continue;
        }
        // identifiers or other characters (like letters) are not allowed in constant case labels
        throw std::runtime_error("non-constant expression in case label");
    }
    out.push_back({Token::END, 0});
    return out;
}

struct Parser {
    const std::vector<Token> &toks;
    size_t pos;
    Parser(const std::vector<Token> &v) : toks(v), pos(0) {}

    Token peek() const { return toks[pos]; }
    Token consume() { return toks[pos++]; }

    int64_t parsePrimary() {
        Token tk = peek();
        if (tk.type == Token::NUM) { consume(); return tk.value; }
        if (tk.type == Token::CHAR) { consume(); return tk.value; }
        if (tk.type == Token::LPAREN) {
            consume(); int64_t v = parseAdd();
            if (peek().type != Token::RPAREN) throw std::runtime_error("missing closing parenthesis in case label");
            consume();
            return v;
        }
        throw std::runtime_error("invalid primary in case label");
    }

    int64_t parseUnary() {
        Token tk = peek();
        if (tk.type == Token::PLUS) { consume(); return parseUnary(); }
        if (tk.type == Token::MINUS) { consume(); return -parseUnary(); }
        return parsePrimary();
    }

    int64_t parseMul() {
        int64_t lhs = parseUnary();
        while (true) {
            Token tk = peek();
            if (tk.type == Token::MUL) { consume(); int64_t rhs = parseUnary(); lhs *= rhs; }
            else if (tk.type == Token::DIV) { consume(); int64_t rhs = parseUnary(); if (rhs == 0) throw std::runtime_error("division by zero in case label"); lhs /= rhs; }
            else if (tk.type == Token::MOD) { consume(); int64_t rhs = parseUnary(); if (rhs == 0) throw std::runtime_error("modulo by zero in case label"); lhs %= rhs; }
            else break;
        }
        return lhs;
    }

    int64_t parseAdd() {
        int64_t lhs = parseMul();
        while (true) {
            Token tk = peek();
            if (tk.type == Token::PLUS) { consume(); int64_t rhs = parseMul(); lhs += rhs; }
            else if (tk.type == Token::MINUS) { consume(); int64_t rhs = parseMul(); lhs -= rhs; }
            else break;
        }
        return lhs;
    }
};

static int64_t eval_case_from_text(const std::string &text) {
    auto toks = tokenize_case_text(text);
    Parser p(toks);
    int64_t v = p.parseAdd();
    if (p.peek().type != Token::END) throw std::runtime_error("trailing tokens in case label");
    return v;
}

bool fits_switch_type(IRType t, int64_t value) {
    switch (t) {
        case IRType::INT8:
            return value >= std::numeric_limits<int8_t>::min() && value <= std::numeric_limits<int8_t>::max();
        case IRType::INT32:
            return value >= std::numeric_limits<int32_t>::min() && value <= std::numeric_limits<int32_t>::max();
        case IRType::INT64:
            return true;
        default:
            return false;
    }
}
}

antlrcpp::Any CodeGenVisitor::visitProg(ifccParser::ProgContext *ctx)
{
    // First pass: pre-register top-level function signatures so forward calls resolve.
    for (auto stmt : ctx->statement()) {
        if (auto* def = stmt->function_definition()) {
            std::string func_name = def->VAR()->getText();
            if (cfg->get_function(func_name) != nullptr) {
                continue;
            }

            IRType return_type = irtype_from_string(def->type_specifier()->getText());
            std::vector<IRType> paramTypes;
            std::vector<std::string> paramNames;
            if (def->param_list()) {
                for (auto param : def->param_list()->param()) {
                    paramTypes.push_back(irtype_from_string(param->type_specifier()->getText()));
                    paramNames.push_back(param->VAR()->getText());
                }
            }
            cfg->add_function(func_name, return_type, paramTypes, paramNames);
        } else if (auto* decl = stmt->function_declaration()) {
            std::string func_name = decl->VAR()->getText();
            if (cfg->get_function(func_name) != nullptr) {
                continue;
            }

            IRType return_type = irtype_from_string(decl->type_specifier()->getText());
            std::vector<IRType> paramTypes;
            std::vector<std::string> paramNames;
            if (decl->param_list()) {
                for (auto param : decl->param_list()->param()) {
                    paramTypes.push_back(irtype_from_string(param->type_specifier()->getText()));
                    paramNames.push_back(param->VAR()->getText());
                }
            }
            cfg->add_function(func_name, return_type, paramTypes, paramNames);
        }
    }

    for (auto stmt : ctx->statement()) {
        this->visit(stmt);
    }

    if (cfg->get_function("main") == nullptr) {
        std::cerr << "error: 'main' function not defined" << std::endl;
        exit(1);
    }

    std::cerr << "BBs: " << cfg->getBBs().size() << std::endl; for(auto bb : cfg->getBBs()) { std::cerr << "BB " << bb->label << " has " << bb->instrs.size() << " instructions" << std::endl; } return "0";
}

antlrcpp::Any CodeGenVisitor::visitReturn_stmt(ifccParser::Return_stmtContext *ctx)
{
    auto* bb = cfg->current_bb;
    const string current_function_name = cfg->getCurrentFunction();
    CFG::FunctionSignature* current_function_signature = cfg->get_function(current_function_name);
    const IRType current_function_return_type = current_function_signature->returnType;

    // when returning something, as opposed to "return ;"
    if (ctx->expr() != nullptr) {
        StackParam var = any_cast_to_stack_param_or_throw_on_nullptr(this->visit(ctx->expr()));

        // GCC accepts "return expr;" in void functions as an extension.
        // Evaluate expression for side-effects, then return without a value.
        if (current_function_return_type == IRType::VOID) {
            bb->add_IRInstr(new RetInstr(bb, IRType::VOID));
            return nullptr;
        }

        if (var.type != current_function_return_type) {
            bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, var.name, var.type));
            bb->generate_conversion_instruction(Reg::W0, var.type, Reg::W1, current_function_return_type);
            bb->add_IRInstr(new CopyRegInstr(bb, Reg::RET, Reg::W1, current_function_return_type));
        } else {
            bb->add_IRInstr(new LoadStackInstr(bb, Reg::RET, var.name, var.type));
        }
        bb->add_IRInstr(new RetInstr(bb, current_function_return_type));
    }
    // empty return
    else {
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

antlrcpp::Any CodeGenVisitor::visitChar_constant(ifccParser::Char_constantContext* ctx) {
    return ::visitChar_constant(this, ctx);
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
    CFG* cfg = this->cfg;
    BasicBlock* preBB = cfg->current_bb;

    BasicBlock* scopeBB = new BasicBlock(cfg, cfg->new_BB_name());
    scopeBB->functionName = preBB->functionName;
    cfg->add_bb(scopeBB);

    preBB->exit_true = scopeBB;

    cfg->current_bb = scopeBB;
    cfg->getStackBBs().push_back(scopeBB);

    // Si on est dans un switch (decl_target_bb actif), un scope explicite { }
    // crée son propre scope indépendant : on suspend decl_target_bb le temps de ce bloc et on le restaure en sortant.
    BasicBlock* savedDeclTarget = cfg->decl_target_bb;
    cfg->decl_target_bb = nullptr;

    for (auto stmt : ctx->statement()) {
        this->visit(stmt);
    }

    cfg->decl_target_bb = savedDeclTarget;
    cfg->getStackBBs().pop_back();

    BasicBlock* afterBB = new BasicBlock(cfg, cfg->new_BB_name());
    afterBB->functionName = preBB->functionName;
    cfg->add_bb(afterBB);

    BasicBlock* lastBB = cfg->current_bb;
    if (lastBB->exit_true == nullptr &&
        (lastBB->instrs.empty() ||
         dynamic_cast<RetInstr*>(lastBB->instrs.back()) == nullptr)) {
        lastBB->exit_true = afterBB;
         }

    cfg->current_bb = afterBB;
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

    // Find nearest enclosing loop OR switch block and jump to its break target.
    BasicBlock* currentBB = cfg->current_bb;
    BasicBlock* targetBB = nullptr;
    std::vector<BasicBlock*> bbStack = cfg->getStackBBs();
    for (auto it = bbStack.rbegin(); it != bbStack.rend(); ++it) {
        BasicBlock* bb = *it;
        if (bb->loop_break_target != nullptr) {
            targetBB = bb->loop_break_target;
            break;
        }
    }

    if (targetBB) {
        currentBB->exit_true  = targetBB;
        currentBB->exit_false = nullptr;
        currentBB->test_var_name = "";
    } else {
        std::cerr << "Error: 'break' statement not within a loop or switch." << std::endl;
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

antlrcpp::Any CodeGenVisitor::visitSwitch_stmt(ifccParser::Switch_stmtContext *ctx) {
    CFG* cfg = this->cfg;
    StackParam switchExpr = std::any_cast<StackParam>(this->visit(ctx->expr()));

    if (switchExpr.type != IRType::INT8 && switchExpr.type != IRType::INT32 && switchExpr.type != IRType::INT64) {
        std::cerr << "Error: switch expression must be of integer type." << std::endl;
        exit(1);
    }

    BasicBlock* afterBB = new BasicBlock(cfg, cfg->new_BB_name());
    cfg->add_bb(afterBB);

    int nCases = static_cast<int>(ctx->case_block().size());
    bool hasDefault = (ctx->default_block() != nullptr);

    std::vector<int64_t> caseValues;
    for (auto caseBlockCtx : ctx->case_block()) {
        int64_t caseValue;
        try {
            caseValue = eval_case_from_text(caseBlockCtx->expr()->getText());
        } catch (const std::exception& e) {
            std::cerr << "Error: invalid case label expression: " << e.what() << std::endl;
            exit(1);
        }

        if (!fits_switch_type(switchExpr.type, caseValue)) {
            std::cerr << "Error: case value " << caseValue << " is not compatible with switch expression type." << std::endl;
            exit(1);
        }

        if (std::find(caseValues.begin(), caseValues.end(), caseValue) != caseValues.end()) {
            std::cerr << "Error: duplicate case value " << caseValue
                      << " in switch statement." << std::endl;
            exit(1);
        }
        caseValues.push_back(caseValue);
    }

    BasicBlock* scopeBB = new BasicBlock(cfg, cfg->new_BB_name());
    scopeBB->functionName = cfg->current_bb->functionName;
    scopeBB->loop_break_target = afterBB;
    cfg->add_bb(scopeBB);

    std::vector<BasicBlock*> caseBBs;
    for (int i = 0; i < nCases; i++) {
        BasicBlock* bb = new BasicBlock(cfg, cfg->new_BB_name());
        bb->functionName = scopeBB->functionName;
        cfg->add_bb(bb);
        caseBBs.push_back(bb);
    }

    BasicBlock* defaultBB = nullptr;
    if (hasDefault) {
        defaultBB = new BasicBlock(cfg, cfg->new_BB_name());
        defaultBB->functionName = scopeBB->functionName;
        cfg->add_bb(defaultBB);
    }

    BasicBlock* dispatchBB = cfg->current_bb;

    for (int i = 0; i < nCases; i++) {
        dispatchBB->add_IRInstr(
            new LdConstInstr(dispatchBB, Reg::W1, switchExpr.type,
                             static_cast<int64_t>(caseValues[i])));
        dispatchBB->add_IRInstr(
            new LoadStackInstr(dispatchBB, Reg::W0, switchExpr.name, switchExpr.type));
        dispatchBB->add_IRInstr(
            new CmpEqInstr(dispatchBB, Reg::W0, Reg::W0, Reg::W1, switchExpr.type));

        std::string cmpTmp = dispatchBB->create_new_tempvar(IRType::INT32);
        dispatchBB->add_IRInstr(
            new StoreStackInstr(dispatchBB, cmpTmp, Reg::W0, IRType::INT32));

        dispatchBB->test_var_name = cmpTmp;
        dispatchBB->exit_true     = caseBBs[i];

        if (i < nCases - 1) {
            BasicBlock* nextDispatch = new BasicBlock(cfg, cfg->new_BB_name());
            cfg->add_bb(nextDispatch);
            dispatchBB->exit_false = nextDispatch;
            dispatchBB = nextDispatch;
        } else {
            dispatchBB->exit_false = defaultBB ? defaultBB : afterBB;
        }
    }

    if (nCases == 0) {
        dispatchBB->exit_true  = defaultBB ? defaultBB : afterBB;
        dispatchBB->exit_false = nullptr;
    }

    cfg->getStackBBs().push_back(scopeBB);
    cfg->decl_target_bb = scopeBB;

    for (int i = 0; i < nCases; i++) {
        BasicBlock* fallThrough = (i + 1 < nCases) ? caseBBs[i + 1]
                                : (hasDefault       ? defaultBB
                                                    : afterBB);
        cfg->current_bb = caseBBs[i];

        for (auto stmt : ctx->case_block(i)->statement()) {
            this->visit(stmt);
        }

        BasicBlock* lastBB = cfg->current_bb;
        if (lastBB->exit_true == nullptr &&
            (lastBB->instrs.empty() ||
             dynamic_cast<RetInstr*>(lastBB->instrs.back()) == nullptr)) {
            lastBB->exit_true = fallThrough;
             }
    }

    if (hasDefault) {
        cfg->current_bb = defaultBB;

        for (auto stmt : ctx->default_block()->statement()) {
            this->visit(stmt);
        }

        BasicBlock* lastBB = cfg->current_bb;
        if (lastBB->exit_true == nullptr &&
            (lastBB->instrs.empty() ||
             dynamic_cast<RetInstr*>(lastBB->instrs.back()) == nullptr)) {
            lastBB->exit_true = afterBB;
             }
    }

    cfg->decl_target_bb = nullptr;
    cfg->getStackBBs().pop_back();

    cfg->current_bb = afterBB;
    return 0;
}