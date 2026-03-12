#ifndef TYPE_H
#define TYPE_H

#include <string>
#include <cstdint>
#include <variant>
#include <stdexcept>

// ============================================================
//  Source-level type (used by the parser / symbol table)
// ============================================================

typedef enum {
    INT,
    CHAR,
    VOID
} Type;

// ============================================================
//  IRType — the type on which an IR operation works
// ============================================================

enum class IRType {
    INT32,   ///< 32-bit signed integer  (C int)
    INT64,   ///< 64-bit signed integer  (C long)
    FLOAT32, ///< 32-bit float           (C float)
    FLOAT64, ///< 64-bit float           (C double)
};

inline std::string irtype_name(IRType t) {
    switch (t) {
        case IRType::INT32:   return "i32";
        case IRType::INT64:   return "i64";
        case IRType::FLOAT32: return "f32";
        case IRType::FLOAT64: return "f64";
    }
    return "?";
}

/** Convert a parser Type to an IRType */
inline IRType toIRType(Type t) {
    switch (t) {
        case INT:  return IRType::INT32;
        case CHAR: return IRType::INT32;
        default:   return IRType::INT32;
    }
}

/** Convert an IRType back to a parser Type (best effort) */
inline Type fromIRType(IRType t) {
    switch (t) {
        case IRType::INT32:
        case IRType::INT64:   return INT;
        case IRType::FLOAT32:
        case IRType::FLOAT64: return INT; // no float in parser yet
    }
    return INT;
}

// ============================================================
//  TypedConst — a compile-time constant with an explicit IRType
// ============================================================

/**
 * Holds a constant value whose precise type is known at the IR level.
 *
 * Only INT32 is fully implemented for now; the other cases are present
 * for future expansion and throw if their accessor is called with the
 * wrong type.
 *
 * Usage:
 *   TypedConst c(42);                          // INT32 constant
 *   TypedConst c(IRType::INT32, 42);           // explicit
 *   int32_t v = c.as_i32();
 *   std::string s = c.to_string();
 */
struct TypedConst {
    IRType type;

private:
    std::variant<int32_t, int64_t, float, double> value;

public:
    // ── Constructors ──────────────────────────────────────────────────────

    TypedConst(IRType t, int64_t v) : type(t) {
        switch (t) {
            case IRType::INT32:  value = static_cast<int32_t>(v); break;
            case IRType::INT64:  value = v;                        break;
            case IRType::FLOAT32:
            case IRType::FLOAT64:
                throw std::invalid_argument("use float/double constructor for FLOAT types");
        }
    }

    TypedConst(IRType t, double v) : type(t) {
        switch (t) {
            case IRType::FLOAT32: value = static_cast<float>(v); break;
            case IRType::FLOAT64: value = v;                      break;
            case IRType::INT32:
            case IRType::INT64:
                throw std::invalid_argument("use int constructor for INT types");
        }
    }

    // ── Typed accessors ──────────────────────────────────────────────────

    int32_t as_i32() const {
        if (type == IRType::INT32) return std::get<int32_t>(value);
        throw std::bad_variant_access();
    }
    int64_t as_i64() const {
        if (type == IRType::INT64) return std::get<int64_t>(value);
        throw std::bad_variant_access();
    }
    float as_f32() const {
        if (type == IRType::FLOAT32) return std::get<float>(value);
        throw std::bad_variant_access();
    }
    double as_f64() const {
        if (type == IRType::FLOAT64) return std::get<double>(value);
        throw std::bad_variant_access();
    }

    /** Return value as int64_t regardless of type (useful for code-gen). */
    int64_t raw_int() const {
        switch (type) {
            case IRType::INT32:  return as_i32();
            case IRType::INT64:  return as_i64();
            case IRType::FLOAT32:
            case IRType::FLOAT64:
                throw std::invalid_argument("raw_int() called on float constant");
        }
        return 0;
    }

    std::string to_string() const {
        switch (type) {
            case IRType::INT32:   return std::to_string(as_i32());
            case IRType::INT64:   return std::to_string(as_i64()) + "L";
            case IRType::FLOAT32: return std::to_string(as_f32()) + "f";
            case IRType::FLOAT64: return std::to_string(as_f64());
        }
        return "?";
    }

    bool operator==(const TypedConst& o) const {
        return type == o.type && value == o.value;
    }
    bool operator!=(const TypedConst& o) const { return !(*this == o); }
};

#endif // TYPE_H
