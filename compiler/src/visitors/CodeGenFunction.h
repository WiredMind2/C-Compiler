#pragma once

#include <string>

#include "../generated/ifccBaseVisitor.h"
#include "antlr4-runtime.h"
#include "utils.h"

/********** CodeGenFunction - Handles function definitions and calls for code generation **********/

class CodeGenVisitor;

enum StandardLibraryFunction { GETCHAR, PUTCHAR, MALLOC, UNKNOWN };
antlrcpp::Any visitFunction_definition(CodeGenVisitor* visitor, ifccParser::Function_definitionContext* ctx);
antlrcpp::Any visitFunction_declaration(CodeGenVisitor* visitor, ifccParser::Function_declarationContext* ctx);
antlrcpp::Any visitFunctionCall(CodeGenVisitor* visitor, ifccParser::Function_callContext* ctx);
int isFunctionStandardLibrary(CodeGenVisitor* visitor, const std::string& func_name, ifccParser::Function_callContext* ctx);
StandardLibraryFunction getStandardLibraryFunction(const std::string& func_name);