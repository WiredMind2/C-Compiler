#include "ConstantPropagation.h"
#include "../IR.h"
#include <iostream>
#include <algorithm>

namespace optim {

bool ConstantPropagationPass::optimize(CFG* cfg) {
    if (!cfg) return false;
    
    bool modified = false;
    bool progress = true;
    
    // Run multiple iterations until no more progress
    while (progress) {
        progress = false;
        
        for (auto* bb : cfg->getBBs()) {
            if (optimizeBasicBlock(bb)) {
                progress = true;
            }
        }
        
        if (progress) modified = true;
    }
    
    return modified;
}

bool ConstantPropagationPass::optimizeBasicBlock(BasicBlock* bb) {
    bool modified = false;
    auto& instrs = bb->instrs;
    
    // Map: variable name -> constant value
    std::map<std::string, std::string> constants;
    
    // Track which variables have been "returned" (copied to !eax)
    // Once returned, we can eliminate earlier stores
    std::set<std::string> returnedVars;
    
    for (size_t i = 0; i < instrs.size(); ++i) {
        auto* instr = instrs[i];
        
        // Pattern 1: ldconst var, value -> record the constant
        if (instr->op == IRInstr::Operation::ldconst) {
            if (instr->params.size() >= 2) {
                std::string var = instr->params[0];
                std::string value = instr->params[1];
                
                // Check if this ldconst's destination is already used in copy to !eax
                // If so, we can skip storing it entirely
                if (returnedVars.count(var) > 0) {
                    // The value is already returned, we don't need this store
                    // But keep it for now - it will be cleaned up by dead store elimination
                }
                
                constants[var] = value;
            }
            continue;
        }
        
        // Pattern 2: copy dest, src -> propagate if src is known constant
        if (instr->op == IRInstr::Operation::copy) {
            if (instr->params.size() >= 2) {
                std::string dest = instr->params[0];
                std::string src = instr->params[1];
                
                // Check if source is a known constant
                auto it = constants.find(src);
                if (it != constants.end()) {
                    // Source is known constant! Change to ldconst
                    instr->op = IRInstr::Operation::ldconst;
                    instr->params.clear();
                    instr->params.push_back(dest);
                    instr->params.push_back(it->second);
                    modified = true;
                }
                
                // IMPORTANT: When we assign to a variable (dest), we need to 
                // clear any constant tracking for that variable, because the
                // variable's value has changed!
                // Only keep constant if we're copying from another known constant
                if (src[0] != '!' && it != constants.end()) {
                    // src was propagated from a constant - keep it for dest
                    constants[dest] = it->second;
                } else if (src[0] != '!' && src != dest) {
                    // Copy from non-constant variable OR expression result
                    // Clear any constant we knew about dest
                    constants.erase(dest);
                } else if (dest[0] != '!' && dest != "!eax") {
                    // Direct assignment to variable - clear its constant
                    constants.erase(dest);
                }
            }
            continue;
        }
        
        // Pattern 3: Arithmetic operations (add, sub, mul, div) produce new values
        // When we compute a new value, the result is NOT a constant
        if (instr->op == IRInstr::Operation::add ||
            instr->op == IRInstr::Operation::sub ||
            instr->op == IRInstr::Operation::mul ||
            instr->op == IRInstr::Operation::div) {
            if (instr->params.size() >= 1) {
                std::string dest = instr->params[0];
                if (dest[0] != '!') {
                    constants.erase(dest);
                }
            }
            continue;
        }
        
        // Pattern 3: Check for return (copy !eax, var) - we'll handle this
        // by looking for rmem operations that read variables
        
        // Pattern 4: rmem (read memory) - marks variable as used
        if (instr->op == IRInstr::Operation::rmem) {
            if (instr->params.size() >= 2) {
                std::string dest = instr->params[0];
                std::string src = instr->params[1];
                
                // If we read from a variable, mark it as "used"
                // This helps us know which stores we can't eliminate
                if (src[0] != '!') {
                    // Variable is read - we can't eliminate storing to it
                }
            }
            continue;
        }
        
        // Check for any instruction that uses !eax as destination (like return)
        // These indicate a variable's value is being returned
        for (size_t p = 0; p < instr->params.size(); ++p) {
            if (instr->params[p] == "!eax" && p == 0) {
                // !eax is the destination - check if source is a known var
                if (instr->op == IRInstr::Operation::copy && 
                    instr->params.size() >= 2) {
                    std::string src = instr->params[1];
                    if (src[0] != '!' && src != "!eax") {
                        returnedVars.insert(src);
                    }
                }
            }
        }
    }
    
    // Second pass: eliminate dead stores
    // A store to variable X is dead if X is in returnedVars
    // (because we already returned its value)
    for (size_t i = 0; i < instrs.size(); ) {
        auto* instr = instrs[i];
        
        bool canRemove = false;
        
        // Check if this is a store (copy to a variable, not !eax)
        if (instr->op == IRInstr::Operation::copy) {
            if (instr->params.size() >= 2) {
                std::string dest = instr->params[0];
                std::string src = instr->params[1];
                
                // Check if this is a store (dest is a variable, not special register)
                if (dest[0] != '!' && dest != "!eax") {
                    // Check if this variable was returned
                    if (returnedVars.count(dest) > 0) {
                        // Check if source is !eax (meaning we're storing the return value)
                        // Those stores are also dead after return
                        canRemove = true;
                    }
                }
            }
        }
        
        if (canRemove) {
            // Remove the dead store
            delete instr;
            instrs.erase(instrs.begin() + i);
            modified = true;
            // Don't increment i - check the next instruction
        } else {
            ++i;
        }
    }
    
    // Third pass: final optimization for return statement
    // Look for: copy !eax, var where var is a known constant
    // This needs to run AFTER we track constants from ldconst
    for (size_t i = 0; i < instrs.size(); ++i) {
        auto* instr = instrs[i];
        
        if (instr->op == IRInstr::Operation::copy) {
            if (instr->params.size() >= 2) {
                std::string dest = instr->params[0];
                std::string src = instr->params[1];
                
                // If copying to !eax and source is a known constant
                if (dest == "!eax" && src[0] != '!') {
                    auto it = constants.find(src);
                    if (it != constants.end()) {
                        // Replace with ldconst !eax, value
                        instr->op = IRInstr::Operation::ldconst;
                        instr->params.clear();
                        instr->params.push_back("!eax");
                        instr->params.push_back(it->second);
                        modified = true;
                    }
                }
            }
        }
    }
    
    return modified;
}

} // namespace optim

