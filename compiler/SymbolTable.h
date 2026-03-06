#pragma once
#include <map>
#include <string>
#include <iostream>

class SymbolTable {
    struct VarInfo {
        int offset;
        bool initialized;
    };

    struct Scope {
        std::string functionName;
        std::map<std::string, VarInfo> vars;
        int nextOffset = 0;
    };

    std::map<std::string, Scope> scopes; // name -> Scope

public:
    // Creates a new scope (call in DeclarationVisitor).
    void addScope(const std::string &functionName) {
        if (scopes.count(functionName)) {
            std::cerr << "error: scope '" << functionName << "' already exists\n";
            exit(1);
        }
        scopes[functionName] = {functionName, {}, 0};
    }

    // Declare a variable in a named scope; returns its stack offset.
    int declare(const std::string &scopeName, const std::string &name) {
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

    int getOffset(const std::string &scopeName, const std::string &name) {
        auto &vars = getScope(scopeName).vars;
        if (!vars.count(name)) {
            std::cerr << "error: variable '" << name << "' undeclared in '" << scopeName << "'\n";
            exit(1);
        }
        return vars[name].offset;
    }

    bool isDeclared(const std::string &scopeName, const std::string &name) const {
        if (!scopes.count(scopeName)) return false;
        return scopes.at(scopeName).vars.count(name) > 0;
    }

    void markInitialized(const std::string &scopeName, const std::string &name) {
        auto &vars = getScope(scopeName).vars;
        if (!vars.count(name)) {
            std::cerr << "error: variable '" << name << "' undeclared in '" << scopeName << "'\n";
            exit(1);
        }
        vars[name].initialized = true;
    }

    bool isInitialized(const std::string &scopeName, const std::string &name) {
        if (!isDeclared(scopeName, name)) return false;
        if (!scopes.at(scopeName).vars[name].initialized) {
            std::cerr << "warning: variable '" << name << "' used before initialization\n";
            return false;
        }
        return true;
    }

    // Stack size for a given scope, aligned to 16 bytes.
    int stackSize(const std::string &scopeName) const {
        int n = getScope(scopeName).nextOffset;
        return (n + 15) & ~15;
    }

private:
    Scope &getScope(const std::string &name) {
        if (!scopes.count(name)) {
            std::cerr << "error: scope '" << name << "' not found\n";
            exit(1);
        }
        return scopes.at(name);
    }

    const Scope &getScope(const std::string &name) const {
        if (!scopes.count(name)) {
            std::cerr << "error: scope '" << name << "' not found\n";
            exit(1);
        }
        return scopes.at(name);
    }
};
