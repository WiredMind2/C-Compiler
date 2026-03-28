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
antlrcpp::Any visitDouble_constant(CodeGenVisitor* visitor, ifccParser::Double_constantContext *ctx);
antlrcpp::Any visitChar_constant(CodeGenVisitor* visitor, ifccParser::Char_constantContext *ctx);
antlrcpp::Any visitVariable(CodeGenVisitor* visitor, ifccParser::VariableContext *ctx);
antlrcpp::Any visitVar_decl_list(CodeGenVisitor* visitor, ifccParser::Var_decl_listContext *ctx);
antlrcpp::Any visitVar_decl_with_init(CodeGenVisitor* visitor, ifccParser::Var_decl_with_initContext *ctx);
antlrcpp::Any visitAssignment(CodeGenVisitor* visitor, ifccParser::AssignmentContext *ctx);
antlrcpp::Any visitDereference(CodeGenVisitor* visitor, ifccParser::DereferenceContext *ctx);
antlrcpp::Any visitAddressOf(CodeGenVisitor* visitor, ifccParser::AddressOfContext *ctx);
antlrcpp::Any visitArray_subscript(CodeGenVisitor* visitor, ifccParser::Array_subscriptContext *ctx);
