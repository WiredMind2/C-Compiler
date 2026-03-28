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
antlrcpp::Any visitVariable(CodeGenVisitor* visitor, ifccParser::VariableContext *ctx);
antlrcpp::Any visitDeclaration_list(CodeGenVisitor* visitor, ifccParser::Declaration_listContext *ctx);
antlrcpp::Any visitVar_decl(CodeGenVisitor* visitor, ifccParser::Var_declContext *ctx);
antlrcpp::Any visitVar_decl_with_init(CodeGenVisitor* visitor, ifccParser::Var_decl_with_initContext *ctx);
antlrcpp::Any visitAssignment(CodeGenVisitor* visitor, ifccParser::AssignmentContext *ctx);
