#pragma once
#include <algorithm>

#include "IRType.h"
#include "Reg.h"

using namespace std;

enum class ParamKind { Const, Stack, Reg };

struct Param {
    virtual ~Param() = default;
    virtual ParamKind kind() const = 0;
    virtual string to_string() const = 0;
};

/** A compile-time constant with its IRType (replaces the old ConstParam) */
struct ConstParam : Param {
    TypedConst val;
    explicit ConstParam(TypedConst v)       : val(v) {}
    ParamKind kind() const override { return ParamKind::Const; }
    string to_string() const override { return val.to_string(); }
};

/** A variable or temporary allocated on the stack, identified by name */
struct StackParam : Param {
    string name;
    explicit StackParam(string n) : name(std::move(n)) {}
    ParamKind kind() const override { return ParamKind::Stack; }
    string to_string() const override { return name; }
};

/** A machine register, identified by the architecture-agnostic Reg enum */
struct RegParam : Param {
    Reg reg;
    explicit RegParam(Reg r) : reg(r) {}
    ParamKind kind() const override { return ParamKind::Reg; }
    string to_string() const override { return reg_name(reg); }
};
