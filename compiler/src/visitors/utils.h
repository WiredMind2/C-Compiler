#ifndef C_COMPILER_UTILS_H
#define C_COMPILER_UTILS_H

#include "../ir/IR.h"
#include <iostream>

#include "antlr4-runtime.h"

inline StackParam any_cast_to_stack_param_or_throw_on_nullptr(antlrcpp::Any stack_param_or_nullptr) {
    if (!stack_param_or_nullptr.has_value()) {
        std::cerr << "Void value is not ignored as it should be";
        exit(1);
    }

    return std::any_cast<StackParam>(stack_param_or_nullptr);

}

#endif  // C_COMPILER_UTILS_H
