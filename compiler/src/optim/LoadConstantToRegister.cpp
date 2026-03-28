#include "LoadConstantToRegister.h"
#include "../ir/IR.h"
#include <iostream>

namespace optim {

bool LoadConstantToRegisterPass::optimize(CFG* cfg) {
    if (!cfg) return false;
    bool modified = false;
    for (auto* bb : cfg->getBBs())
        if (optimizeBasicBlock(bb)) modified = true;
    return modified;
}

bool LoadConstantToRegisterPass::optimizeBasicBlock(BasicBlock* bb) {
    bool modified = false;
    auto& instrs = bb->instrs;

    size_t i = 0;
    while (i + 2 < instrs.size()) {
        if (tryOptimizePattern(bb, i))
            modified = true;
        else
            i++;
    }
    return modified;
}

bool LoadConstantToRegisterPass::tryOptimizePattern(BasicBlock* bb, size_t i) {
    auto& instrs = bb->instrs;
    if (i + 2 >= instrs.size()) return false;

    // Pattern:
    //   [i]   ldconst.i32  W0_32, <val>
    //   [i+1] store_stack.i32  <tmp>, W0_32
    //   [i+2] load_stack.i32   W0_32, <tmp>
    //
    // If <tmp> is not used anywhere after i+2, we can drop [i+1] and [i+2]:
    // the constant is already in W0_32 after [i].

    auto* ldc   = dynamic_cast<LdConstInstr*>  (instrs[i]);
    auto* store = dynamic_cast<StoreStackInstr*>(instrs[i + 1]);
    auto* load  = dynamic_cast<LoadStackInstr*> (instrs[i + 2]);

    if (!ldc || !store || !load) return false;

    // The store must save the same register that ldconst wrote
    if (store->src.reg != ldc->dest.reg) return false;
    // The load must reload the same stack slot that was just stored
    if (load->src.name != store->dest.name) return false;
    // The load destination must be the same register as ldconst destination
    if (load->dest.reg != ldc->dest.reg) return false;
    // Types must match — a cross-type load requires a conversion instruction
    if (load->type != ldc->type) return false;

    // Make sure <tmp> is not used anywhere after i+2
    const std::string& tmp = store->dest.name;
    for (size_t j = i + 3; j < instrs.size(); ++j) {
        if (instrs[j]->to_string().find(tmp) != std::string::npos)
            return false;
    }

    // Safe to remove store + load
    delete store;
    delete load;
    instrs.erase(instrs.begin() + i + 1, instrs.begin() + i + 3);

#ifdef DEBUG_OPTIMIZATION
    std::cout << "[LoadConstantToRegister] Folded ldconst+store+load for " << tmp << "\n";
#endif

    return true;
}

} // namespace optim
