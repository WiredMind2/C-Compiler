#pragma once
#include <map>
#include <string>
#include <iostream>

class SymbolTable {
    struct VarInfo {
        int offset;
        bool initialized;
    };
    typedef struct Function {
        std::string name;
        int paramCount;
        int *paramVartTypes;
        int returnType;
    } Function;

    struct Scope {
        Function *function{};
        std::map<std::string, VarInfo> vars;
        int nextOffset = 0;
    };

    std::map<std::string, Scope> scopes;

public:
    void addScope(const std::string &functionName);

    int declare(const std::string &scopeName, const std::string &name);

    int getOffset(const std::string &scopeName, const std::string &name);

    bool isDeclared(const std::string &scopeName, const std::string &name) const;

    void markInitialized(const std::string &scopeName, const std::string &name);

    bool isInitialized(const std::string &scopeName, const std::string &name);

    int stackSize(const std::string &scopeName) const;

private:
    Scope &getScope(const std::string &name);

    const Scope &getScope(const std::string &name) const;
};
