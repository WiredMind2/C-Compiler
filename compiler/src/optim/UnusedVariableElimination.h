#pragma once

#include "OptimizationPass.h"

namespace optim {

class UnusedVariableEliminationPass : public OptimizationPass {
public:
    UnusedVariableEliminationPass() = default;
    ~UnusedVariableEliminationPass() override = default;

    std::string getName() const override { return "unused-variable-elim"; }
    std::string getDescription() const override { return "Remove stack slots for variables that are never read"; }
    PassKind getKind() const override { return PassKind::IR_OPT; }

    bool optimize(CFG* cfg) override;
};

} // namespace optim
