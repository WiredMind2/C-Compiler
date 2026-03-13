#include "PassRegistry.h"
#include <algorithm>

namespace optim {

PassRegistry& PassRegistry::getInstance() {
    static PassRegistry instance;
    return instance;
}

void PassRegistry::registerPass(const std::string& name, PassBuilder builder) {
    if (builders_.count(name) > 0) {
        // Pass already registered - could warn or ignore
        return;
    }
    builders_[name] = std::move(builder);
}

PassPtr PassRegistry::createPass(const std::string& name) const {
    auto it = builders_.find(name);
    if (it != builders_.end()) {
        return it->second();
    }
    return nullptr;
}

bool PassRegistry::isRegistered(const std::string& name) const {
    return builders_.count(name) > 0;
}

std::vector<std::string> PassRegistry::getAvailablePasses() const {
    std::vector<std::string> names;
    for (const auto& pair : builders_) {
        names.push_back(pair.first);
    }
    return names;
}

void PassRegistry::unregisterPass(const std::string& name) {
    builders_.erase(name);
}

void PassRegistry::clear() {
    builders_.clear();
}

} // namespace optim
