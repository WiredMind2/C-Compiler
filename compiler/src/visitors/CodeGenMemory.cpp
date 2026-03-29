#include "CodeGenVisitor.h"
#include "../ir/IR.h"
#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <limits>
#include <functional>

using namespace std;

IRType irtype_from_string(const std::string& str);

antlrcpp::Any visitConstant(CodeGenVisitor* visitor, ifccParser::ConstantContext *ctx) {
    auto* bb = visitor->getCFG()->current_bb;
    string tmp = bb->create_new_tempvar(IRType::INT32);
    int64_t val = std::stoll(ctx->CONST()->getText());
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::INT32, val));
    bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, IRType::INT32));
    return StackParam(tmp, IRType::INT32);
}

antlrcpp::Any visitDouble_constant(CodeGenVisitor* visitor, ifccParser::Double_constantContext *ctx) {
    auto* bb = visitor->getCFG()->current_bb;
    string tmp = bb->create_new_tempvar(IRType::FLOAT64);
    double val = std::stod(ctx->DOUBLE_CONST()->getText());
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::FLOAT64, val));
    bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, IRType::FLOAT64));
    return StackParam(tmp, IRType::FLOAT64);
}

antlrcpp::Any visitChar_constant(CodeGenVisitor* visitor, ifccParser::Char_constantContext *ctx)
{
    string text = ctx->CHAR_CONST()->getText(); // e.g. 'a' or '\n'
    int8_t val;
    if (text[1] == '\\') {
        switch (text[2]) {
            case 'n':  val = '\n'; break;
            case 't':  val = '\t'; break;
            case 'r':  val = '\r'; break;
            case '0':  val = '\0'; break;
            case '\\': val = '\\'; break;
            case '\'': val = '\''; break;
            case 'a':  val = '\a'; break;
            case 'b':  val = '\b'; break;
            case 'f':  val = '\f'; break;
            case 'v':  val = '\v'; break;
            default:   val = static_cast<int8_t>(text[2]); break;
        }
    } else {
        val = static_cast<int8_t>(text[1]);
    }
    BasicBlock *bb = visitor->getCFG()->current_bb;

    bb->add_IRInstr(new LdConstInstr(bb, Reg::W0, IRType::INT8, static_cast<int64_t>(val)));
    string tmp = bb->create_new_tempvar(IRType::INT8);
    bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, IRType::INT8));
    return StackParam(tmp, IRType::INT8);
}

antlrcpp::Any visitVariable(CodeGenVisitor* visitor, ifccParser::VariableContext *ctx)
{
    string name = visitor->getCFG()->current_bb->resolve_var_name(ctx->VAR()->getText());
    auto* bb = visitor->getCFG()->current_bb;
    IRType type = bb->get_var_type(name);
    
    // Arrays decay to pointers when evaluated as rvalues
    if (bb->is_array(name)) {
        string tmp = bb->create_new_tempvar(IRType::POINTER);
        bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, name));
        bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, IRType::POINTER));
        return StackParam(tmp, IRType::POINTER);
    }
    
    return StackParam(name, type);
}

// legacy var_decl_list removed; use visitDeclaration_no_semi/visitDeclaration instead
// New declaration handling
antlrcpp::Any visitDeclaration(CodeGenVisitor* visitor, ifccParser::DeclarationContext *ctx)
{
    if (ctx->declaration_no_semi()) {
        return visitDeclaration_no_semi(visitor, ctx->declaration_no_semi());
    }
    return 0;
}

antlrcpp::Any visitDeclaration_no_semi(CodeGenVisitor* visitor, ifccParser::Declaration_no_semiContext *ctx)
{
    // Handle declarations used in for-loop initializers: type_specifier init_declarator (, init_declarator)*
    IRType baseType = irtype_from_string(ctx->type_specifier()->getText());

    // First register all variables
    for (auto init_decl : ctx->init_declarator()) {
        // init_declarator: declarator ('=' expr | '=' initializer)? ;
        auto* decl = init_decl->declarator();
        string var = decl->VAR()->getText();
        int pointerDepth = 0;
        int arraySize = 0;
        bool isArray = false;
        if (decl->children.size() > 0) {
            for (size_t i = 0; i < decl->children.size(); ++i) {
                auto* ch = decl->children[i];
                auto* t = dynamic_cast<antlr4::tree::TerminalNode*>(ch);
                if (t && t->getText() == "*") pointerDepth++;
                if (t && t->getText() == "[") {
                    // expect next child to be CONST then ']'
                    if (i + 1 < decl->children.size()) {
                        auto* next = dynamic_cast<antlr4::tree::TerminalNode*>(decl->children[i+1]);
                        if (next) {
                            arraySize = std::stoi(next->getText());
                            isArray = true;
                        }
                    }
                }
            }
        }
        bool isPointer = pointerDepth > 0;
        IRType type = isPointer ? IRType::POINTER : baseType;
        BasicBlock* targetBB = visitor->getCFG()->decl_target_bb;
        if (!targetBB) targetBB = visitor->getCFG()->current_bb;
        targetBB->add_var_to_symbol_table(var, type);
        if (isPointer) visitor->getCFG()->set_array_element_type(var, baseType);
        if (isArray) {
            targetBB->set_is_array(var, true);
            visitor->getCFG()->set_array_element_type(var, baseType);
            int elemSize = irtype_size(baseType);
            int totalSize = elemSize * arraySize;
            if (totalSize > elemSize) {
                // allocate the extra bytes needed for the remaining elements
                targetBB->allocate_bytes_on_symbol_table(totalSize - elemSize);
            }
        }
    }

    // Then, for single-init forms, handle initializer if present
    if (ctx->init_declarator().size() == 1) {
        auto* init_decl = ctx->init_declarator()[0];
        auto* bb = visitor->getCFG()->current_bb;
        if (init_decl->expr() != nullptr) {
            auto* decl = init_decl->declarator();
            string var = decl->VAR()->getText();
            int pointerDepth = 0;
            if (decl->children.size() > 0) {
                for (auto* ch : decl->children) {
                    auto* t = dynamic_cast<antlr4::tree::TerminalNode*>(ch);
                    if (t && t->getText() == "*") pointerDepth++;
                }
            }
            IRType variable_type = pointerDepth > 0 ? IRType::POINTER : baseType;

            StackParam src = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(init_decl->expr()));
            auto* bb = visitor->getCFG()->current_bb;
            string mangled_var = bb->resolve_var_name(var);

            if (src.type != variable_type) {
                bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, src.name, src.type));
                bb->generate_conversion_instruction(Reg::W0, src.type, Reg::W1, variable_type);
                bb->add_IRInstr(new StoreStackInstr(bb, mangled_var, Reg::W1, variable_type));
            } else {
                bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, src.name, variable_type));
                bb->add_IRInstr(new StoreStackInstr(bb, mangled_var, Reg::W0, variable_type));
            }
            return StackParam(mangled_var, variable_type);
        }
        else if (init_decl->initializer() != nullptr) {
            // Handle brace initializer for arrays: write each expr into successive slots
            auto* decl = init_decl->declarator();
            string var = decl->VAR()->getText();
            int pointerDepth = 0;
            if (decl->children.size() > 0) {
                for (auto* ch : decl->children) {
                    auto* t = dynamic_cast<antlr4::tree::TerminalNode*>(ch);
                    if (t && t->getText() == "*") pointerDepth++;
                }
            }
            IRType variable_type = pointerDepth > 0 ? IRType::POINTER : baseType;
            string mangled_var = bb->resolve_var_name(var);
            IRType elemType = baseType;
            int elemSize = irtype_size(elemType);
            auto inits = init_decl->initializer()->expr();

            // Create a stable base address temp
            string baseAddr = bb->create_new_tempvar(IRType::POINTER);
            bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, mangled_var));
            bb->add_IRInstr(new StoreStackInstr(bb, baseAddr, Reg::W0, IRType::POINTER));

            for (size_t i = 0; i < inits.size(); ++i) {
                StackParam src = any_cast_to_stack_param_or_throw_on_nullptr(visitor->visit(inits[i]));

                // compute element address: addr = base + i * elemSize
                string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, baseAddr, IRType::POINTER));
                bb->add_IRInstr(new LdConstInstr(bb, Reg::W1, IRType::INT32, (int64_t)(i * elemSize)));
                bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));

                // load value and store through pointer
                bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, src.name, src.type));
                if (src.type != elemType) {
                    bb->generate_conversion_instruction(Reg::W0, src.type, Reg::W0, elemType);
                }
                bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, addrTmp, IRType::POINTER));
                bb->add_IRInstr(new StorePointerInstr(bb, Reg::W1, Reg::W0, elemType));
            }

            return StackParam(mangled_var, variable_type);
        }
    }

    return 0;
}

