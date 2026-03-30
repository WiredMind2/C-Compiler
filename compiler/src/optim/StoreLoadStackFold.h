#pragma once

#include <string>
#include <unordered_set>

#include "OptimizationPass.h"

namespace optim {

/**
 * @brief Fold immediate stack store/load pairs into cheaper register operations.
 *
 * Matched pattern (strictly adjacent):
 *   store_stack.<t> slot, Rsrc
 *   load_stack.<t>  Rdst, slot
 *
 * Behavior:
 * - If Rsrc == Rdst:
 *   - keep store and remove only load when the slot is used later,
 *   - otherwise remove both store and load.
 * - If Rsrc != Rdst:
 *   - replace load with copy_reg Rdst, Rsrc when the slot is used later,
 *   - otherwise replace store+load with a single copy_reg Rdst, Rsrc.
 *
 * Slot usage checks include current block and reachable successor blocks in the
 * same function, with cycle-safe traversal of the CFG.
 */
class StoreLoadStackFoldPass : public OptimizationPass {
   public:
    StoreLoadStackFoldPass() = default;

    std::string getName() const override { return "store-load-stack-fold"; }

    std::string getDescription() const override { return "Remove redundant immediate store/load stack pairs"; }

    PassKind getKind() const override { return PassKind::IR_OPT; }

    bool optimize(CFG* cfg) override;

   private:
    bool optimizeBasicBlock(BasicBlock* bb);
    bool tryFoldPattern(BasicBlock* bb, size_t i);
    bool isSlotLoadedAfter(BasicBlock* bb, size_t startIdx, const std::string& slot);
    bool isSlotLoadedAfter(BasicBlock* bb, size_t startIdx, const std::string& slot, std::unordered_set<const BasicBlock*>& visited);
};

}  // namespace optim
