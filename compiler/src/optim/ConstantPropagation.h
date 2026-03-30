#pragma once

#include <cstddef>
#include <map>
#include <set>
#include <string>

#include "OptimizationPass.h"

namespace optim {

/**
 * @brief Constant Propagation Optimization
 *
 * This pass performs:
 * 1. Copy propagation: When we see "b = a" and we know "a = 42", we know "b = 42"
 * 2. Constant propagation: When we see "return b" and we know "b = 42", use 42 directly
 * 3. Dead store elimination: Remove stores to variables that are never read
 *
 * Example transformation:
 *   int a = 42;
 *   int b = a;
 *   return b;
 *
 * Becomes:
 *   return 42;
 */
class ConstantPropagationPass : public OptimizationPass {
   public:
    ConstantPropagationPass() = default;

    // ==================== Pass Identification ====================

    std::string getName() const override { return "constant-propagation"; }

    std::string getDescription() const override { return "Propagate constant values and eliminate dead stores"; }

    PassKind getKind() const override { return PassKind::IR_OPT; }

    // ==================== Optimization ====================

    bool optimize(CFG* cfg) override;

   private:
    /**
     * @brief Optimize a single basic block
     * @param bb The basic block to optimize
     * @return true if any optimization was applied
     */
    bool optimizeBasicBlock(BasicBlock* bb);
};

}  // namespace optim
