#pragma once

#include "OptimizationPass.h"
#include <vector>
#include <memory>
#include <string>
#include <set>

namespace optim {

/**
 * @brief Manages and orchestrates running optimization passes
 *
 * This class is responsible for:
 * - Managing a collection of optimization passes
 * - Running passes in insertion order
 * - Coordinating the optimization pipeline
 */
class OptimizationManager {
public:
    OptimizationManager() = default;
    ~OptimizationManager() = default;

    // ==================== Pass Management ====================

    /**
     * @brief Add a pass to the manager
     * @param pass Unique pointer to the optimization pass
     */
    void addPass(PassPtr pass);

    /**
     * @brief Add a pass by name (requires registry)
     * @param passName Name of the pass to add
     * @return true if pass was found and added
     */
    bool addPassByName(const std::string& passName);

    /**
     * @brief Remove a pass by name
     * @param passName Name of the pass to remove
     */
    void removePass(const std::string& passName);

    /**
     * @brief Clear all passes
     */
    void clearPasses();

    // ==================== Configuration ====================

    /**
     * @brief Set optimization level (0-3)
     * @param level Optimization level
     */
    void setOptimizationLevel(int level);

    /**
     * @brief Get current optimization level
     * @return Current level (0-3)
     */
    int getOptimizationLevel() const { return optimizationLevel_; }

    /**
     * @brief Enable a specific pass
     * @param passName Name of the pass
     */
    void enablePass(const std::string& passName);

    /**
     * @brief Disable a specific pass
     * @param passName Name of the pass
     */
    void disablePass(const std::string& passName);

    // ==================== Execution ====================

    /**
     * @brief Run all passes on a CFG
     * @param cfg The control flow graph to optimize
     * @return true if any optimization was applied
     */
    bool runOptimizations(CFG* cfg);

    /**
     * @brief Run only IR-level passes
     * @param cfg The control flow graph to optimize
     * @return true if any optimization was applied
     */
    bool runIRPasses(CFG* cfg);

    /**
     * @brief Run only assembly-level passes
     * @param cfg The control flow graph to optimize
     * @return true if any optimization was applied
     */
    bool runAsmPasses(CFG* cfg);

    /**
     * @brief Get list of registered pass names
     * @return Vector of pass names
     */
    std::vector<std::string> getPassNames() const;

    /**
     * @brief Get list of enabled pass names
     * @return Vector of enabled pass names
     */
    std::vector<std::string> getEnabledPassNames() const;

private:
    // Get passes of a specific kind
    std::vector<OptimizationPass*> getPassesByKind(PassKind kind);

    std::vector<PassPtr> passes_;
    std::set<std::string> disabledPasses_;
    int optimizationLevel_ = 2;
};

} // namespace optim
