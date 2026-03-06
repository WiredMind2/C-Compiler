#pragma once

#include "antlr4-runtime.h"
#include "../generated/ifccBaseVisitor.h"
#include "../IR.h"
#include "../SymbolTable.h"
#include <string>

/**
 * CodeGenComparison - Handles comparison operations for code generation
 * 
 * Methods:
 * - visitEquals: Handles == operator
 * - visitDifferent: Handles != operator
 */
class CodeGenVisitor;

antlrcpp::Any visitEquals(CodeGenVisitor* visitor, ifccParser::EqualsContext *ctx);
antlrcpp::Any visitDifferent(CodeGenVisitor* visitor, ifccParser::DifferentContext *ctx);
