#include "StoreLoadToRegister.h"
#include "../ir/IR.h"
#include "../ir/IRInstr.h"

namespace optim {

bool StoreLoadToRegisterPass::optimize(CFG* cfg) {
    if (!cfg) return false;

    functionRegisterState_.clear();

    bool modified = false;
    for (auto* bb : cfg->getBBs()) {
        if (optimizeBasicBlock(bb)) {
            modified = true;
        }
    }
    return modified;
}

bool StoreLoadToRegisterPass::optimizeBasicBlock(BasicBlock* bb) {
    if (!bb) return false;

    bool modified = false;
    auto& instrs = bb->instrs;

    size_t i = 0;
    while (i < instrs.size()) {
        if (dynamic_cast<StoreStackInstr*>(instrs[i])) {
            if (tryOptimizeStoreLoad(bb, i)) {
                modified = true;
                continue;
            }
        }
        ++i;
    }

    return modified;
}

bool StoreLoadToRegisterPass::tryOptimizeStoreLoad(BasicBlock* bb, size_t storeIdx) {
    auto& instrs = bb->instrs;
    auto* store = dynamic_cast<StoreStackInstr*>(instrs[storeIdx]);
    if (!store) return false;

    const std::string& slot = store->dest.name;
    const Reg srcReg = store->src.reg;
    const IRType storeType = store->type;

    const int loadIdx = findLoadAfterStore(bb, storeIdx + 1, slot);
    if (loadIdx < 0) return false;

    const int lastLoadIdx = findLastLoadAfterStore(bb, storeIdx + 1, slot);
    if (lastLoadIdx < 0) return false;

    auto* load = dynamic_cast<LoadStackInstr*>(instrs[loadIdx]);
    if (!load || load->type != storeType) return false;

    // Required by design: slot must not be used in sibling/child BBs of the same function scope.
    if (isSlotUsedOutsideBBInSameFunction(bb, slot)) {
        return false;
    }

    if (hasCallBetween(bb, storeIdx, static_cast<size_t>(lastLoadIdx))) {
        return false;
    }

    Reg workReg = Reg::W2;
    if (!getAvailableWorkRegister(bb, slot, workReg)) {
        return false;
    }

    // Rewrite all slot accesses in the validated interval.
    // store_stack slot, R -> copy_reg workReg, R
    // load_stack  R, slot -> copy_reg R, workReg
    for (size_t k = storeIdx; k <= static_cast<size_t>(lastLoadIdx); ++k) {
        if (auto* s = dynamic_cast<StoreStackInstr*>(instrs[k]); s && s->dest.name == slot && s->type == storeType) {
            auto* copyStore = new CopyRegInstr(bb, workReg, s->src.reg, storeType);
            delete instrs[k];
            instrs[k] = copyStore;
            continue;
        }

        if (auto* l = dynamic_cast<LoadStackInstr*>(instrs[k]); l && l->src.name == slot && l->type == storeType) {
            auto* copyLoad = new CopyRegInstr(bb, l->dest.reg, workReg, storeType);
            delete instrs[k];
            instrs[k] = copyLoad;
        }
    }

    return true;
}

int StoreLoadToRegisterPass::findLoadAfterStore(BasicBlock* bb, size_t startIdx, const std::string& slot) {
    for (size_t i = startIdx; i < bb->instrs.size(); ++i) {
        auto* s = dynamic_cast<StoreStackInstr*>(bb->instrs[i]);
        if (s && s->dest.name == slot) {
            // Keep scanning: we want to match all loads for this slot in the block.
            continue;
        }
        auto* load = dynamic_cast<LoadStackInstr*>(bb->instrs[i]);
        if (load && load->src.name == slot) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int StoreLoadToRegisterPass::findLastLoadAfterStore(BasicBlock* bb, size_t startIdx, const std::string& slot) {
    int lastLoad = -1;

    for (size_t i = startIdx; i < bb->instrs.size(); ++i) {
        auto* s = dynamic_cast<StoreStackInstr*>(bb->instrs[i]);
        if (s && s->dest.name == slot) {
            // Keep scanning: we want the last load of this slot in the block.
            continue;
        }

        auto* load = dynamic_cast<LoadStackInstr*>(bb->instrs[i]);
        if (load && load->src.name == slot) {
            lastLoad = static_cast<int>(i);
        }
    }

    return lastLoad;
}

bool StoreLoadToRegisterPass::hasCallBetween(BasicBlock* bb, size_t fromIdx, size_t toIdx) {
    for (size_t i = fromIdx + 1; i < toIdx; ++i) {
        if (dynamic_cast<CallInstr*>(bb->instrs[i]) != nullptr) {
            return true;
        }
    }
    return false;
}

bool StoreLoadToRegisterPass::hasCallInBlock(BasicBlock* bb) {
    for (auto* instr : bb->instrs) {
        if (dynamic_cast<CallInstr*>(instr) != nullptr) {
            return true;
        }
    }
    return false;
}

std::string StoreLoadToRegisterPass::getFunctionName(BasicBlock* bb) const {
    if (!bb) return "";
    if (!bb->functionName.empty()) return bb->functionName;
    return bb->cfg ? bb->cfg->getCurrentFunction() : "";
}

bool StoreLoadToRegisterPass::isSlotUsedOutsideBBInSameFunction(BasicBlock* bb, const std::string& slot) {
    if (!bb || !bb->cfg) return true;

    const std::string currentFunc = getFunctionName(bb);

    for (auto* other : bb->cfg->getBBs()) {
        if (!other || other == bb) continue;
        if (getFunctionName(other) != currentFunc) continue;

        if (other->test_var_name == slot) {
            return true;
        }

        for (auto* instr : other->instrs) {
            auto* load = dynamic_cast<LoadStackInstr*>(instr);
            if (load && load->src.name == slot) {
                return true;
            }
            auto* store = dynamic_cast<StoreStackInstr*>(instr);
            if (store && store->dest.name == slot) {
                return true;
            }
        }
    }

    return false;
}

bool StoreLoadToRegisterPass::getAvailableWorkRegister(BasicBlock* bb, const std::string& slot, Reg& outReg) {
    const std::string funcName = getFunctionName(bb);
    auto& state = functionRegisterState_[funcName];

    const auto cached = state.slotToReg.find(slot);
    if (cached != state.slotToReg.end()) {
        outReg = cached->second;
        return true;
    }

    if (state.availableRegs.empty()) {
        return false;
    }

    outReg = *state.availableRegs.begin();
    state.availableRegs.erase(state.availableRegs.begin());
    state.slotToReg[slot] = outReg;
    return true;
}

} // namespace optim

