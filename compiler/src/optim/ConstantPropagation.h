#pragma once

#include "OptimizationPass.h"
#include <string>
#include <map>
#include <set>

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
    
    std::string getName() const override {
        return "constant-propagation";
    }
    
    std::string getDescription() const override {
        return "Propagate constant values and eliminate dead stores";
    }
    
    PassKind getKind() const override {
        return PassKind::IR_OPT;
    }
    
    PassTiming getTiming() const override {
        return PassTiming::EARLY;
    }
    
    // ==================== Optimization ====================
    
    bool optimize(CFG* cfg) override;
    
private:
    /**
     * @brief Optimize a single basic block
     * @param bb The basic block to optimize
     * @return true if any optimization was applied
     */
    bool optimizeBasicBlock(BasicBlock* bb);
    
    /**
     * @brief Try to propagate constant at given instruction index
     * @param bb The basic block
     * @param i Index of the instruction
     * @param constants Map of known constants
     * @return true if optimization was applied
     */
    bool tryPropagateConstant(BasicBlock* bb, size_t i, 
                             std::map<std::string, std::string>& constants);
};

} // namespace optim