antlrcpp::Any visitAssignment(CodeGenVisitor* visitor, ifccParser::AssignmentContext *ctx)
{
    auto* bb = visitor->getCFG()->current_bb;

    // Helper to evaluate an lvalue and return its memory address
    std::function<StackParam(ifccParser::LvalueContext*, bool)> getLvalueAddr = [&](ifccParser::LvalueContext* c, bool isTopLevelLvalue) -> StackParam {
        if (c->primitive()) {
            // Case lvalue: primitive ('[' expr ']')?
            ifccParser::PrimitiveContext* prim = c->primitive();
            if (c->expr()) {
                    // Array access on a primitive (e.g., p[5] = ...)
                    // We must obtain the base address (variable or nested primitive) WITHOUT
                    // invoking the general visitor on the primitive, because that would
                    // produce an rvalue load. Walk the primitive to the underlying base.
                    std::function<StackParam(ifccParser::PrimitiveContext*)> getPrimitiveBase = [&](ifccParser::PrimitiveContext* p) -> StackParam {
                        if (!p) return StackParam("", IRType::INT32);
                        ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(p);
                        if (varCtx) {
                            string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                            IRType t = bb->get_var_type(varName);
                            return StackParam(varName, t);
                        }
                        ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(p);
                        if (arrCtx) {
                            return getPrimitiveBase(arrCtx->primitive());
                        }
                        // fallback: evaluate visitor (may produce rvalue)
                        return std::any_cast<StackParam>(visitor->visit(p));
                    };

                    StackParam base = getPrimitiveBase(prim);
                    StackParam index = std::any_cast<StackParam>(visitor->visit(c->expr()));

                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
                    // Multiply index by size (assuming 4 for now)
                    // We do INT32 multiplication, the result in W1.
                    // Determine element size for scaling
                    int elemSize = irtype_size(IRType::INT32);
                    if (bb->cfg->has_array_element_type(base.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(base.name));
                    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
                    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));

                    if (bb->is_array(base.name)) {
                        bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    } else {
                        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    }
                    // 64-bit Add using W0 and W1
                    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
            } else {
                // Must be a variable or an array subscript parsed entirely in primitive
                ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(prim);
                if (varCtx) {
                    string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, varName));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
                }
                
                ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(prim);
                if (arrCtx) {
                    StackParam base = std::any_cast<StackParam>(visitor->visit(arrCtx->primitive()));
                    StackParam index = std::any_cast<StackParam>(visitor->visit(arrCtx->expr()));
                    
                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
                    int elemSize = irtype_size(IRType::INT32);
                    if (bb->cfg->has_array_element_type(base.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(base.name));
                    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
                    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));
                    
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
                }

                // If parenthesis e.g. *(p+1), prim is parenthesis, which falls back to its rvalue evaluation
                StackParam rvalue = std::any_cast<StackParam>(visitor->visit(prim));
                if (isTopLevelLvalue && rvalue.name.find("!tmp") == 0) {
                    throw std::runtime_error("cannot assign to an rvalue");
                }
                // For non-pointer rvalues AND pointer rvalues, take the address of the temporary so that
                // pointers-to-temporaries behave consistently.
                string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, rvalue.name));
                bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                return StackParam(addrTmp, IRType::POINTER);
            }
        } else {
            // Case lvalue: '*' expr_inner
            // We want the value of expr_inner (as an rvalue) to be the address where we store.
            // Evaluate the inner lvalue to obtain its address, then load
            // the pointer value stored there into a new temp so that
            // callers receive a stack slot holding the address to write to.
            StackParam innerAddr = getLvalueAddr(c->lvalue(), false);
            string addrTmp = bb->create_new_tempvar(IRType::POINTER);
            // innerAddr holds the address of the inner lvalue (e.g. &p2).
            // Load that address, then load the pointer value stored at that address
            // so that we obtain the inner pointer (e.g. p2 -> yields &p1).
            bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, innerAddr.name, IRType::POINTER));
            bb->add_IRInstr(new LoadPointerInstr(bb, Reg::W0, Reg::W1, IRType::POINTER));
            bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
            return StackParam(addrTmp, IRType::POINTER);
        }
    };

    // General case: evaluate address and store
    // Evaluate LHS address first to avoid temp-name collisions during RHS evaluation
    StackParam targetAddr = getLvalueAddr(ctx->lvalue(), true);

    // Snapshot the address into a fresh temp to avoid later name collisions
    string addrHolder = bb->create_new_tempvar(IRType::POINTER);
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, targetAddr.name, IRType::POINTER));
    bb->add_IRInstr(new StoreStackInstr(bb, addrHolder, Reg::W1, IRType::POINTER));

    StackParam src = std::any_cast<StackParam>(visitor->visit(ctx->compoundAssignment()));

    // Determine the target variable type to insert any needed conversion
    IRType targetType = IRType::INT32;
    if (ctx->lvalue() && ctx->lvalue()->primitive()) {
        ifccParser::PrimitiveContext* prim = ctx->lvalue()->primitive();
        if (prim) {
            ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(prim);
            if (varCtx) {
                string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                targetType = bb->get_var_type(varName);
            }
            ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(prim);
            if (arrCtx) {
                // To avoid generating IR twice, try to find the variable recursively
                std::function<string(ifccParser::PrimitiveContext*)> getArrayName = [&](ifccParser::PrimitiveContext* p) -> string {
                    ifccParser::VariableContext* v = dynamic_cast<ifccParser::VariableContext*>(p);
                    if (v) return bb->resolve_var_name(v->VAR()->getText());
                    ifccParser::Array_subscriptContext* a = dynamic_cast<ifccParser::Array_subscriptContext*>(p);
                    if (a) return getArrayName(a->primitive());
                    return "";
                };
                string baseName = getArrayName(arrCtx->primitive());
                if (baseName != "" && bb->cfg->has_array_element_type(baseName)) {
                    targetType = bb->cfg->get_array_element_type(baseName);
                }
            }
        }
    }

    // Reload the stable address and perform the store
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, addrHolder, IRType::POINTER));
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, src.name, src.type));
    if (src.type != targetType) {
        bb->generate_conversion_instruction(Reg::W0, src.type, Reg::W0, targetType);
    }
    bb->add_IRInstr(new StorePointerInstr(bb, Reg::W1, Reg::W0, targetType));
    return src;
}


