#include "ConstantPropagation.h"
#include "../ir/IR.h"
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

    // Track variables whose address has been taken.
    // If a variable's address is taken, it might be modified via a pointer,
    // so we cannot safely propagate its value even if we see a store_stack.
    std::set<std::string> aliased;
    for (auto* instr : instrs) {
        if (auto* addr = dynamic_cast<AddressOfSymbolInstr*>(instr)) {
            aliased.insert(addr->src.name);
        }
    }

    // Map: stack variable name → known compile-time constant value (integers only)
    // A variable enters the map when we see:  ldconst W0, val  +  store_stack var, W0
    // It leaves the map when any other store writes to it, or if its address is taken (aliasing).
    std::map<std::string, int64_t> constants;

    for (size_t i = 0; i < instrs.size(); ++i) {
        auto* instr = instrs[i];

        // store_stack var, reg
        //   → if the previous instruction was ldconst reg, val, record the constant
        //   → otherwise invalidate the variable
        if (auto* st = dynamic_cast<StoreStackInstr*>(instr)) {
            // Cannot track constants for variables that might be modified via pointers
            if (aliased.count(st->dest.name)) {
                constants.erase(st->dest.name);
                continue;
            }

            // Look for a preceding ldconst that wrote to st->src.reg
            // We check i > 0 to be safe; the ldconst should be [i-1]
            bool foundConst = false;
            if (i > 0) {
                if (auto* ldc = dynamic_cast<LdConstInstr*>(instrs[i - 1])) {
                    if (ldc->dest.reg == st->src.reg
                        && (ldc->type == IRType::INT32 || ldc->type == IRType::INT64)) {
                        constants[st->dest.name] = ldc->val.raw_int();
                        foundConst = true;
                    }
                }
            }
            if (!foundConst)
                constants.erase(st->dest.name);
            continue;
        }

        // store_ptr ptr, src
        //   → This could modify ANY variable in the current BB if we don't have proper pointers analysis.
        //   → For now, conservatively invalidate all tracked constants.
        if (dynamic_cast<StorePointerInstr*>(instr)) {
            constants.clear();
            continue;
        }

        // load_stack reg, var
        //   → if var is a known constant, replace the load_stack with ldconst reg, val
        if (auto* ld = dynamic_cast<LoadStackInstr*>(instr)) {
            // Only propagate for integer loads
            if (ld->type != IRType::INT32 && ld->type != IRType::INT64) continue;
            auto it = constants.find(ld->src.name);
            if (it != constants.end()) {
                auto* newInstr = new LdConstInstr(bb, ld->dest.reg, ld->type, it->second);
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
