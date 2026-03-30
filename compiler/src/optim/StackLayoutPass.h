#pragma once

#include "OptimizationPass.h"

namespace optim {

class StackLayoutPass : public OptimizationPass {
public:
    std::string getName() const override { return "stack-layout"; }
    std::string getDescription() const override { return "Packs the stack by separating user variables from temporary compiler variables."; }
    PassKind getKind() const override { return PassKind::IR_OPT; }

    bool optimize(CFG* cfg) override;
};

} // namespace optim