antlrcpp::Any visitDereference(CodeGenVisitor* visitor, ifccParser::DereferenceContext *ctx) {
    StackParam ptr = std::any_cast<StackParam>(visitor->visit(ctx->unary()));
    auto* bb = visitor->getCFG()->current_bb;
    string tmp = bb->create_new_tempvar(IRType::INT32);
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, ptr.name, IRType::POINTER));
    bb->add_IRInstr(new LoadPointerInstr(bb, Reg::W0, Reg::W1, IRType::INT32));
    bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, IRType::INT32));
    return StackParam(tmp, IRType::INT32);
}

antlrcpp::Any visitAddressOf(CodeGenVisitor* visitor, ifccParser::AddressOfContext *ctx) {
    auto* bb = visitor->getCFG()->current_bb;

    // Handle: & ( primitive ) and & ( array_subscript ) via existing primitive path
    ifccParser::PrimitiveExprRefContext* primRefCtx = dynamic_cast<ifccParser::PrimitiveExprRefContext*>(ctx->unary());
    if (primRefCtx) {
        ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(primRefCtx->primitive());
        if (varCtx) {
            string varName = bb->resolve_var_name(varCtx->VAR()->getText());
            string tmp = bb->create_new_tempvar(IRType::POINTER);
            bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, varName));
            bb->add_IRInstr(new StoreStackInstr(bb, tmp, Reg::W0, IRType::POINTER));
            return StackParam(tmp, IRType::POINTER);
        }

        ifccParser::Array_subscriptContext* arrayCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(primRefCtx->primitive());
        if (arrayCtx) {
            StackParam base = std::any_cast<StackParam>(visitor->visit(arrayCtx->primitive()));
            StackParam index = std::any_cast<StackParam>(visitor->visit(arrayCtx->expr()));

            string tmp1 = bb->create_new_tempvar(IRType::POINTER);
            bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
            int elemSize = irtype_size(IRType::INT32);
            if (bb->cfg->has_array_element_type(base.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(base.name));
            bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
            bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));

            bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
            bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
            bb->add_IRInstr(new StoreStackInstr(bb, tmp1, Reg::W0, IRType::POINTER));
            return StackParam(tmp1, IRType::POINTER);
        }
    }

    // Support &(*p) -> p for the common case where unary is a dereference
    ifccParser::DereferenceContext* derefCtx = dynamic_cast<ifccParser::DereferenceContext*>(ctx->unary());
    if (derefCtx) {
        // If the inner unary is a primitive variable, return that variable (holds the pointer)
        ifccParser::PrimitiveExprRefContext* innerPrim = dynamic_cast<ifccParser::PrimitiveExprRefContext*>(derefCtx->unary());
        if (innerPrim) {
            ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(innerPrim->primitive());
            if (varCtx) {
                string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                return StackParam(varName, IRType::POINTER);
            }
        }
    }

    throw std::runtime_error("unsupported form for '&' operator");
}

antlrcpp::Any visitArray_subscript(CodeGenVisitor* visitor, ifccParser::Array_subscriptContext *ctx) {
    auto* bb = visitor->getCFG()->current_bb;
    StackParam base = std::any_cast<StackParam>(visitor->visit(ctx->primitive()));
    StackParam index = std::any_cast<StackParam>(visitor->visit(ctx->expr()));
    
    // address = base + index * size
    string tmp1 = bb->create_new_tempvar(IRType::POINTER);
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
    int elemSize = irtype_size(IRType::INT32);
    if (bb->cfg->has_array_element_type(base.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(base.name));
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));
    
    // Add to base: if base is an array symbol, take its address, otherwise load pointer value
    if (bb->cfg->has_array_element_type(base.name)) {
        bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, base.name, IRType::POINTER));
    } else {
        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
    }
    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
    bb->add_IRInstr(new StoreStackInstr(bb, tmp1, Reg::W0, IRType::POINTER));

    // Load value from address
    string tmp2 = bb->create_new_tempvar(IRType::INT32);
    bb->add_IRInstr(new LoadPointerInstr(bb, Reg::W0, Reg::W0, IRType::INT32));
    bb->add_IRInstr(new StoreStackInstr(bb, tmp2, Reg::W0, IRType::INT32));

    return StackParam(tmp2, IRType::INT32);
}



