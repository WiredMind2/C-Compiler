#pragma once

#include "antlr4-runtime.h"
#include "../generated/ifccBaseVisitor.h"
#include "../generated/ifccParser.h"
#include "utils.h"

/**
 * CodeGenMemory - Handles memory and variable operations for code generation
 *
 * Methods:
 * - visitConstant: Handles integer constants
 * - visitVariable: Handles variable references
 * - visitVar_decl: Handles variable declarations
 * - visitDeclaration_list: Handles multiple variable declarations
 * - visitVar_decl_with_init: Handles declaration with initialization
 * - visitAssignment: Handles variable assignments
 * - visitDereference: Handles dereferencing of pointers (indirection)
 * - visitAddressOf: Handles address-of operator for variables
 */
class CodeGenVisitor;

antlrcpp::Any visitConstant(CodeGenVisitor* visitor, ifccParser::ConstantContext *ctx);
antlrcpp::Any visitDoubleConstant(CodeGenVisitor* visitor, ifccParser::DoubleConstantContext *ctx);
antlrcpp::Any visitStringConstant(CodeGenVisitor* visitor, ifccParser::StringConstantContext *ctx);
antlrcpp::Any visitCharConstant(CodeGenVisitor* visitor, ifccParser::CharConstantContext *ctx);
antlrcpp::Any visitVariable(CodeGenVisitor* visitor, ifccParser::VariableContext *ctx);
// legacy var_decl_* functions removed; use declaration handlers below
antlrcpp::Any visitDeclaration_no_semi(CodeGenVisitor* visitor, ifccParser::Declaration_no_semiContext *ctx);
antlrcpp::Any visitDeclaration(CodeGenVisitor* visitor, ifccParser::DeclarationContext *ctx);
antlrcpp::Any visitAssignment(CodeGenVisitor* visitor, ifccParser::AssignmentContext *ctx);
antlrcpp::Any visitDereference(CodeGenVisitor* visitor, ifccParser::DereferenceContext *ctx);
antlrcpp::Any visitAddressOf(CodeGenVisitor* visitor, ifccParser::AddressOfContext *ctx);
antlrcpp::Any visitArray_subscript(CodeGenVisitor* visitor, ifccParser::Array_subscriptContext *ctx);

antlrcpp::Any visitAddAssignment(CodeGenVisitor* visitor, ifccParser::AddAssignmentContext *ctx);
antlrcpp::Any visitSubAssignment(CodeGenVisitor* visitor, ifccParser::SubAssignmentContext *ctx);
antlrcpp::Any visitPreIncrement(CodeGenVisitor* visitor, ifccParser::PreIncrementContext *ctx);
antlrcpp::Any visitPreDecrement(CodeGenVisitor* visitor, ifccParser::PreDecrementContext *ctx);
antlrcpp::Any visitPostIncrement(CodeGenVisitor* visitor, ifccParser::PostIncrementContext *ctx);
antlrcpp::Any visitPostDecrement(CodeGenVisitor* visitor, ifccParser::PostDecrementContext *ctx);

