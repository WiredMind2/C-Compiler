#pragma once

#include "OptimizationPass.h"
#include <string>

namespace optim {

/**
 * @brief Remove trivial adjacent swap-like copies inside one basic block.
 *
 * Matched pattern:
 *   copy_reg a, b
 *   copy_reg b, a
 *
 * The pair is removed only if the next access to `a` is a write (or `a` is never
 * accessed again), so the temporary value kept in `a` is provably dead.
 */
class CopyRegChainPropagationPass : public OptimizationPass {
public:
    CopyRegChainPropagationPass() = default;

    std::string getName() const override {
        return "copy-reg-chain-prop";
    }

    std::string getDescription() const override {
        return "Propagate copy_reg sources across local move chains";
    }

    PassKind getKind() const override {
        return PassKind::IR_OPT;
    }


    bool optimize(CFG* cfg) override;

private:
    bool optimizeBasicBlock(BasicBlock* bb);
};

} // namespace optim