antlrcpp::Any visitAddAssignment(CodeGenVisitor* visitor, ifccParser::AddAssignmentContext *ctx)
{
    auto* bb = visitor->getCFG()->current_bb;

    // Helper to evaluate an lvalue and return its memory address
    std::function<StackParam(ifccParser::LvalueContext*, bool)> getLvalueAddr = [&](ifccParser::LvalueContext* c, bool isTopLevelLvalue) -> StackParam {
        if (c->primitive()) {
            // Case lvalue: primitive ('[' expr ']')?
            ifccParser::PrimitiveContext* prim = c->primitive();
            if (c->expr()) {
                    // Array access on a primitive (e.g., p[5] = ...)
                    // We must obtain the base address (variable or nested primitive) WITHOUT
                    // invoking the general visitor on the primitive, because that would
                    // produce an rvalue load. Walk the primitive to the underlying base.
                    std::function<StackParam(ifccParser::PrimitiveContext*)> getPrimitiveBase = [&](ifccParser::PrimitiveContext* p) -> StackParam {
                        if (!p) return StackParam("", IRType::INT32);
                        ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(p);
                        if (varCtx) {
                            string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                            IRType t = bb->get_var_type(varName);
                            return StackParam(varName, t);
                        }
                        ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(p);
                        if (arrCtx) {
                            return getPrimitiveBase(arrCtx->primitive());
                        }
                        // fallback: evaluate visitor (may produce rvalue)
                        return std::any_cast<StackParam>(visitor->visit(p));
                    };

                    StackParam base = getPrimitiveBase(prim);
                    StackParam index = std::any_cast<StackParam>(visitor->visit(c->expr()));

                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
                    // Multiply index by size (assuming 4 for now)
                    // We do INT32 multiplication, the result in W1.
                    // Determine element size for scaling
                    int elemSize = irtype_size(IRType::INT32);
                    if (bb->cfg->has_array_element_type(base.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(base.name));
                    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
                    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));

                    if (bb->is_array(base.name)) {
                        bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    } else {
                        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    }
                    // 64-bit Add using W0 and W1
                    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
            } else {
                // Must be a variable or an array subscript parsed entirely in primitive
                ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(prim);
                if (varCtx) {
                    string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, varName));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
                }
                
                ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(prim);
                if (arrCtx) {
                    StackParam base = std::any_cast<StackParam>(visitor->visit(arrCtx->primitive()));
                    StackParam index = std::any_cast<StackParam>(visitor->visit(arrCtx->expr()));
                    
                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
                    int elemSize = irtype_size(IRType::INT32);
                    if (bb->cfg->has_array_element_type(base.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(base.name));
                    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
                    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));
                    
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
                }

                // If parenthesis e.g. *(p+1), prim is parenthesis, which falls back to its rvalue evaluation
                StackParam rvalue = std::any_cast<StackParam>(visitor->visit(prim));
                if (isTopLevelLvalue && rvalue.name.find("!tmp") == 0) {
                    throw std::runtime_error("cannot assign to an rvalue");
                }
                // For non-pointer rvalues AND pointer rvalues, take the address of the temporary so that
                // pointers-to-temporaries behave consistently.
                string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, rvalue.name));
                bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                return StackParam(addrTmp, IRType::POINTER);
            }
        } else {
            // Case lvalue: '*' expr_inner
            // We want the value of expr_inner (as an rvalue) to be the address where we store.
            // Evaluate the inner lvalue to obtain its address, then load
            // the pointer value stored there into a new temp so that
            // callers receive a stack slot holding the address to write to.
            StackParam innerAddr = getLvalueAddr(c->lvalue(), false);
            string addrTmp = bb->create_new_tempvar(IRType::POINTER);
            // innerAddr holds the address of the inner lvalue (e.g. &p2).
            // Load that address, then load the pointer value stored at that address
            // so that we obtain the inner pointer (e.g. p2 -> yields &p1).
            bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, innerAddr.name, IRType::POINTER));
            bb->add_IRInstr(new LoadPointerInstr(bb, Reg::W0, Reg::W1, IRType::POINTER));
            bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
            return StackParam(addrTmp, IRType::POINTER);
        }
    };

    // General case: evaluate address and store
    // Evaluate LHS address first to avoid temp-name collisions during RHS evaluation
    StackParam targetAddr = getLvalueAddr(ctx->lvalue(), true);

    // Snapshot the address into a fresh temp to avoid later name collisions
    string addrHolder = bb->create_new_tempvar(IRType::POINTER);
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, targetAddr.name, IRType::POINTER));
    bb->add_IRInstr(new StoreStackInstr(bb, addrHolder, Reg::W1, IRType::POINTER));

    StackParam src = std::any_cast<StackParam>(visitor->visit(ctx->compoundAssignment()));

    // Determine the target variable type to insert any needed conversion
    IRType targetType = IRType::INT32;
    if (ctx->lvalue() && ctx->lvalue()->primitive()) {
        ifccParser::PrimitiveContext* prim = ctx->lvalue()->primitive();
        if (prim) {
            ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(prim);
            if (varCtx) {
                string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                targetType = bb->get_var_type(varName);
            }
            ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(prim);
            if (arrCtx) {
                // To avoid generating IR twice, try to find the variable recursively
                std::function<string(ifccParser::PrimitiveContext*)> getArrayName = [&](ifccParser::PrimitiveContext* p) -> string {
                    ifccParser::VariableContext* v = dynamic_cast<ifccParser::VariableContext*>(p);
                    if (v) return bb->resolve_var_name(v->VAR()->getText());
                    ifccParser::Array_subscriptContext* a = dynamic_cast<ifccParser::Array_subscriptContext*>(p);
                    if (a) return getArrayName(a->primitive());
                    return "";
                };
                string baseName = getArrayName(arrCtx->primitive());
                if (baseName != "" && bb->cfg->has_array_element_type(baseName)) {
                    targetType = bb->cfg->get_array_element_type(baseName);
                }
            }
        }
    }


    // Reload the stable address and perform the store
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, addrHolder, IRType::POINTER));
    bb->add_IRInstr(new LoadPointerInstr(bb, Reg::W3, Reg::W1, targetType));
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W2, src.name, src.type));
    if (src.type != targetType) {
        bb->generate_conversion_instruction(Reg::W2, src.type, Reg::W2, targetType);
    }
    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W3, Reg::W2, targetType));
    bb->add_IRInstr(new StorePointerInstr(bb, Reg::W1, Reg::W0, targetType));
    
    string resTmp = bb->create_new_tempvar(targetType);
    bb->add_IRInstr(new StoreStackInstr(bb, resTmp, Reg::W0, targetType));
    return StackParam(resTmp, targetType);
    
}

antlrcpp::Any visitSubAssignment(CodeGenVisitor* visitor, ifccParser::SubAssignmentContext *ctx)
{
    auto* bb = visitor->getCFG()->current_bb;

    // Helper to evaluate an lvalue and return its memory address
    std::function<StackParam(ifccParser::LvalueContext*, bool)> getLvalueAddr = [&](ifccParser::LvalueContext* c, bool isTopLevelLvalue) -> StackParam {
        if (c->primitive()) {
            // Case lvalue: primitive ('[' expr ']')?
            ifccParser::PrimitiveContext* prim = c->primitive();
            if (c->expr()) {
                    // Array access on a primitive (e.g., p[5] = ...)
                    // We must obtain the base address (variable or nested primitive) WITHOUT
                    // invoking the general visitor on the primitive, because that would
                    // produce an rvalue load. Walk the primitive to the underlying base.
                    std::function<StackParam(ifccParser::PrimitiveContext*)> getPrimitiveBase = [&](ifccParser::PrimitiveContext* p) -> StackParam {
                        if (!p) return StackParam("", IRType::INT32);
                        ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(p);
                        if (varCtx) {
                            string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                            IRType t = bb->get_var_type(varName);
                            return StackParam(varName, t);
                        }
                        ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(p);
                        if (arrCtx) {
                            return getPrimitiveBase(arrCtx->primitive());
                        }
                        // fallback: evaluate visitor (may produce rvalue)
                        return std::any_cast<StackParam>(visitor->visit(p));
                    };

                    StackParam base = getPrimitiveBase(prim);
                    StackParam index = std::any_cast<StackParam>(visitor->visit(c->expr()));

                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
                    // Multiply index by size (assuming 4 for now)
                    // We do INT32 multiplication, the result in W1.
                    // Determine element size for scaling
                    int elemSize = irtype_size(IRType::INT32);
                    if (bb->cfg->has_array_element_type(base.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(base.name));
                    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
                    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));

                    if (bb->is_array(base.name)) {
                        bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    } else {
                        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    }
                    // 64-bit Add using W0 and W1
                    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
            } else {
                // Must be a variable or an array subscript parsed entirely in primitive
                ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(prim);
                if (varCtx) {
                    string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, varName));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
                }
                
                ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(prim);
                if (arrCtx) {
                    StackParam base = std::any_cast<StackParam>(visitor->visit(arrCtx->primitive()));
                    StackParam index = std::any_cast<StackParam>(visitor->visit(arrCtx->expr()));
                    
                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
                    int elemSize = irtype_size(IRType::INT32);
                    if (bb->cfg->has_array_element_type(base.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(base.name));
                    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
                    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));
                    
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
                }

                // If parenthesis e.g. *(p+1), prim is parenthesis, which falls back to its rvalue evaluation
                StackParam rvalue = std::any_cast<StackParam>(visitor->visit(prim));
                if (isTopLevelLvalue && rvalue.name.find("!tmp") == 0) {
                    throw std::runtime_error("cannot assign to an rvalue");
                }
                // For non-pointer rvalues AND pointer rvalues, take the address of the temporary so that
                // pointers-to-temporaries behave consistently.
                string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, rvalue.name));
                bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                return StackParam(addrTmp, IRType::POINTER);
            }
        } else {
            // Case lvalue: '*' expr_inner
            // We want the value of expr_inner (as an rvalue) to be the address where we store.
            // Evaluate the inner lvalue to obtain its address, then load
            // the pointer value stored there into a new temp so that
            // callers receive a stack slot holding the address to write to.
            StackParam innerAddr = getLvalueAddr(c->lvalue(), false);
            string addrTmp = bb->create_new_tempvar(IRType::POINTER);
            // innerAddr holds the address of the inner lvalue (e.g. &p2).
            // Load that address, then load the pointer value stored at that address
            // so that we obtain the inner pointer (e.g. p2 -> yields &p1).
            bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, innerAddr.name, IRType::POINTER));
            bb->add_IRInstr(new LoadPointerInstr(bb, Reg::W0, Reg::W1, IRType::POINTER));
            bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
            return StackParam(addrTmp, IRType::POINTER);
        }
    };

    // General case: evaluate address and store
    // Evaluate LHS address first to avoid temp-name collisions during RHS evaluation
    StackParam targetAddr = getLvalueAddr(ctx->lvalue(), true);

    // Snapshot the address into a fresh temp to avoid later name collisions
    string addrHolder = bb->create_new_tempvar(IRType::POINTER);
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, targetAddr.name, IRType::POINTER));
    bb->add_IRInstr(new StoreStackInstr(bb, addrHolder, Reg::W1, IRType::POINTER));

    StackParam src = std::any_cast<StackParam>(visitor->visit(ctx->compoundAssignment()));

    // Determine the target variable type to insert any needed conversion
    IRType targetType = IRType::INT32;
    if (ctx->lvalue() && ctx->lvalue()->primitive()) {
        ifccParser::PrimitiveContext* prim = ctx->lvalue()->primitive();
        if (prim) {
            ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(prim);
            if (varCtx) {
                string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                targetType = bb->get_var_type(varName);
            }
            ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(prim);
            if (arrCtx) {
                // To avoid generating IR twice, try to find the variable recursively
                std::function<string(ifccParser::PrimitiveContext*)> getArrayName = [&](ifccParser::PrimitiveContext* p) -> string {
                    ifccParser::VariableContext* v = dynamic_cast<ifccParser::VariableContext*>(p);
                    if (v) return bb->resolve_var_name(v->VAR()->getText());
                    ifccParser::Array_subscriptContext* a = dynamic_cast<ifccParser::Array_subscriptContext*>(p);
                    if (a) return getArrayName(a->primitive());
                    return "";
                };
                string baseName = getArrayName(arrCtx->primitive());
                if (baseName != "" && bb->cfg->has_array_element_type(baseName)) {
                    targetType = bb->cfg->get_array_element_type(baseName);
                }
            }
        }
    }


    // Reload the stable address and perform the store
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, addrHolder, IRType::POINTER));
    bb->add_IRInstr(new LoadPointerInstr(bb, Reg::W3, Reg::W1, targetType));
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W2, src.name, src.type));
    if (src.type != targetType) {
        bb->generate_conversion_instruction(Reg::W2, src.type, Reg::W2, targetType);
    }
    bb->add_IRInstr(new SubInstr(bb, Reg::W0, Reg::W3, Reg::W2, targetType));
    bb->add_IRInstr(new StorePointerInstr(bb, Reg::W1, Reg::W0, targetType));
    
    string resTmp = bb->create_new_tempvar(targetType);
    bb->add_IRInstr(new StoreStackInstr(bb, resTmp, Reg::W0, targetType));
    return StackParam(resTmp, targetType);
    
}

