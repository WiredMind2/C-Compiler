#pragma once

#include <string>
#include <stdexcept>

/**
 * Architecture-agnostic register identifiers — size-less.
 * The actual bit-width is determined by the IRType of the containing
 * RegParam (INT32 → 32-bit view, INT64 → 64-bit view, etc.).
 *
 * Naming:
 *   W0..W5     = general-purpose working registers
 *   ARG0..ARG5 = argument-passing registers
 *   RET        = return-value register
 */
enum class Reg {
    W0, W1, W2, W3, W4, W5,
    ARG0, ARG1, ARG2, ARG3, ARG4, ARG5,
    FRAME_PTR,
    RET,
};

/** Human-readable name for debug output. */
inline std::string reg_name(Reg r) {
    switch (r) {
        case Reg::W0:   return "W0";
        case Reg::W1:   return "W1";
        case Reg::W2:   return "W2";
        case Reg::W3:   return "W3";
        case Reg::W4:   return "W4";
        case Reg::W5:   return "W5";
        case Reg::ARG0: return "ARG0";
        case Reg::ARG1: return "ARG1";
        case Reg::ARG2: return "ARG2";
        case Reg::ARG3: return "ARG3";
        case Reg::ARG4: return "ARG4";
        case Reg::ARG5: return "ARG5";
        case Reg::RET:  return "RET";
    }
    throw std::invalid_argument("reg_name: unknown Reg");
}
