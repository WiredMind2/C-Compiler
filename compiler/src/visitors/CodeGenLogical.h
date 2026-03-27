#pragma once

#include "antlr4-runtime.h"
#include "../generated/ifccBaseVisitor.h"
#include <string>
#include "utils.h"

/**
 * CodeGenLogical - Handles logical operations for code generation
 *
 * Methods:
 * - visitLogicalORRule: Handles || operator
 * - visitLogicalANDRule: Handles && operator
 * - visitLogicalORRef: Handles logical OR reference
 * - visitLogicalANDRef: Handles logical AND reference
 */
class CodeGenVisitor;

antlrcpp::Any visitLogicalORRule(CodeGenVisitor* visitor, ifccParser::LogicalORRuleContext *ctx);
antlrcpp::Any visitLogicalANDRule(CodeGenVisitor* visitor, ifccParser::LogicalANDRuleContext *ctx);
antlrcpp::Any visitLogicalORRef(CodeGenVisitor* visitor, ifccParser::LogicalORRefContext *ctx);
antlrcpp::Any visitLogicalANDRef(CodeGenVisitor* visitor, ifccParser::LogicalANDRefContext *ctx);