antlrcpp::Any visitPreIncrement(CodeGenVisitor* visitor, ifccParser::PreIncrementContext *ctx)
{
    auto* bb = visitor->getCFG()->current_bb;

    // Helper to evaluate an lvalue and return its memory address
    std::function<StackParam(ifccParser::LvalueContext*, bool)> getLvalueAddr = [&](ifccParser::LvalueContext* c, bool isTopLevelLvalue) -> StackParam {
        if (c->primitive()) {
            // Case lvalue: primitive ('[' expr ']')?
            ifccParser::PrimitiveContext* prim = c->primitive();
            if (c->expr()) {
                    // Array access on a primitive (e.g., p[5] = ...)
                    // We must obtain the base address (variable or nested primitive) WITHOUT
                    // invoking the general visitor on the primitive, because that would
                    // produce an rvalue load. Walk the primitive to the underlying base.
                    std::function<StackParam(ifccParser::PrimitiveContext*)> getPrimitiveBase = [&](ifccParser::PrimitiveContext* p) -> StackParam {
                        if (!p) return StackParam("", IRType::INT32);
                        ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(p);
                        if (varCtx) {
                            string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                            IRType t = bb->get_var_type(varName);
                            return StackParam(varName, t);
                        }
                        ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(p);
                        if (arrCtx) {
                            return getPrimitiveBase(arrCtx->primitive());
                        }
                        // fallback: evaluate visitor (may produce rvalue)
                        return std::any_cast<StackParam>(visitor->visit(p));
                    };

                    StackParam base = getPrimitiveBase(prim);
                    StackParam index = std::any_cast<StackParam>(visitor->visit(c->expr()));

                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
                    // Multiply index by size (assuming 4 for now)
                    // We do INT32 multiplication, the result in W1.
                    // Determine element size for scaling
                    int elemSize = irtype_size(IRType::INT32);
                    if (bb->cfg->has_array_element_type(base.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(base.name));
                    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
                    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));

                    if (bb->is_array(base.name)) {
                        bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    } else {
                        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    }
                    // 64-bit Add using W0 and W1
                    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
            } else {
                // Must be a variable or an array subscript parsed entirely in primitive
                ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(prim);
                if (varCtx) {
                    string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, varName));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
                }
                
                ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(prim);
                if (arrCtx) {
                    StackParam base = std::any_cast<StackParam>(visitor->visit(arrCtx->primitive()));
                    StackParam index = std::any_cast<StackParam>(visitor->visit(arrCtx->expr()));
                    
                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
                    int elemSize = irtype_size(IRType::INT32);
                    if (bb->cfg->has_array_element_type(base.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(base.name));
                    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
                    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));
                    
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
                }

                // If parenthesis e.g. *(p+1), prim is parenthesis, which falls back to its rvalue evaluation
                StackParam rvalue = std::any_cast<StackParam>(visitor->visit(prim));
                if (isTopLevelLvalue && rvalue.name.find("!tmp") == 0) {
                    throw std::runtime_error("cannot assign to an rvalue");
                }
                // For non-pointer rvalues AND pointer rvalues, take the address of the temporary so that
                // pointers-to-temporaries behave consistently.
                string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, rvalue.name));
                bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                return StackParam(addrTmp, IRType::POINTER);
            }
        } else {
            // Case lvalue: '*' expr_inner
            // We want the value of expr_inner (as an rvalue) to be the address where we store.
            // Evaluate the inner lvalue to obtain its address, then load
            // the pointer value stored there into a new temp so that
            // callers receive a stack slot holding the address to write to.
            StackParam innerAddr = getLvalueAddr(c->lvalue(), false);
            string addrTmp = bb->create_new_tempvar(IRType::POINTER);
            // innerAddr holds the address of the inner lvalue (e.g. &p2).
            // Load that address, then load the pointer value stored at that address
            // so that we obtain the inner pointer (e.g. p2 -> yields &p1).
            bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, innerAddr.name, IRType::POINTER));
            bb->add_IRInstr(new LoadPointerInstr(bb, Reg::W0, Reg::W1, IRType::POINTER));
            bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
            return StackParam(addrTmp, IRType::POINTER);
        }
    };

    // General case: evaluate address and store
    // Evaluate LHS address first to avoid temp-name collisions during RHS evaluation
    StackParam targetAddr = getLvalueAddr(ctx->lvalue(), true);

    // Snapshot the address into a fresh temp to avoid later name collisions
    string addrHolder = bb->create_new_tempvar(IRType::POINTER);
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, targetAddr.name, IRType::POINTER));
    bb->add_IRInstr(new StoreStackInstr(bb, addrHolder, Reg::W1, IRType::POINTER));

    // Determine the target variable type to insert any needed conversion
    IRType targetType = IRType::INT32;
    if (ctx->lvalue() && ctx->lvalue()->primitive()) {
        ifccParser::PrimitiveContext* prim = ctx->lvalue()->primitive();
        if (prim) {
            ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(prim);
            if (varCtx) {
                string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                targetType = bb->get_var_type(varName);
            }
            ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(prim);
            if (arrCtx) {
                // To avoid generating IR twice, try to find the variable recursively
                std::function<string(ifccParser::PrimitiveContext*)> getArrayName = [&](ifccParser::PrimitiveContext* p) -> string {
                    ifccParser::VariableContext* v = dynamic_cast<ifccParser::VariableContext*>(p);
                    if (v) return bb->resolve_var_name(v->VAR()->getText());
                    ifccParser::Array_subscriptContext* a = dynamic_cast<ifccParser::Array_subscriptContext*>(p);
                    if (a) return getArrayName(a->primitive());
                    return "";
                };
                string baseName = getArrayName(arrCtx->primitive());
                if (baseName != "" && bb->cfg->has_array_element_type(baseName)) {
                    targetType = bb->cfg->get_array_element_type(baseName);
                }
            }
        }
    }


    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, addrHolder, IRType::POINTER));
    bb->add_IRInstr(new LoadPointerInstr(bb, Reg::W3, Reg::W1, targetType));
    
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, targetType, (int64_t)1));
    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W3, Reg::W2, targetType));
    
    bb->add_IRInstr(new StorePointerInstr(bb, Reg::W1, Reg::W0, targetType));
    
    string resTmp = bb->create_new_tempvar(targetType);
    bb->add_IRInstr(new StoreStackInstr(bb, resTmp, Reg::W0, targetType));
    return StackParam(resTmp, targetType);
    
}

