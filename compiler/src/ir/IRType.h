#pragma once

#include <string>
#include <cstdint>
#include <iostream>

enum class IRType {
    VOID,
    INT8,     ///< (C char)
    INT32,    ///< 32-bit signed integer  (C int)
    INT64,    ///< 64-bit signed integer  (C long)
    FLOAT32,  ///< 32-bit float           (C float)
    FLOAT64,  ///< 64-bit float           (C double)
};

inline std::string irtype_name(IRType t) {
    switch (t) {
        case IRType::INT8:
            return "i8";
        case IRType::INT32:
            return "i32";
        case IRType::INT64:
            return "i64";
        case IRType::FLOAT32:
            return "f32";
        case IRType::FLOAT64:
            return "f64";
    }
    return "?";
}

inline int irtype_size(IRType t) {
    switch (t) {
        case IRType::VOID:    throw std::runtime_error("Void has no size");
        case IRType::INT8:
            return 1;
        case IRType::INT32:
            return 4;
        case IRType::INT64:
            return 8;
        case IRType::FLOAT32:
            return 4;
        case IRType::FLOAT64:
            return 8;
    }
    return 4;
}

inline IRType irtype_from_string(const std::string& str) {
    if (str == "void") return IRType::VOID;
    if (str == "char") return IRType::INT8;

    if (str == "int") return IRType::INT32;
    if (str == "double") return IRType::FLOAT64;
    if (str == "long") return IRType::INT64;
    if (str == "float") return IRType::FLOAT32;
    std::cerr << "Unknown type '" << str << "'" << std::endl;
    exit(1);
}
