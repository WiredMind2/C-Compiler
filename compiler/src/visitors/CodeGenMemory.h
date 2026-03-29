#pragma once

#include "antlr4-runtime.h"
#include "../generated/ifccBaseVisitor.h"
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
 */
class CodeGenVisitor;

antlrcpp::Any visitConstant(CodeGenVisitor* visitor, ifccParser::ConstantContext *ctx);
antlrcpp::Any visitDouble_constant(CodeGenVisitor* visitor, ifccParser::Double_constantContext *ctx);
antlrcpp::Any visitChar_constant(CodeGenVisitor* visitor, ifccParser::Char_constantContext *ctx);
antlrcpp::Any visitVariable(CodeGenVisitor* visitor, ifccParser::VariableContext *ctx);
antlrcpp::Any visitDeclaration(CodeGenVisitor* visitor, ifccParser::DeclarationContext *ctx);
antlrcpp::Any visitAssignment(CodeGenVisitor* visitor, ifccParser::AssignmentContext *ctx);
antlrcpp::Any visitAddAssignment(CodeGenVisitor* visitor, ifccParser::AddAssignmentContext *ctx);
antlrcpp::Any visitSubAssignment(CodeGenVisitor* visitor, ifccParser::SubAssignmentContext *ctx);
antlrcpp::Any visitPreIncrement(CodeGenVisitor* visitor, ifccParser::PreIncrementContext *ctx);
antlrcpp::Any visitPreDecrement(CodeGenVisitor* visitor, ifccParser::PreDecrementContext *ctx);
antlrcpp::Any visitPostIncrement(CodeGenVisitor* visitor, ifccParser::PostIncrementContext *ctx);
antlrcpp::Any visitPostDecrement(CodeGenVisitor* visitor, ifccParser::PostDecrementContext *ctx);