antlrcpp::Any visitPreDecrement(CodeGenVisitor* visitor, ifccParser::PreDecrementContext *ctx)
{
    auto* bb = visitor->getCFG()->current_bb;

    // Helper to evaluate an lvalue and return its memory address
    std::function<StackParam(ifccParser::LvalueContext*, bool)> getLvalueAddr = [&](ifccParser::LvalueContext* c, bool isTopLevelLvalue) -> StackParam {
        if (c->primitive()) {
            // Case lvalue: primitive ('[' expr ']')?
            ifccParser::PrimitiveContext* prim = c->primitive();
            if (c->expr()) {
                    // Array access on a primitive (e.g., p[5] = ...)
                    // We must obtain the base address (variable or nested primitive) WITHOUT
                    // invoking the general visitor on the primitive, because that would
                    // produce an rvalue load. Walk the primitive to the underlying base.
                    std::function<StackParam(ifccParser::PrimitiveContext*)> getPrimitiveBase = [&](ifccParser::PrimitiveContext* p) -> StackParam {
                        if (!p) return StackParam("", IRType::INT32);
                        ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(p);
                        if (varCtx) {
                            string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                            IRType t = bb->get_var_type(varName);
                            return StackParam(varName, t);
                        }
                        ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(p);
                        if (arrCtx) {
                            return getPrimitiveBase(arrCtx->primitive());
                        }
                        // fallback: evaluate visitor (may produce rvalue)
                        return std::any_cast<StackParam>(visitor->visit(p));
                    };

                    StackParam base = getPrimitiveBase(prim);
                    StackParam index = std::any_cast<StackParam>(visitor->visit(c->expr()));

                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
                    // Multiply index by size (assuming 4 for now)
                    // We do INT32 multiplication, the result in W1.
                    // Determine element size for scaling
                    int elemSize = irtype_size(IRType::INT32);
                    if (bb->cfg->has_array_element_type(base.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(base.name));
                    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
                    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));

                    if (bb->is_array(base.name)) {
                        bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    } else {
                        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    }
                    // 64-bit Add using W0 and W1
                    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
            } else {
                // Must be a variable or an array subscript parsed entirely in primitive
                ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(prim);
                if (varCtx) {
                    string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, varName));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
                }
                
                ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(prim);
                if (arrCtx) {
                    StackParam base = std::any_cast<StackParam>(visitor->visit(arrCtx->primitive()));
                    StackParam index = std::any_cast<StackParam>(visitor->visit(arrCtx->expr()));
                    
                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
                    int elemSize = irtype_size(IRType::INT32);
                    if (bb->cfg->has_array_element_type(base.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(base.name));
                    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
                    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));
                    
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
                }

                // If parenthesis e.g. *(p+1), prim is parenthesis, which falls back to its rvalue evaluation
                StackParam rvalue = std::any_cast<StackParam>(visitor->visit(prim));
                if (isTopLevelLvalue && rvalue.name.find("!tmp") == 0) {
                    throw std::runtime_error("cannot assign to an rvalue");
                }
                // For non-pointer rvalues AND pointer rvalues, take the address of the temporary so that
                // pointers-to-temporaries behave consistently.
                string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, rvalue.name));
                bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                return StackParam(addrTmp, IRType::POINTER);
            }
        } else {
            // Case lvalue: '*' expr_inner
            // We want the value of expr_inner (as an rvalue) to be the address where we store.
            // Evaluate the inner lvalue to obtain its address, then load
            // the pointer value stored there into a new temp so that
            // callers receive a stack slot holding the address to write to.
            StackParam innerAddr = getLvalueAddr(c->lvalue(), false);
            string addrTmp = bb->create_new_tempvar(IRType::POINTER);
            // innerAddr holds the address of the inner lvalue (e.g. &p2).
            // Load that address, then load the pointer value stored at that address
            // so that we obtain the inner pointer (e.g. p2 -> yields &p1).
            bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, innerAddr.name, IRType::POINTER));
            bb->add_IRInstr(new LoadPointerInstr(bb, Reg::W0, Reg::W1, IRType::POINTER));
            bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
            return StackParam(addrTmp, IRType::POINTER);
        }
    };

    // General case: evaluate address and store
    // Evaluate LHS address first to avoid temp-name collisions during RHS evaluation
    StackParam targetAddr = getLvalueAddr(ctx->lvalue(), true);

    // Snapshot the address into a fresh temp to avoid later name collisions
    string addrHolder = bb->create_new_tempvar(IRType::POINTER);
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, targetAddr.name, IRType::POINTER));
    bb->add_IRInstr(new StoreStackInstr(bb, addrHolder, Reg::W1, IRType::POINTER));

    // Determine the target variable type to insert any needed conversion
    IRType targetType = IRType::INT32;
    if (ctx->lvalue() && ctx->lvalue()->primitive()) {
        ifccParser::PrimitiveContext* prim = ctx->lvalue()->primitive();
        if (prim) {
            ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(prim);
            if (varCtx) {
                string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                targetType = bb->get_var_type(varName);
            }
            ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(prim);
            if (arrCtx) {
                // To avoid generating IR twice, try to find the variable recursively
                std::function<string(ifccParser::PrimitiveContext*)> getArrayName = [&](ifccParser::PrimitiveContext* p) -> string {
                    ifccParser::VariableContext* v = dynamic_cast<ifccParser::VariableContext*>(p);
                    if (v) return bb->resolve_var_name(v->VAR()->getText());
                    ifccParser::Array_subscriptContext* a = dynamic_cast<ifccParser::Array_subscriptContext*>(p);
                    if (a) return getArrayName(a->primitive());
                    return "";
                };
                string baseName = getArrayName(arrCtx->primitive());
                if (baseName != "" && bb->cfg->has_array_element_type(baseName)) {
                    targetType = bb->cfg->get_array_element_type(baseName);
                }
            }
        }
    }


    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, addrHolder, IRType::POINTER));
    bb->add_IRInstr(new LoadPointerInstr(bb, Reg::W3, Reg::W1, targetType));
    
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, targetType, (int64_t)1));
    bb->add_IRInstr(new SubInstr(bb, Reg::W0, Reg::W3, Reg::W2, targetType));
    
    bb->add_IRInstr(new StorePointerInstr(bb, Reg::W1, Reg::W0, targetType));
    
    string resTmp = bb->create_new_tempvar(targetType);
    bb->add_IRInstr(new StoreStackInstr(bb, resTmp, Reg::W0, targetType));
    return StackParam(resTmp, targetType);
    
}

