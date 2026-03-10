#include "SymbolTable.h"

void SymbolTable::addScope(const std::string &functionName) {
    if (scopes.count(functionName)) {
        std::cerr << "error: scope '" << functionName << "' already exists\n";
        exit(1);
    }
    scopes[functionName] = {nullptr, {}, 0};
}

int SymbolTable::declare(const std::string &scopeName, const std::string &name) {
    auto &scope = getScope(scopeName);
    if (scope.vars.count(name)) {
        std::cerr << "error: variable '" << name << "' already declared in '" << scopeName << "'\n";
        exit(1);
    }
    int offset = scope.nextOffset;
    scope.vars[name] = {offset, false};
    scope.nextOffset += 4;
    return offset;
}

int SymbolTable::getOffset(const std::string &scopeName, const std::string &name) {
    auto &vars = getScope(scopeName).vars;
    if (!vars.count(name)) {
        std::cerr << "error: variable '" << name << "' undeclared in '" << scopeName << "'\n";
        exit(1);
    }
    return vars[name].offset;
}

bool SymbolTable::isDeclared(const std::string &scopeName, const std::string &name) const {
    if (!scopes.count(scopeName)) return false;
    return scopes.at(scopeName).vars.count(name) > 0;
}

void SymbolTable::markInitialized(const std::string &scopeName, const std::string &name) {
    auto &vars = getScope(scopeName).vars;
    if (!vars.count(name)) {
        std::cerr << "error: variable '" << name << "' undeclared in '" << scopeName << "'\n";
        exit(1);
    }
    vars[name].initialized = true;
}

bool SymbolTable::isInitialized(const std::string &scopeName, const std::string &name) {
    if (!isDeclared(scopeName, name)) return false;
    if (!scopes.at(scopeName).vars[name].initialized) {
        std::cerr << "warning: variable '" << name << "' used before initialization\n";
        return false;
    }
    return true;
}

int SymbolTable::stackSize(const std::string &scopeName) const {
    int n = getScope(scopeName).nextOffset;
    return (n + 15) & ~15;
}

SymbolTable::Scope &SymbolTable::getScope(const std::string &name) {
    if (!scopes.count(name)) {
        std::cerr << "error: scope '" << name << "' not found\n";
        exit(1);
    }
    return scopes.at(name);
}

const SymbolTable::Scope &SymbolTable::getScope(const std::string &name) const {
    if (!scopes.count(name)) {
        std::cerr << "error: scope '" << name << "' not found\n";
        exit(1);
    }
    return scopes.at(name);
}

SymbolTable::Function* SymbolTable::getScopeFunction(const std::string &scopeName) {
    return getScope(scopeName).function;
}