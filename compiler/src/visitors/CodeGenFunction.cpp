//
// Created by dupic on 06/03/2026.
//

#include "CodeGenFunction.h"
#include "../CodeGenVisitor.h"
#include <iostream>

antlrcpp::Any visitFunction_definition(CodeGenVisitor* visitor, ifccParser::Function_definitionContext *ctx) {
    std::string func_name = ctx->VAR()->getText();
    
    // Get parameter types and names from param_list
    std::vector<Type> paramTypes;
    std::vector<std::string> paramNames;
    
    if (ctx->param_list()) {
        auto params = ctx->param_list()->param();
        for (auto param : params) {
            paramTypes.push_back(INT);  // All parameters are int in our grammar
            paramNames.push_back(param->VAR()->getText());
        }
    }
    
    // Create function entry point
    BasicBlock* entryBB = visitor->getCFG()->getOrCreateFunctionEntryBB(func_name, INT, paramTypes, paramNames);
    // Set current bb to the function entry block
    BasicBlock* formerBB = visitor->getCFG()->current_bb; // Save the former current BB to restore later
    visitor->getCFG()->current_bb = entryBB;
    // Add the entryBB on the stack of BBs to manage variable scopes
    visitor->getCFG()->getStackBBs().push_back(entryBB);
    // Visit the function body (scope)
    if (ctx->scope()) {
        visitor->visit(ctx->scope());
    }
    // Restore the former current BB after visiting the function body
    visitor->getCFG()->current_bb = formerBB;
    // Pop the function entry BB from the stack of BBs
    visitor->getCFG()->getStackBBs().pop_back();
    return 0;
}

antlrcpp::Any visitFunction_declaration(CodeGenVisitor* visitor, ifccParser::Function_declarationContext *ctx) {
    std::string func_name = ctx->VAR()->getText();
    
    // Get parameter types from param_list
    std::vector<Type> paramTypes;
    std::vector<std::string> paramNames;
    
    if (ctx->param_list()) {
        auto params = ctx->param_list()->param();
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
    
    // Check if function has been declared or defined before this call
    if (visitor->getCFG()->get_function(func_name) == nullptr) {
        std::cerr << "Error: implicit declaration of function '" << func_name << "'" << std::endl;
        exit(1);
    }
    
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
    std::string resultTmp = visitor->getCFG()->getCurrentBB()->create_new_tempvar(INT);
    
    // Generate call instruction
    // params format: {function_label, destination_register, arg1, arg2, ...}
    std::vector<std::string> params;
    params.push_back(func_name);  // Function label
    params.push_back(resultTmp);   // Destination register (where return value goes)
    
    // Add arguments
    for (const auto& arg : argValues) {
        params.push_back(arg);
    }
    
    visitor->getCFG()->getCurrentBB()->add_IRInstr(IRInstr::call, INT, params);
    
    return resultTmp;
}
