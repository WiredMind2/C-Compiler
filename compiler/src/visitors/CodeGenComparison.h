#pragma once

#include "antlr4-runtime.h"
#include "../generated/ifccBaseVisitor.h"
#include "utils.h"

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
