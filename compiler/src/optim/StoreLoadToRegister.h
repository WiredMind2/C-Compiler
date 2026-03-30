#pragma once

#include "OptimizationPass.h"
#include "../ir/Reg.h"
#include <string>
#include <set>
#include <unordered_map>
#include <cstddef>

namespace optim {

/**
 * @brief Rewrite stack slot accesses to register copies when a block-local slot can be cached.
 *
 * The pass detects a store/load chain for a stack slot inside one BasicBlock,
 * then rewrites all matching accesses of that slot in the validated interval:
 *   store_stack.<t> slot, R -> copy_reg.<t> Wk, R
 *   load_stack.<t>  R, slot -> copy_reg.<t> R, Wk
 *
 * Safety conditions:
 * - no call is allowed between the first candidate store and the last matched load,
 * - the slot must not be read from other basic blocks of the same function.
 *
 * Uses W2..W5 as cache registers and tracks register availability per function.
 * Allocation policy reserves W2 for temporary slots (name starts with '!');
 * non-temporary slots are assigned from W3..W5.
 */
class StoreLoadToRegisterPass : public OptimizationPass {
public:
    StoreLoadToRegisterPass() = default;

    std::string getName() const override {
        return "store-load-to-register";
    }

    std::string getDescription() const override {
        return "Replace store/load stack patterns with register copies when possible";
    }

    PassKind getKind() const override {
        return PassKind::IR_OPT;
    }


    bool optimize(CFG* cfg) override;

private:
    struct FunctionRegisterState {
        struct SlotCacheInfo {
            Reg reg;
            const BasicBlock* bb;
            size_t lastLoadIdx;
        };

        std::set<Reg> availableRegs{Reg::W2, Reg::W3, Reg::W4, Reg::W5};
        std::unordered_map<std::string, Reg> slotToReg;
        std::unordered_map<std::string, SlotCacheInfo> slotCacheInfo;
    };

    bool optimizeBasicBlock(BasicBlock* bb);
    bool tryOptimizeStoreLoad(BasicBlock* bb, size_t storeIdx);
    int  findLoadAfterStore(BasicBlock* bb, size_t storeIdx, const std::string& slot);
    int  findLastLoadAfterStore(BasicBlock* bb, size_t storeIdx, const std::string& slot);
    bool hasCallBetween(BasicBlock* bb, size_t fromIdx, size_t toIdx);
    bool hasPointerAliasRiskBetween(BasicBlock* bb, size_t fromIdx, size_t toIdx, const std::string& slot);
    bool hasCallInBlock(BasicBlock* bb);
    bool isRegUsedBetween(BasicBlock* bb, size_t fromIdx, size_t toIdx, const std::string& slot, Reg r);
    bool isSlotUsedOutsideBBInSameFunction(BasicBlock* bb, const std::string& slot);
    void releaseExpiredCaches(BasicBlock* bb, size_t currentIdx);
    bool getAvailableWorkRegister(BasicBlock* bb, const std::string& slot, size_t storeIdx, size_t lastLoadIdx, Reg& outReg);
    std::string getFunctionName(BasicBlock* bb) const;

    std::unordered_map<std::string, FunctionRegisterState> functionRegisterState_;
};

} // namespace optim

