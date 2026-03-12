#include "OptimizationManager.h"
#include "PassRegistry.h"
#include <algorithm>
#include <iostream>

namespace optim {

void OptimizationManager::addPass(PassPtr pass) {
    if (pass) {
        passes_.push_back(std::move(pass));
    }
}

bool OptimizationManager::addPassByName(const std::string& passName) {
    auto pass = PassRegistry::getInstance().createPass(passName);
    if (pass) {
        addPass(std::move(pass));
        return true;
    }
    return false;
}

void OptimizationManager::removePass(const std::string& passName) {
    passes_.erase(
        std::remove_if(passes_.begin(), passes_.end(),
            [&passName](const PassPtr& pass) {
                return pass->getName() == passName;
            }),
        passes_.end()
    );
}

void OptimizationManager::clearPasses() {
    passes_.clear();
}

void OptimizationManager::setOptimizationLevel(int level) {
    optimizationLevel_ = std::max(0, std::min(3, level));
}

void OptimizationManager::enablePass(const std::string& passName) {
    disabledPasses_.erase(passName);
}

void OptimizationManager::disablePass(const std::string& passName) {
    disabledPasses_.insert(passName);
}

bool OptimizationManager::runOptimizations(CFG* cfg) {
    if (!cfg) return false;
    
    bool anyModified = false;
    
    // Sort passes by timing
    sortPasses();
    
    // Run each enabled pass
    for (auto& pass : passes_) {
        // Skip disabled passes
        if (disabledPasses_.count(pass->getName()) > 0) {
            continue;
        }
        
        // Skip if pass is not enabled
        if (!pass->isEnabled()) {
            continue;
        }
        
        // Run analysis passes first if needed
        if (pass->getKind() == PassKind::ANALYSIS) {
            pass->analyze(cfg);
            continue;
        }
        
        // Run IR and ASM optimizations
        if (pass->getKind() == PassKind::IR_OPT || pass->getKind() == PassKind::ASM_OPT) {
            bool modified = pass->optimize(cfg);
            anyModified = anyModified || modified;
        }
    }
    
    return anyModified;
}

bool OptimizationManager::runIRPasses(CFG* cfg) {
    if (!cfg) return false;
    
    bool anyModified = false;
    sortPasses();
    
    for (auto& pass : passes_) {
        if (pass->getKind() == PassKind::IR_OPT && pass->isEnabled()) {
            if (disabledPasses_.count(pass->getName()) == 0) {
                bool modified = pass->optimize(cfg);
                anyModified = anyModified || modified;
            }
        }
    }
    
    return anyModified;
}

bool OptimizationManager::runAsmPasses(CFG* cfg) {
    if (!cfg) return false;
    
    bool anyModified = false;
    sortPasses();
    
    for (auto& pass : passes_) {
        if (pass->getKind() == PassKind::ASM_OPT && pass->isEnabled()) {
            if (disabledPasses_.count(pass->getName()) == 0) {
                bool modified = pass->optimize(cfg);
                anyModified = anyModified || modified;
            }
        }
    }
    
    return anyModified;
}

std::vector<std::string> OptimizationManager::getPassNames() const {
    std::vector<std::string> names;
    for (const auto& pass : passes_) {
        names.push_back(pass->getName());
    }
    return names;
}

std::vector<std::string> OptimizationManager::getEnabledPassNames() const {
    std::vector<std::string> names;
    for (const auto& pass : passes_) {
        if (pass->isEnabled() && disabledPasses_.count(pass->getName()) == 0) {
            names.push_back(pass->getName());
        }
    }
    return names;
}

void OptimizationManager::sortPasses() {
    std::sort(passes_.begin(), passes_.end(),
        [](const PassPtr& a, const PassPtr& b) {
            // Sort by timing: EARLY -> NORMAL -> LATE
            if (a->getTiming() != b->getTiming()) {
                return a->getTiming() < b->getTiming();
            }
            // Same timing: alphabetical by name
            return a->getName() < b->getName();
        }
    );
}

std::vector<OptimizationPass*> OptimizationManager::getPassesByKind(PassKind kind) {
    std::vector<OptimizationPass*> result;
    for (auto& pass : passes_) {
        if (pass->getKind() == kind && pass->isEnabled()) {
            result.push_back(pass.get());
        }
    }
    return result;
}

} // namespace optim