antlrcpp::Any visitPostIncrement(CodeGenVisitor* visitor, ifccParser::PostIncrementContext *ctx)
{
    auto* bb = visitor->getCFG()->current_bb;

    // Helper to evaluate an lvalue and return its memory address
    std::function<StackParam(ifccParser::LvalueContext*, bool)> getLvalueAddr = [&](ifccParser::LvalueContext* c, bool isTopLevelLvalue) -> StackParam {
        if (c->primitive()) {
            // Case lvalue: primitive ('[' expr ']')?
            ifccParser::PrimitiveContext* prim = c->primitive();
            if (c->expr()) {
                    // Array access on a primitive (e.g., p[5] = ...)
                    // We must obtain the base address (variable or nested primitive) WITHOUT
                    // invoking the general visitor on the primitive, because that would
                    // produce an rvalue load. Walk the primitive to the underlying base.
                    std::function<StackParam(ifccParser::PrimitiveContext*)> getPrimitiveBase = [&](ifccParser::PrimitiveContext* p) -> StackParam {
                        if (!p) return StackParam("", IRType::INT32);
                        ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(p);
                        if (varCtx) {
                            string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                            IRType t = bb->get_var_type(varName);
                            return StackParam(varName, t);
                        }
                        ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(p);
                        if (arrCtx) {
                            return getPrimitiveBase(arrCtx->primitive());
                        }
                        // fallback: evaluate visitor (may produce rvalue)
                        return std::any_cast<StackParam>(visitor->visit(p));
                    };

                    StackParam base = getPrimitiveBase(prim);
                    StackParam index = std::any_cast<StackParam>(visitor->visit(c->expr()));

                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
                    // Multiply index by size (assuming 4 for now)
                    // We do INT32 multiplication, the result in W1.
                    // Determine element size for scaling
                    int elemSize = irtype_size(IRType::INT32);
                    if (bb->cfg->has_array_element_type(base.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(base.name));
                    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
                    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));

                    if (bb->is_array(base.name)) {
                        bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    } else {
                        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    }
                    // 64-bit Add using W0 and W1
                    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
            } else {
                // Must be a variable or an array subscript parsed entirely in primitive
                ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(prim);
                if (varCtx) {
                    string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, varName));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
                }
                
                ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(prim);
                if (arrCtx) {
                    StackParam base = std::any_cast<StackParam>(visitor->visit(arrCtx->primitive()));
                    StackParam index = std::any_cast<StackParam>(visitor->visit(arrCtx->expr()));
                    
                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
                    int elemSize = irtype_size(IRType::INT32);
                    if (bb->cfg->has_array_element_type(base.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(base.name));
                    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
                    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));
                    
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
                }

                // If parenthesis e.g. *(p+1), prim is parenthesis, which falls back to its rvalue evaluation
                StackParam rvalue = std::any_cast<StackParam>(visitor->visit(prim));
                if (isTopLevelLvalue && rvalue.name.find("!tmp") == 0) {
                    throw std::runtime_error("cannot assign to an rvalue");
                }
                // For non-pointer rvalues AND pointer rvalues, take the address of the temporary so that
                // pointers-to-temporaries behave consistently.
                string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, rvalue.name));
                bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                return StackParam(addrTmp, IRType::POINTER);
            }
        } else {
            // Case lvalue: '*' expr_inner
            // We want the value of expr_inner (as an rvalue) to be the address where we store.
            // Evaluate the inner lvalue to obtain its address, then load
            // the pointer value stored there into a new temp so that
            // callers receive a stack slot holding the address to write to.
            StackParam innerAddr = getLvalueAddr(c->lvalue(), false);
            string addrTmp = bb->create_new_tempvar(IRType::POINTER);
            // innerAddr holds the address of the inner lvalue (e.g. &p2).
            // Load that address, then load the pointer value stored at that address
            // so that we obtain the inner pointer (e.g. p2 -> yields &p1).
            bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, innerAddr.name, IRType::POINTER));
            bb->add_IRInstr(new LoadPointerInstr(bb, Reg::W0, Reg::W1, IRType::POINTER));
            bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
            return StackParam(addrTmp, IRType::POINTER);
        }
    };

    // General case: evaluate address and store
    // Evaluate LHS address first to avoid temp-name collisions during RHS evaluation
    StackParam targetAddr = getLvalueAddr(ctx->lvalue(), true);

    // Snapshot the address into a fresh temp to avoid later name collisions
    string addrHolder = bb->create_new_tempvar(IRType::POINTER);
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, targetAddr.name, IRType::POINTER));
    bb->add_IRInstr(new StoreStackInstr(bb, addrHolder, Reg::W1, IRType::POINTER));

    // Determine the target variable type to insert any needed conversion
    IRType targetType = IRType::INT32;
    if (ctx->lvalue() && ctx->lvalue()->primitive()) {
        ifccParser::PrimitiveContext* prim = ctx->lvalue()->primitive();
        if (prim) {
            ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(prim);
            if (varCtx) {
                string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                targetType = bb->get_var_type(varName);
            }
            ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(prim);
            if (arrCtx) {
                // To avoid generating IR twice, try to find the variable recursively
                std::function<string(ifccParser::PrimitiveContext*)> getArrayName = [&](ifccParser::PrimitiveContext* p) -> string {
                    ifccParser::VariableContext* v = dynamic_cast<ifccParser::VariableContext*>(p);
                    if (v) return bb->resolve_var_name(v->VAR()->getText());
                    ifccParser::Array_subscriptContext* a = dynamic_cast<ifccParser::Array_subscriptContext*>(p);
                    if (a) return getArrayName(a->primitive());
                    return "";
                };
                string baseName = getArrayName(arrCtx->primitive());
                if (baseName != "" && bb->cfg->has_array_element_type(baseName)) {
                    targetType = bb->cfg->get_array_element_type(baseName);
                }
            }
        }
    }


    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, addrHolder, IRType::POINTER));
    bb->add_IRInstr(new LoadPointerInstr(bb, Reg::W3, Reg::W1, targetType));
    
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, targetType, (int64_t)1));
    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W3, Reg::W2, targetType));
    
    bb->add_IRInstr(new StorePointerInstr(bb, Reg::W1, Reg::W0, targetType));
    
    string resTmp = bb->create_new_tempvar(targetType);
    bb->add_IRInstr(new StoreStackInstr(bb, resTmp, Reg::W3, targetType));
    return StackParam(resTmp, targetType);
    
}

