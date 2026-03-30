#pragma once

#include <memory>
#include <string>

#include "../ir/IR.h"

namespace optim {

/**
 * @brief Kind of optimization pass
 */
enum class PassKind {
    IR_OPT,  /**< Operates on IR (before assembly generation) */
    ASM_OPT, /**< Operates on assembly (after generation) */
    ANALYSIS /**< Analysis pass (gathers info, no modification) */
};

/**
 * @brief Base class for all optimization passes
 *
 * All optimizations should inherit from this class.
 * The framework will automatically invoke the optimize() method
 * on each pass.
 */
class OptimizationPass {
   public:
    virtual ~OptimizationPass() = default;

    // ==================== Pass Identification ====================

    /**
     * @brief Get the name of this pass
     * @return String identifier (e.g., "load-const-to-reg")
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Get a description of what this pass does
     * @return Human-readable description
     */
    virtual std::string getDescription() const = 0;

    /**
     * @brief Get the kind of this pass
     * @return PassKind indicating IR, ASM, or ANALYSIS
     */
    virtual PassKind getKind() const = 0;

    // ==================== Main Execution ====================

    /**
     * @brief Run this optimization on the CFG
     * @param cfg The control flow graph to optimize
     * @return true if any optimization was applied, false otherwise
     */
    virtual bool optimize(CFG* cfg) = 0;

    /**
     * @brief Run analysis (optional, for ANALYSIS passes)
     * @param cfg The control flow graph to analyze
     */
    virtual void analyze(CFG* cfg) {}

    // ==================== Helpers ====================

    /**
     * @brief Check if this pass is enabled
     * @return true if enabled
     */
    bool isEnabled() const { return enabled_; }

    /**
     * @brief Enable or disable this pass
     * @param enabled New enabled state
     */
    void setEnabled(bool enabled) { enabled_ = enabled; }

   protected:
    /**
     * @brief Constructor (protected for derived classes)
     */
    OptimizationPass() : enabled_(true) {}

    bool enabled_;
};

// Smart pointer type for passes
using PassPtr = std::unique_ptr<OptimizationPass>;

}  // namespace optim
