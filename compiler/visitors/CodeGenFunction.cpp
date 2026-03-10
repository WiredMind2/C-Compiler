//
// Created by dupic on 06/03/2026.
//

#include "CodeGenFunction.h"
#include "../CodeGenVisitor.h"
#include <iostream>


antlrcpp::Any visitFunction_definition(CodeGenVisitor* visitor, ifccParser::Function_definitionContext *ctx) {
    std::string func_name = ctx->VARIABLES()->getText();
    std::cout << "    # Function definition: " << func_name << "\n";
}

antlrcpp::Any visitFunction_declaration(CodeGenVisitor* visitor, ifccParser::Function_declarationContext *ctx) {
    // Retrieve the function name from the context
    std::string func_name = ctx->VARIABLES()->getText();
    // TODO : handle different types
    Type returnType = INT;
    // Retrieve the parameters from the context
    std::vector<std::string> params;
    std::vector<Type> paramTypes;
    for (size_t i = 0; i < ctx->var_declarations_function().size(); ++i) {
        auto param_ctx = ctx->var_declarations_function(i);
        std::string param_name = param_ctx->VARIABLES()->getText();
        params.push_back(param_name);
        paramTypes.push_back(INT);
    }

    // Add the function to the symbol table
    visitor->getSymbolTable()->addFunction(func_name, returnType, paramTypes);




}

