#include "UnusedVariableElimination.h"
#include "PassRegistry.h"
#include <set>
#include <algorithm>
#include <climits>

using namespace std;

namespace optim {

// Register the pass
REGISTER_PASS(UnusedVariableEliminationPass)

 

bool UnusedVariableEliminationPass::optimize(CFG* cfg) {
    if (!cfg) return false;

    bool modified = false;

    // For each function, analyze its basic blocks
    for (auto &func : cfg->get_functions()) {
        // Collect set of all declared (non-temp, non-param) locals in this function
        std::set<string> declared;
        for (auto* bb : func.bbs) {
            for (const auto &name : bb->get_symbol_names()) {
                if (name.rfind("!tmp", 0) == 0) continue;
                int idx = bb->get_var_index_or_none(name);
                if (idx != INT_MIN && idx >= 0) continue; // parameter or non-local slot
                declared.insert(name);
            }
        }

        if (declared.empty()) continue;

        // Find used variables: any variable that is read or whose address is taken
        std::set<string> used;
        for (auto* bb : func.bbs) {
            // Only mark test_var_name as used if it's actually read for a conditional jump
            if (bb->exit_true && bb->exit_false && !bb->test_var_name.empty()) {
                used.insert(bb->test_var_name);
            }
            for (auto* instr : bb->instrs) {
                if (auto* ld = dynamic_cast<LoadStackInstr*>(instr)) {
                    used.insert(ld->src.name);
                } else if (auto* addr = dynamic_cast<AddressOfSymbolInstr*>(instr)) {
                    used.insert(addr->src.name);
                } else if (auto* st = dynamic_cast<StoreStackInstr*>(instr)) {
                    // store is a write; don't mark as used (we want to remove pure stores)
                    (void)st;
                }
            }
        }

        // Variables eligible for removal = declared - used
        std::vector<string> toRemove;
        for (const auto &v : declared) if (used.count(v) == 0) toRemove.push_back(v);
        if (toRemove.empty()) continue;

        // Remove stores/loads referring to these variables and erase them from symbol tables
        for (const auto &var : toRemove) {
            for (auto* bb : func.bbs) {
                // Remove any store_stack or load_stack that references var
                auto &instrs = bb->instrs;
                for (auto it = instrs.begin(); it != instrs.end();) {
                    IRInstr* instr = *it;
                    bool erase = false;
                    if (auto* st = dynamic_cast<StoreStackInstr*>(instr)) {
                        if (st->dest.name == var) erase = true;
                    } else if (auto* ld = dynamic_cast<LoadStackInstr*>(instr)) {
                        if (ld->src.name == var) erase = true;
                    } else if (auto* addr = dynamic_cast<AddressOfSymbolInstr*>(instr)) {
                        if (addr->src.name == var) erase = true;
                    }

                    if (erase) {
                        delete instr;
                        it = instrs.erase(it);
                        modified = true;
                    } else {
                        ++it;
                    }
                }

                // Remove symbol table entries if present; remove_symbol returns
                // whether something was actually erased so we can update `modified`.
                if (bb->remove_symbol(var)) modified = true;
            }
        }
    }

    return modified;
}

} // namespace optim
