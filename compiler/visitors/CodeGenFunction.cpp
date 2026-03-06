//
// Created by dupic on 06/03/2026.
//

#include "CodeGenFunction.h"
#include "../CodeGenVisitor.h"
#include <iostream>


antlrcpp::Any visitFunction_definition(CodeGenVisitor* visitor, ifccParser::Function_definitionContext *ctx) {
    std::string func_name = ctx->VARIABLES()->getText();
    std::cout << "    # Function definition: " << func_name << "\n";


