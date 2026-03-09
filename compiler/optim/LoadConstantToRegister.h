#pragma once

#include "OptimizationPass.h"
#include <string>

namespace optim {

/**
 * @brief Optimization: Load constant directly to target register
 * 
 * This pass optimizes the pattern:
 *   ldconst tmp, value
 *   copy target, tmp
 * 
 * To:
 *   ldconst target, value
 * 
 * This eliminates the intermediate temporary variable and
 * loads the constant directly to the target register.
 * 
 * Example: For `return 42;`:
 *   Before: movl $42, -4(%rbp) + movl -4(%rbp), %eax
 *   After:  movl $42, %eax
 */
class LoadConstantToRegisterPass : public OptimizationPass {
public:
    LoadConstantToRegisterPass() = default;
    
    // ==================== Pass Identification ====================
    
    std::string getName() const override {
        return "load-const-to-reg";
    }
    
    std::string getDescription() const override {
        return "Optimize ldconst+copy patterns to load constants directly to target registers";
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
     * @brief Try to optimize ldconst+copy pattern
     * @param bb The basic block
     * @param i Index of the ldconst instruction
     * @return true if optimization was applied
     */
    bool tryOptimizePattern(BasicBlock* bb, size_t i);
};

} // namespace optim
