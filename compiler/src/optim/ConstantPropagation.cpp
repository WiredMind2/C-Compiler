#include "ConstantPropagation.h"
#include "../IR.h"
#include <iostream>

namespace optim {

bool ConstantPropagationPass::optimize(CFG* cfg) {
    if (!cfg) return false;

    bool modified = false;
    bool progress = true;
    while (progress) {
        progress = false;
        for (auto* bb : cfg->getBBs())
            if (optimizeBasicBlock(bb)) { progress = true; }
        if (progress) modified = true;
    }
    return modified;
}

bool ConstantPropagationPass::optimizeBasicBlock(BasicBlock* bb) {
    bool modified = false;
    auto& instrs = bb->instrs;

    // Map: stack variable name → known compile-time constant value
    // A variable enters the map when we see:  ldconst W0, val  +  store_stack var, W0
    // It leaves the map when any other store writes to it.
    std::map<std::string, int> constants;

    for (size_t i = 0; i < instrs.size(); ++i) {
        auto* instr = instrs[i];

        // store_stack var, reg
        //   → if the previous instruction was ldconst reg, val, record the constant
        //   → otherwise invalidate the variable
        if (auto* st = dynamic_cast<StoreStackInstr*>(instr)) {
            // Look for a preceding ldconst that wrote to st->src.reg
            // We check i > 0 to be safe; the ldconst should be [i-1]
            bool foundConst = false;
            if (i > 0) {
                if (auto* ldc = dynamic_cast<LdConstInstr*>(instrs[i - 1])) {
                    if (ldc->dest.reg == st->src.reg) {
                        constants[st->dest.name] = ldc->val.as_i32();
                        foundConst = true;
                    }
                }
            }
            if (!foundConst)
                constants.erase(st->dest.name);
            continue;
        }

        // load_stack reg, var
        //   → if var is a known constant, replace the load_stack with ldconst reg, val
        if (auto* ld = dynamic_cast<LoadStackInstr*>(instr)) {
            auto it = constants.find(ld->src.name);
            if (it != constants.end()) {
                auto* newInstr = new LdConstInstr(bb, ld->dest.reg, TypedConst(ld->type, static_cast<int64_t>(it->second)));
                delete ld;
                instrs[i] = newInstr;
                modified = true;
            }
            continue;
        }

        // All other instructions: do nothing (registers are not tracked here)
    }

    return modified;
}

} // namespace optim
