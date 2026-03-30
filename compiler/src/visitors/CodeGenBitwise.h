#pragma once

#include "../generated/ifccBaseVisitor.h"
#include "antlr4-runtime.h"
#include "utils.h"

/**
 * CodeGenBitwise - Handles bitwise operations for code generation
 *
 * Methods:
 * - visitBitwiseORRule: Handles | operator
 * - visitBitwiseXORRule: Handles ^ operator
 * - visitBitwiseANDRule: Handles & operator
 * - visitUnaryNot: Handles ~ operator (unary not)
 */
class CodeGenVisitor;

antlrcpp::Any visitBitwiseORRule(CodeGenVisitor* visitor, ifccParser::BitwiseORRuleContext* ctx);
antlrcpp::Any visitBitwiseXORRule(CodeGenVisitor* visitor, ifccParser::BitwiseXORRuleContext* ctx);
antlrcpp::Any visitBitwiseANDRule(CodeGenVisitor* visitor, ifccParser::BitwiseANDRuleContext* ctx);
antlrcpp::Any visitUnaryNot(CodeGenVisitor* visitor, ifccParser::UnaryNotContext* ctx);
antlrcpp::Any visitUnaryBitNot(CodeGenVisitor* visitor, ifccParser::UnaryBitNotContext* ctx);
antlrcpp::Any visitShiftExprRef(CodeGenVisitor* visitor, ifccParser::ShiftExprRefContext* ctx);
antlrcpp::Any visitShiftLeft(CodeGenVisitor* visitor, ifccParser::ShiftLeftContext* ctx);
antlrcpp::Any visitShiftRight(CodeGenVisitor* visitor, ifccParser::ShiftRightContext* ctx);
