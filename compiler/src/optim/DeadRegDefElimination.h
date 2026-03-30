#pragma once

#include "OptimizationPass.h"
#include <string>

namespace optim {

/**
 * @brief Remove dead register definitions introduced by earlier IR rewrites.
 *
 * The pass removes `ldconst` and `copy_reg` instructions when the destination
 * register is redefined before any later read in the same basic block.
 */
class DeadRegDefEliminationPass : public OptimizationPass {
public:
    DeadRegDefEliminationPass() = default;

    std::string getName() const override {
        return "dead-reg-def-elim";
    }

    std::string getDescription() const override {
        return "Remove dead ldconst/copy_reg definitions overwritten before use";
    }

    PassKind getKind() const override {
        return PassKind::IR_OPT;
    }


    bool optimize(CFG* cfg) override;

private:
    bool optimizeBasicBlock(BasicBlock* bb);
};

} // namespace optim

