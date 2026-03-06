#pragma once

#include "antlr4-runtime.h"
#include "../generated/ifccBaseVisitor.h"
#include "../IR.h"
#include "../SymbolTable.h"
#include <string>

/**
 * CodeGenMemory - Handles memory and variable operations for code generation
 * 
 * Methods:
 * - visitConstant: Handles integer constants
 * - visitVariable: Handles variable references
 * - visitDeclaration: Handles variable declarations
 * - visitAssignment: Handles variable assignments
 * - visitDeclaration_assignement: Handles declaration with initialization
 */
class CodeGenVisitor;

antlrcpp::Any visitConstant(CodeGenVisitor* visitor, ifccParser::ConstantContext *ctx);
antlrcpp::Any visitVariable(CodeGenVisitor* visitor, ifccParser::VariableContext *ctx);
antlrcpp::Any visitDeclaration(CodeGenVisitor* visitor, ifccParser::DeclarationContext *ctx);
antlrcpp::Any visitAssignment(CodeGenVisitor* visitor, ifccParser::AssignmentContext *ctx);
antlrcpp::Any visitDeclaration_assignement(CodeGenVisitor* visitor, ifccParser::Declaration_assignementContext *ctx);
