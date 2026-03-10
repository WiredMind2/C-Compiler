//
// Created by dupic on 06/03/2026.
//

#include "CodeGenFunction.h"
#include "../CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitFunction_definition(CodeGenVisitor* visitor, ifccParser::Function_definitionContext *ctx) {
    std::string func_name = ctx->VAR()->getText();
    
    // Get parameter types and names from var_declarations_function
    std::vector<Type> paramTypes;
    std::vector<std::string> paramNames;
    
    auto params = ctx->var_declarations_function();
    if (!params.empty()) {
        for (auto param : params) {
            paramTypes.push_back(INT);  // All parameters are int in our grammar
            paramNames.push_back(param->VAR()->getText());
        }
    }
    
    // Create function entry point
    BasicBlock* entryBB = visitor->getCFG()->create_function_entry(func_name, INT, paramTypes, paramNames);
    
    // Visit the function body (scope)
    if (ctx->scope()) {
        visitor->visit(ctx->scope());
    }
    
    return 0;
}

antlrcpp::Any visitFunction_declaration(CodeGenVisitor* visitor, ifccParser::Function_declarationContext *ctx) {
    std::string func_name = ctx->VAR()->getText();
    
    // Get parameter types from var_declarations_function
    std::vector<Type> paramTypes;
    std::vector<std::string> paramNames;
    
    auto params = ctx->var_declarations_function();
    if (!params.empty()) {
        for (auto param : params) {
            paramTypes.push_back(INT);  // All parameters are int in our grammar
            paramNames.push_back(param->VAR()->getText());
        }
    }
    
    // Register function signature (without creating entry block)
    visitor->getCFG()->add_function(func_name, INT, paramTypes, paramNames);
    
    return 0;
}

antlrcpp::Any visitFunctionCall(CodeGenVisitor* visitor, ifccParser::Function_callContext *ctx) {
    std::string func_name = ctx->VAR()->getText();
    
    // Evaluate and pass arguments
    std::vector<std::string> argValues;
    auto args = ctx->expr();
    if (!args.empty()) {
        for (auto expr : args) {
            std::string arg = std::any_cast<std::string>(visitor->visit(expr));
            argValues.push_back(arg);
        }
    }
    
    // Create temporary for return value
    std::string resultTmp = visitor->getCFG()->current_bb->create_new_tempvar(INT);
    
    // Generate call instruction
    // params format: {function_label, destination_register, arg1, arg2, ...}
    std::vector<std::string> params;
    params.push_back(func_name);  // Function label
    params.push_back(resultTmp);   // Destination register (where return value goes)
    
    // Add arguments
    for (const auto& arg : argValues) {
        params.push_back(arg);
    }
    
    visitor->getCFG()->current_bb->add_IRInstr(IRInstr::call, INT, params);
    
    return resultTmp;
}