antlrcpp::Any visitPostDecrement(CodeGenVisitor* visitor, ifccParser::PostDecrementContext *ctx)
{
    auto* bb = visitor->getCFG()->current_bb;

    // Helper to evaluate an lvalue and return its memory address
    std::function<StackParam(ifccParser::LvalueContext*, bool)> getLvalueAddr = [&](ifccParser::LvalueContext* c, bool isTopLevelLvalue) -> StackParam {
        if (c->primitive()) {
            // Case lvalue: primitive ('[' expr ']')?
            ifccParser::PrimitiveContext* prim = c->primitive();
            if (c->expr()) {
                    // Array access on a primitive (e.g., p[5] = ...)
                    // We must obtain the base address (variable or nested primitive) WITHOUT
                    // invoking the general visitor on the primitive, because that would
                    // produce an rvalue load. Walk the primitive to the underlying base.
                    std::function<StackParam(ifccParser::PrimitiveContext*)> getPrimitiveBase = [&](ifccParser::PrimitiveContext* p) -> StackParam {
                        if (!p) return StackParam("", IRType::INT32);
                        ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(p);
                        if (varCtx) {
                            string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                            IRType t = bb->get_var_type(varName);
                            return StackParam(varName, t);
                        }
                        ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(p);
                        if (arrCtx) {
                            return getPrimitiveBase(arrCtx->primitive());
                        }
                        // fallback: evaluate visitor (may produce rvalue)
                        return std::any_cast<StackParam>(visitor->visit(p));
                    };

                    StackParam base = getPrimitiveBase(prim);
                    StackParam index = std::any_cast<StackParam>(visitor->visit(c->expr()));

                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
                    // Multiply index by size (assuming 4 for now)
                    // We do INT32 multiplication, the result in W1.
                    // Determine element size for scaling
                    int elemSize = irtype_size(IRType::INT32);
                    if (bb->cfg->has_array_element_type(base.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(base.name));
                    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
                    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));

                    if (bb->is_array(base.name)) {
                        bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    } else {
                        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    }
                    // 64-bit Add using W0 and W1
                    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
            } else {
                // Must be a variable or an array subscript parsed entirely in primitive
                ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(prim);
                if (varCtx) {
                    string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, varName));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
                }
                
                ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(prim);
                if (arrCtx) {
                    StackParam base = std::any_cast<StackParam>(visitor->visit(arrCtx->primitive()));
                    StackParam index = std::any_cast<StackParam>(visitor->visit(arrCtx->expr()));
                    
                    string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
                    int elemSize = irtype_size(IRType::INT32);
                    if (bb->cfg->has_array_element_type(base.name)) elemSize = irtype_size(bb->cfg->get_array_element_type(base.name));
                    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)elemSize));
                    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));
                    
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
                }

                // If parenthesis e.g. *(p+1), prim is parenthesis, which falls back to its rvalue evaluation
                StackParam rvalue = std::any_cast<StackParam>(visitor->visit(prim));
                if (isTopLevelLvalue && rvalue.name.find("!tmp") == 0) {
                    throw std::runtime_error("cannot assign to an rvalue");
                }
                // For non-pointer rvalues AND pointer rvalues, take the address of the temporary so that
                // pointers-to-temporaries behave consistently.
                string addrTmp = bb->create_new_tempvar(IRType::POINTER);
                bb->add_IRInstr(new AddressOfSymbolInstr(bb, Reg::W0, rvalue.name));
                bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                return StackParam(addrTmp, IRType::POINTER);
            }
        } else {
            // Case lvalue: '*' expr_inner
            // We want the value of expr_inner (as an rvalue) to be the address where we store.
            // Evaluate the inner lvalue to obtain its address, then load
            // the pointer value stored there into a new temp so that
            // callers receive a stack slot holding the address to write to.
            StackParam innerAddr = getLvalueAddr(c->lvalue(), false);
            string addrTmp = bb->create_new_tempvar(IRType::POINTER);
            // innerAddr holds the address of the inner lvalue (e.g. &p2).
            // Load that address, then load the pointer value stored at that address
            // so that we obtain the inner pointer (e.g. p2 -> yields &p1).
            bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, innerAddr.name, IRType::POINTER));
            bb->add_IRInstr(new LoadPointerInstr(bb, Reg::W0, Reg::W1, IRType::POINTER));
            bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
            return StackParam(addrTmp, IRType::POINTER);
        }
    };

    // General case: evaluate address and store
    // Evaluate LHS address first to avoid temp-name collisions during RHS evaluation
    StackParam targetAddr = getLvalueAddr(ctx->lvalue(), true);

    // Snapshot the address into a fresh temp to avoid later name collisions
    string addrHolder = bb->create_new_tempvar(IRType::POINTER);
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, targetAddr.name, IRType::POINTER));
    bb->add_IRInstr(new StoreStackInstr(bb, addrHolder, Reg::W1, IRType::POINTER));

    // Determine the target variable type to insert any needed conversion
    IRType targetType = IRType::INT32;
    if (ctx->lvalue() && ctx->lvalue()->primitive()) {
        ifccParser::PrimitiveContext* prim = ctx->lvalue()->primitive();
        if (prim) {
            ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(prim);
            if (varCtx) {
                string varName = bb->resolve_var_name(varCtx->VAR()->getText());
                targetType = bb->get_var_type(varName);
            }
            ifccParser::Array_subscriptContext* arrCtx = dynamic_cast<ifccParser::Array_subscriptContext*>(prim);
            if (arrCtx) {
                // To avoid generating IR twice, try to find the variable recursively
                std::function<string(ifccParser::PrimitiveContext*)> getArrayName = [&](ifccParser::PrimitiveContext* p) -> string {
                    ifccParser::VariableContext* v = dynamic_cast<ifccParser::VariableContext*>(p);
                    if (v) return bb->resolve_var_name(v->VAR()->getText());
                    ifccParser::Array_subscriptContext* a = dynamic_cast<ifccParser::Array_subscriptContext*>(p);
                    if (a) return getArrayName(a->primitive());
                    return "";
                };
                string baseName = getArrayName(arrCtx->primitive());
                if (baseName != "" && bb->cfg->has_array_element_type(baseName)) {
                    targetType = bb->cfg->get_array_element_type(baseName);
                }
            }
        }
    }


    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, addrHolder, IRType::POINTER));
    bb->add_IRInstr(new LoadPointerInstr(bb, Reg::W3, Reg::W1, targetType));
    
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, targetType, (int64_t)1));
    bb->add_IRInstr(new SubInstr(bb, Reg::W0, Reg::W3, Reg::W2, targetType));
    
    bb->add_IRInstr(new StorePointerInstr(bb, Reg::W1, Reg::W0, targetType));
    
    string resTmp = bb->create_new_tempvar(targetType);
    bb->add_IRInstr(new StoreStackInstr(bb, resTmp, Reg::W3, targetType));
    return StackParam(resTmp, targetType);
    
}
