#pragma once

#include "antlr4-runtime.h"
#include "../generated/ifccBaseVisitor.h"
#include "../IR.h"
#include "../SymbolTable.h"
#include <string>

/**
 * CodeGenArithmetic - Handles arithmetic operations for code generation
 * 
 * Methods:
 * - visitAddition: Handles + operator
 * - visitSubstraction: Handles - operator
 * - visitMultiplication: Handles * operator
 * - visitDivision: Handles / operator
 * - visitUnaryPlus: Handles unary + operator
 * - visitUnaryMinus: Handles unary - operator
 */
class CodeGenVisitor;

antlrcpp::Any visitAddition(CodeGenVisitor* visitor, ifccParser::AdditionContext *ctx);
antlrcpp::Any visitSubstraction(CodeGenVisitor* visitor, ifccParser::SubstractionContext *ctx);
antlrcpp::Any visitMultiplication(CodeGenVisitor* visitor, ifccParser::MultiplicationContext *ctx);
antlrcpp::Any visitDivision(CodeGenVisitor* visitor, ifccParser::DivisionContext *ctx);
antlrcpp::Any visitUnaryPlus(CodeGenVisitor* visitor, ifccParser::UnaryPlusContext *ctx);
antlrcpp::Any visitUnaryMinus(CodeGenVisitor* visitor, ifccParser::UnaryMinusContext *ctx);
