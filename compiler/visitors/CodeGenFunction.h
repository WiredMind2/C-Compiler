#pragma once

#include "antlr4-runtime.h"
#include "../generated/ifccBaseVisitor.h"
#include "../IR.h"
#include "../SymbolTable.h"
#include <string>


/********** CodeGenFunction - Handles function definitions and calls for code generation **********/

class CodeGenVisitor;
antlrcpp::Any visitFunction_definition(CodeGenVisitor* visitor, ifccParser::Function_definitionContext *ctx);
antlrcpp::Any visitFunction_declaration(CodeGenVisitor* visitor, ifccParser::Function_declarationContext *ctx);
antlrcpp::Any visitFunctionCall(CodeGenVisitor* visitor, ifccParser::FunctionCallContext *ctx);