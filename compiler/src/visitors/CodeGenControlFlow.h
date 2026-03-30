#pragma once

#include "../generated/ifccBaseVisitor.h"
#include "antlr4-runtime.h"

class CodeGenVisitor;

antlrcpp::Any visitCondition(CodeGenVisitor* visitor, ifccParser::ConditionContext* ctx);
antlrcpp::Any visitWhile_loop(CodeGenVisitor* visitor, ifccParser::While_loopContext* ctx);
antlrcpp::Any visitBreak_stmt(CodeGenVisitor* visitor, ifccParser::Break_stmtContext* ctx);
antlrcpp::Any visitContinue_stmt(CodeGenVisitor* visitor, ifccParser::Continue_stmtContext* ctx);
