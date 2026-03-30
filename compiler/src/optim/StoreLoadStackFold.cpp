#include "StoreLoadStackFold.h"

#include "../ir/IR.h"

namespace optim {

bool StoreLoadStackFoldPass::optimize(CFG* cfg) {
    if (!cfg) return false;

    bool modified = false;
    for (auto* bb : cfg->getBBs()) {
        if (optimizeBasicBlock(bb)) {
            modified = true;
        }
    }
    return modified;
}

bool StoreLoadStackFoldPass::optimizeBasicBlock(BasicBlock* bb) {
    bool modified = false;
    auto& instrs = bb->instrs;

    size_t i = 0;
    while (i + 1 < instrs.size()) {
        if (tryFoldPattern(bb, i)) {
            modified = true;
            continue;
        }
        ++i;
    }

    return modified;
}

bool StoreLoadStackFoldPass::tryFoldPattern(BasicBlock* bb, size_t i) {
    auto& instrs = bb->instrs;
    if (i + 1 >= instrs.size()) return false;

    auto* store = dynamic_cast<StoreStackInstr*>(instrs[i]);
    auto* load = dynamic_cast<LoadStackInstr*>(instrs[i + 1]);
    if (!store || !load) return false;

    // Pattern strict: store/stack puis load immédiat du même slot
    // vers le même registre avec le même type.
    if (store->dest.name != load->src.name) return false;
    if (store->type != load->type) return false;

    Reg srcReg = store->src.reg;
    Reg dstReg = load->dest.reg;
    const std::string& slot = store->dest.name;

    // Cas 1: src_reg == dst_reg → la valeur est déjà dans le bon registre
    if (srcReg == dstReg) {
        // Si le slot est utilisé après : juste supprimer le load
        if (isSlotLoadedAfter(bb, i + 2, slot)) {
            delete instrs[i + 1];
            instrs.erase(instrs.begin() + i + 1, instrs.begin() + i + 2);
            return true;
        }
        // Sinon : supprimer store et load
        delete instrs[i + 1];
        delete instrs[i];
        instrs.erase(instrs.begin() + i, instrs.begin() + i + 2);
        return true;
    }

    // Cas 2: src_reg != dst_reg → besoin de transférer
    bool slotUsedAfter = isSlotLoadedAfter(bb, i + 2, slot);

    if (slotUsedAfter) {
        // Si le slot est utilisé après : remplacer le load par un move
        auto* moveInstr = new CopyRegInstr(bb, dstReg, srcReg, store->type);
        delete instrs[i + 1];
        instrs[i + 1] = moveInstr;
        return true;
    } else {
        // Sinon : remplacer store + load par un move
        auto* moveInstr = new CopyRegInstr(bb, dstReg, srcReg, store->type);
        delete instrs[i + 1];
        delete instrs[i];
        instrs[i] = moveInstr;
        instrs.erase(instrs.begin() + i + 1, instrs.begin() + i + 2);
        return true;
    }
}

bool StoreLoadStackFoldPass::isSlotLoadedAfter(BasicBlock* bb, size_t startIdx, const std::string& slot) {
    std::unordered_set<const BasicBlock*> visited;
    return isSlotLoadedAfter(bb, startIdx, slot, visited);
}

bool StoreLoadStackFoldPass::isSlotLoadedAfter(BasicBlock* bb, size_t startIdx,
                                               const std::string& slot,
                                               std::unordered_set<const BasicBlock*>& visited) {
    if (!bb) return false;

    // Stop traversing already visited blocks to avoid infinite recursion on loops.
    if (visited.find(bb) != visited.end()) {
        return false;
    }
    visited.insert(bb);

    // Vérifier les instructions restantes du BB courant
    for (size_t j = startIdx; j < bb->instrs.size(); ++j) {
        auto* load = dynamic_cast<LoadStackInstr*>(bb->instrs[j]);
        if (load && load->src.name == slot) {
            return true;
        }
    }

    // Vérifier le test_var_name (utilisé dans les conditionnelles)
    if (bb->test_var_name == slot) {
        return true;
    }

    // Vérifier récursivement dans les BBs enfants (successeurs)
    // mais seulement s'ils sont dans la même fonction (même scope)
     auto getFunc = [](BasicBlock* b) {
         return b->functionName.empty() ? (b->cfg ? b->cfg->getCurrentFunction() : "") : b->functionName;
     };
     std::string currentFunc = getFunc(bb);

     if (bb->exit_true) {
         std::string childFunc = getFunc(bb->exit_true);
         if (childFunc == currentFunc && isSlotLoadedAfter(bb->exit_true, 0, slot, visited)) {
             return true;
         }
     }
     if (bb->exit_false) {
         std::string childFunc = getFunc(bb->exit_false);
         if (childFunc == currentFunc && isSlotLoadedAfter(bb->exit_false, 0, slot, visited)) {
             return true;
         }
     }

    return false;
}

} // namespace optim

