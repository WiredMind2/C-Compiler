#include "LoadConstantToRegister.h"
#include "../IR.h"
#include <iostream>

namespace optim {

bool LoadConstantToRegisterPass::optimize(CFG* cfg) {
    if (!cfg) return false;
    
    bool modified = false;
    
    // Process each basic block
    for (auto* bb : cfg->getBBs()) {
        if (optimizeBasicBlock(bb)) {
            modified = true;
        }
    }
    
    return modified;
}

bool LoadConstantToRegisterPass::optimizeBasicBlock(BasicBlock* bb) {
    bool modified = false;
    auto& instrs = bb->instrs;
    
    // Iterate through instructions (stop before last since we look at pairs)
    size_t i = 0;
    while (i < instrs.size() - 1) {
        if (tryOptimizePattern(bb, i)) {
            modified = true;
            // After optimization, the copy instruction is removed,
            // so we don't increment i - we need to check if the new
            // instruction at i+1 can also be optimized
        } else {
            i++;
        }
    }
    
    return modified;
}

bool LoadConstantToRegisterPass::tryOptimizePattern(BasicBlock* bb, size_t i) {
    auto* ldconstInstr = bb->instrs[i];
    auto* copyInstr = bb->instrs[i + 1];
    
    // Pattern check 1: First instruction must be ldconst
    if (ldconstInstr->op != IRInstr::Operation::ldconst) {
        return false;
    }
    
    // ldconst params: [destination, constant_value]
    if (ldconstInstr->params.size() < 2) {
        return false;
    }
    
    // Pattern check 2: Second instruction must be copy
    if (copyInstr->op != IRInstr::Operation::copy) {
        return false;
    }
    
    // copy params: [destination, source]
    if (copyInstr->params.size() < 2) {
        return false;
    }
    
    const std::string& ldconstDest = ldconstInstr->params[0];
    const std::string& copyDest = copyInstr->params[0];
    const std::string& copySrc = copyInstr->params[1];
    
    // Pattern check 3: Copy source must be ldconst destination
    // (the temp variable we're trying to eliminate)
    if (copySrc != ldconstDest) {
        return false;
    }
    
    // Pattern check 4: Copy destination must be a register (!eax, !ebx, etc.)
    // We only optimize when the destination is a special register
    if (copyDest.empty() || copyDest[0] != '!') {
        return false;
    }
    
    // OPTIMIZATION: Change ldconst to load directly to the copy destination
    // and remove the now-redundant copy instruction
    
    // Update ldconst destination to be the copy destination
    ldconstInstr->params[0] = copyDest;
    
    // Remove the copy instruction
    delete copyInstr;
    bb->instrs.erase(bb->instrs.begin() + i + 1);
    
#ifdef DEBUG_OPTIMIZATION
    std::cout << "[LoadConstantToRegister] Optimized: ldconst " 
              << ldconstDest << " -> copy to " << copyDest << std::endl;
#endif
    
    return true;
}

} // namespace optim
