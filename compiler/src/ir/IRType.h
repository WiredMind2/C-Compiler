#ifndef TYPE_H
#define TYPE_H

#include <string>
#include <cstdint>
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
        case IRType::FLOAT64: return INT;
    }
    return INT;
}


#endif // TYPE_H
