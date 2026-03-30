#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "OptimizationPass.h"

namespace optim {

/**
 * @brief Function type for creating pass instances
 */
using PassBuilder = std::function<PassPtr()>;

/**
 * @brief Singleton registry for optimization passes
 *
 * Passes can register themselves using the REGISTER_PASS macro,
 * and the registry can create instances by name.
 */
class PassRegistry {
   public:
    /**
     * @brief Get the singleton instance
     * @return Reference to the registry
     */
    static PassRegistry& getInstance();

    /**
     * @brief Register a pass with a builder function
     * @param name Unique name for the pass
     * @param builder Function that creates a new instance
     */
    void registerPass(const std::string& name, PassBuilder builder);

    /**
     * @brief Create a new instance of a pass by name
     * @param name Name of the pass to create
     * @return Unique pointer to the pass, or nullptr if not found
     */
    PassPtr createPass(const std::string& name) const;

    /**
     * @brief Check if a pass is registered
     * @param name Name of the pass
     * @return true if registered
     */
    bool isRegistered(const std::string& name) const;

    /**
     * @brief Get all available pass names
     * @return Vector of pass names
     */
    std::vector<std::string> getAvailablePasses() const;

    /**
     * @brief Unregister a pass (useful for testing)
     * @param name Name of the pass to remove
     */
    void unregisterPass(const std::string& name);

    /**
     * @brief Clear all registered passes
     */
    void clear();

   private:
    PassRegistry() = default;

    std::map<std::string, PassBuilder> builders_;
};

// Macro for auto-registering passes
// Usage: REGISTER_PASS(MyOptimiztionPass);
#define REGISTER_PASS(PASS_CLASS)                                                                                                            \
    namespace {                                                                                                                              \
    struct PassRegistrar##PASS_CLASS {                                                                                                       \
        PassRegistrar##PASS_CLASS() {                                                                                                        \
            optim::PassRegistry::getInstance().registerPass(#PASS_CLASS, []() -> optim::PassPtr { return std::make_unique<PASS_CLASS>(); }); \
        }                                                                                                                                    \
    };                                                                                                                                       \
    static PassRegistrar##PASS_CLASS registrar##PASS_CLASS;                                                                                  \
    }

}  // namespace optim
