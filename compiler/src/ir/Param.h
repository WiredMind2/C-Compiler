#pragma once
#include <string>
#include <variant>
#include <cstdint>
#include <stdexcept>

#include "IRType.h"
#include "Reg.h"

using namespace std;

enum class ParamKind { Const, Stack, Reg };

/** Base Param — carries the IRType shared by all subclasses */
struct Param {
    IRType type;

    explicit Param(IRType t = IRType::INT32) : type(t) {}
    virtual ~Param() = default;
    virtual ParamKind kind() const = 0;
    virtual string to_string() const = 0;
};

/** A compile-time constant. The IRType lives in the Param base. */
struct ConstParam : Param {
    std::variant<int32_t, int64_t, float, double> value;

    // Integer constructors
    ConstParam(IRType t, int64_t v) : Param(t) {
        switch (t) {
            case IRType::INT32:   value = static_cast<int32_t>(v); break;
            case IRType::INT64:
            case IRType::POINTER: value = v;                        break;
            default: throw std::invalid_argument("ConstParam: use float constructor for FLOAT types");
        }
    }

    // Float constructors
    ConstParam(IRType t, double v) : Param(t) {
        switch (t) {
            case IRType::FLOAT32: value = static_cast<float>(v); break;
            case IRType::FLOAT64: value = v;                      break;
            default: throw std::invalid_argument("ConstParam: use int constructor for INT types");
        }
    }

    ParamKind kind() const override { return ParamKind::Const; }

    int32_t as_i32() const { return std::get<int32_t>(value); }
    int64_t as_i64() const { return std::get<int64_t>(value); }
    float   as_f32() const { return std::get<float>(value); }
    double  as_f64() const { return std::get<double>(value); }

    /** Return value as int64_t regardless of integer type (useful for code-gen). */
    int64_t raw_int() const {
        switch (type) {
            case IRType::INT32:   return as_i32();
            case IRType::INT64:
            case IRType::POINTER: return as_i64();
            default: throw std::invalid_argument("raw_int() called on float constant");
        }
    }

    string to_string() const override {
        switch (type) {
            case IRType::INT32:   return std::to_string(as_i32());
            case IRType::INT64:   return std::to_string(as_i64()) + "L";
            case IRType::FLOAT32: return std::to_string(as_f32()) + "f";
            case IRType::FLOAT64: return std::to_string(as_f64());
        }
        return "?";
    }

    bool operator==(const ConstParam& o) const { return type == o.type && value == o.value; }
    bool operator!=(const ConstParam& o) const { return !(*this == o); }
};

/** A variable or temporary allocated on the stack, identified by name. */
struct StackParam : Param {
    string name;

    explicit StackParam(string n, IRType t = IRType::INT32)
        : Param(t), name(std::move(n)) {}

    ParamKind kind() const override { return ParamKind::Stack; }
    string to_string() const override { return name + ":" + irtype_name(type); }
};

/** A machine register; size is determined by the inherited IRType. */
struct RegParam : Param {
    Reg reg;

    explicit RegParam(Reg r, IRType t = IRType::INT32) : Param(t), reg(r) {}

    ParamKind kind() const override { return ParamKind::Reg; }
    string to_string() const override { return reg_name(reg) + ":" + irtype_name(type); }
};
