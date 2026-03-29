#pragma once

#include "antlr4-runtime.h"
#include "../generated/ifccBaseVisitor.h"
#include <string>
#include "utils.h"

/**
 * CodeGenRelational - Handles relational operations for code generation
 *
 * Methods:
 * - visitSmallerStrictThan: Handles < operator
 * - visitGreaterStrictThan: Handles > operator
 * - visitSmallerThan: Handles <= operator
 * - visitGreaterThan: Handles >= operator
 * - visitRelationalExprRef: Handles relational expression reference
 */
class CodeGenVisitor;

antlrcpp::Any visitSmallerStrictThan(CodeGenVisitor* visitor, ifccParser::SmallerStrictThanContext *ctx);
antlrcpp::Any visitGreaterStrictThan(CodeGenVisitor* visitor, ifccParser::GreaterStrictThanContext *ctx);
antlrcpp::Any visitSmallerThan(CodeGenVisitor* visitor, ifccParser::SmallerThanContext *ctx);
antlrcpp::Any visitGreaterThan(CodeGenVisitor* visitor, ifccParser::GreaterThanContext *ctx);
antlrcpp::Any visitRelationalExprRef(CodeGenVisitor* visitor, ifccParser::RelationalExprRefContext *ctx);
