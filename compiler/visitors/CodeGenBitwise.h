#pragma once

#include "antlr4-runtime.h"
#include "../generated/ifccBaseVisitor.h"
#include "../IR.h"
#include "../SymbolTable.h"
#include <string>

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

antlrcpp::Any visitBitwiseORRule(CodeGenVisitor* visitor, ifccParser::BitwiseORRuleContext *ctx);
antlrcpp::Any visitBitwiseXORRule(CodeGenVisitor* visitor, ifccParser::BitwiseXORRuleContext *ctx);
antlrcpp::Any visitBitwiseANDRule(CodeGenVisitor* visitor, ifccParser::BitwiseANDRuleContext *ctx);
antlrcpp::Any visitUnaryNot(CodeGenVisitor* visitor, ifccParser::UnaryNotContext *ctx);
