#include "CodeGenVisitor.h"
#include "../ir/IR.h"
#include <iostream>
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

antlrcpp::Any visitVariable(CodeGenVisitor* visitor, ifccParser::VariableContext *ctx) {
    string name = ctx->VAR()->getText();
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

antlrcpp::Any visitVar_decl_list(CodeGenVisitor* visitor, ifccParser::Var_decl_listContext *ctx)
{
    // Handle multiple variable declarations: int x, y, z;
    IRType baseType = irtype_from_string(ctx->type_specifier()->getText());
    for (auto decl : ctx->declarator()) {
        string var = decl->VAR()->getText();
        bool isPointer = decl->getText().find('*') != string::npos;
        IRType type = isPointer ? IRType::POINTER : baseType;
        
        auto* bb = visitor->getCFG()->current_bb;
        if (decl->CONST()) {
            int numElements = std::stoi(decl->CONST()->getText());
            int totalSize = numElements * 4; // Assume 4 bytes for int
            int offset = bb->allocate_bytes_on_symbol_table(totalSize);
            // Register the array base as a symbol at the reserved offset (no extra allocation)
            bb->add_param_to_symbol_table(var, IRType::POINTER, offset);
            bb->set_is_array(var, true);
        } else {
            bb->add_var_to_symbol_table(var, type);
        }
    }
    return 0;
}

antlrcpp::Any visitVar_decl_with_init(CodeGenVisitor* visitor, ifccParser::Var_decl_with_initContext *ctx)
{
    // Handle declaration with initialization: int x = expr;
    string text = ctx->declarator()->getText();
    bool isPointer = text.find('*') != string::npos;
    IRType baseType = irtype_from_string(ctx->type_specifier()->getText());
    IRType type = isPointer ? IRType::POINTER : baseType;
    
    string var = ctx->declarator()->VAR()->getText();
    visitor->getCFG()->current_bb->add_var_to_symbol_table(var, type);

    StackParam src = std::any_cast<StackParam>(visitor->visit(ctx->expr()));
    auto* bb = visitor->getCFG()->current_bb;
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, src.name, type));
    bb->add_IRInstr(new StoreStackInstr(bb, var, Reg::W0, type));
    return StackParam(var, type);
}

antlrcpp::Any visitAssignment(CodeGenVisitor* visitor, ifccParser::AssignmentContext *ctx)
{
    auto* bb = visitor->getCFG()->current_bb;

    // Helper to evaluate an lvalue and return its memory address
    std::function<StackParam(ifccParser::LvalueContext*)> getLvalueAddr = [&](ifccParser::LvalueContext* c) -> StackParam {
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
                            string varName = varCtx->VAR()->getText();
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
                    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)4));
                    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));

                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    // 64-bit Add using W0 and W1
                    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
            } else {
                // Must be a variable or an array subscript parsed entirely in primitive
                ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(prim);
                if (varCtx) {
                    string varName = varCtx->VAR()->getText();
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
                    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)4));
                    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));
                    
                    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
                    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
                    bb->add_IRInstr(new StoreStackInstr(bb, addrTmp, Reg::W0, IRType::POINTER));
                    return StackParam(addrTmp, IRType::POINTER);
                }

                // If parenthesis e.g. *(p+1), prim is parenthesis, which falls back to its rvalue evaluation
                StackParam rvalue = std::any_cast<StackParam>(visitor->visit(prim));
                // The rvalue is stored in a temporary variable.
                // We return the memory address of this temporary variable so that
                // upstream dereferences (the '* lvalue' case) correctly load this rvalue
                // before doing their LoadPointerInstr.
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
            StackParam innerAddr = getLvalueAddr(c->lvalue());
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
    StackParam targetAddr = getLvalueAddr(ctx->lvalue());

    // Snapshot the address into a fresh temp to avoid later name collisions
    string addrHolder = bb->create_new_tempvar(IRType::POINTER);
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, targetAddr.name, IRType::POINTER));
    bb->add_IRInstr(new StoreStackInstr(bb, addrHolder, Reg::W1, IRType::POINTER));

    StackParam src = std::any_cast<StackParam>(visitor->visit(ctx->compoundAssignment()));

    // Reload the stable address and perform the store
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, addrHolder, IRType::POINTER));
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, src.name, src.type));
    bb->add_IRInstr(new StorePointerInstr(bb, Reg::W1, Reg::W0, src.type));
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
    ifccParser::PrimitiveExprRefContext* primRefCtx = dynamic_cast<ifccParser::PrimitiveExprRefContext*>(ctx->unary());
    if (!primRefCtx) return StackParam("", IRType::INT32);
    
    ifccParser::VariableContext* varCtx = dynamic_cast<ifccParser::VariableContext*>(primRefCtx->primitive());
    if (varCtx) {
        string varName = varCtx->VAR()->getText();
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
        bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)4));
        bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));
        
        bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
        bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
        bb->add_IRInstr(new StoreStackInstr(bb, tmp1, Reg::W0, IRType::POINTER));
        return StackParam(tmp1, IRType::POINTER);
    }

    return StackParam("", IRType::INT32);
}

antlrcpp::Any visitArray_subscript(CodeGenVisitor* visitor, ifccParser::Array_subscriptContext *ctx) {
    auto* bb = visitor->getCFG()->current_bb;
    StackParam base = std::any_cast<StackParam>(visitor->visit(ctx->primitive()));
    StackParam index = std::any_cast<StackParam>(visitor->visit(ctx->expr()));
    
    // address = base + index * size
    string tmp1 = bb->create_new_tempvar(IRType::POINTER);
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W1, index.name, IRType::INT32));
    // Multiply index by size (assuming 4 for now)
    bb->add_IRInstr(new LdConstInstr(bb, Reg::W2, IRType::INT32, (int64_t)4));
    bb->add_IRInstr(new MulInstr(bb, Reg::W1, Reg::W1, Reg::W2, IRType::INT32));
    
    // Add to base
    bb->add_IRInstr(new LoadStackInstr(bb, Reg::W0, base.name, IRType::POINTER));
    bb->add_IRInstr(new AddInstr(bb, Reg::W0, Reg::W0, Reg::W1, IRType::POINTER));
    bb->add_IRInstr(new StoreStackInstr(bb, tmp1, Reg::W0, IRType::POINTER));

    // Load value from address
    string tmp2 = bb->create_new_tempvar(IRType::INT32);
    bb->add_IRInstr(new LoadPointerInstr(bb, Reg::W0, Reg::W0, IRType::INT32));
    bb->add_IRInstr(new StoreStackInstr(bb, tmp2, Reg::W0, IRType::INT32));

    return StackParam(tmp2, IRType::INT32);
}
