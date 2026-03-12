#pragma once

#include <string>
#include <stdexcept>

/**
 * W0_64 / W0_32 / W0_16 / W0_8 are 64 / 32 / 16 / 8-bit view of W0
 * Naming: "W" = working (general temporaries inside a BB),
 *         "ARG" = argument-passing,
 *         "RET" = return value.
 */
enum class Reg {
    // ── General-purpose working registers ────────────────────────────────
    //    W0  (x86: rax/eax/ax/al    ARM64: x0/w0/???/???)
    W0_64, W0_32, W0_16, W0_8,
    //    W1  (x86: rcx/ecx/cx/cl    ARM64: x9/w9/???/???)
    W1_64, W1_32, W1_16, W1_8,
    //    W2  (x86: rdx/edx/dx/dl    ARM64: x10/w10/???/???)
    W2_64, W2_32, W2_16, W2_8,
    //    W3  (x86: rbx/ebx/bx/bl    ARM64: x11/w11/???/???)
    W3_64, W3_32, W3_16, W3_8,

    // ── Argument-passing registers ────────────────────────────────────────
    //    ARG0 (x86: rdi/edi/di/dil   ARM64: x0/w0/???/???)
    ARG0_64, ARG0_32, ARG0_16, ARG0_8,
    //    ARG1 (x86: rsi/esi/si/sil   ARM64: x1/w1/???/???)
    ARG1_64, ARG1_32, ARG1_16, ARG1_8,
    //    ARG2 (x86: rdx/edx/dx/dl    ARM64: x2/w2/???/???)
    ARG2_64, ARG2_32, ARG2_16, ARG2_8,
    //    ARG3 (x86: rcx/ecx/cx/cl    ARM64: x3/w3/???/???)
    ARG3_64, ARG3_32, ARG3_16, ARG3_8,
    //    ARG4 (x86: r8/r8d/r8w/r8b   ARM64: x4/w4/???/???)
    ARG4_64, ARG4_32, ARG4_16, ARG4_8,
    //    ARG5 (x86: r9/r9d/r9w/r9b   ARM64: x5/w5/???/???)
    ARG5_64, ARG5_32, ARG5_16, ARG5_8,

    // ── Return-value register ─────────────────────────────────────────────
    //    RET  (x86: rax/eax/ax/al    ARM64: x0/w0/???/???)
    RET_64, RET_32, RET_16, RET_8,
};


/** Bit-width of a Reg variant. */
inline int reg_width(Reg r) {
    switch (r) {
        case Reg::W0_64:   case Reg::W1_64:   case Reg::W2_64:   case Reg::W3_64:
        case Reg::ARG0_64: case Reg::ARG1_64: case Reg::ARG2_64: case Reg::ARG3_64:
        case Reg::ARG4_64: case Reg::ARG5_64: case Reg::RET_64:
            return 64;
        case Reg::W0_32:   case Reg::W1_32:   case Reg::W2_32:   case Reg::W3_32:
        case Reg::ARG0_32: case Reg::ARG1_32: case Reg::ARG2_32: case Reg::ARG3_32:
        case Reg::ARG4_32: case Reg::ARG5_32: case Reg::RET_32:
            return 32;
        case Reg::W0_16:   case Reg::W1_16:   case Reg::W2_16:   case Reg::W3_16:
        case Reg::ARG0_16: case Reg::ARG1_16: case Reg::ARG2_16: case Reg::ARG3_16:
        case Reg::ARG4_16: case Reg::ARG5_16: case Reg::RET_16:
            return 16;
        case Reg::W0_8:    case Reg::W1_8:    case Reg::W2_8:    case Reg::W3_8:
        case Reg::ARG0_8:  case Reg::ARG1_8:  case Reg::ARG2_8:  case Reg::ARG3_8:
        case Reg::ARG4_8:  case Reg::ARG5_8:  case Reg::RET_8:
            return 8;
    }
    throw std::invalid_argument("reg_width: unknown Reg");
}

/** Human-readable name for debug output. */
inline std::string reg_name(Reg r) {
    switch (r) {
        case Reg::W0_64:   return "W0_64";   case Reg::W0_32: return "W0_32";
        case Reg::W0_16:   return "W0_16";   case Reg::W0_8:  return "W0_8";
        case Reg::W1_64:   return "W1_64";   case Reg::W1_32: return "W1_32";
        case Reg::W1_16:   return "W1_16";   case Reg::W1_8:  return "W1_8";
        case Reg::W2_64:   return "W2_64";   case Reg::W2_32: return "W2_32";
        case Reg::W2_16:   return "W2_16";   case Reg::W2_8:  return "W2_8";
        case Reg::W3_64:   return "W3_64";   case Reg::W3_32: return "W3_32";
        case Reg::W3_16:   return "W3_16";   case Reg::W3_8:  return "W3_8";
        case Reg::ARG0_64: return "ARG0_64"; case Reg::ARG0_32: return "ARG0_32";
        case Reg::ARG0_16: return "ARG0_16"; case Reg::ARG0_8:  return "ARG0_8";
        case Reg::ARG1_64: return "ARG1_64"; case Reg::ARG1_32: return "ARG1_32";
        case Reg::ARG1_16: return "ARG1_16"; case Reg::ARG1_8:  return "ARG1_8";
        case Reg::ARG2_64: return "ARG2_64"; case Reg::ARG2_32: return "ARG2_32";
        case Reg::ARG2_16: return "ARG2_16"; case Reg::ARG2_8:  return "ARG2_8";
        case Reg::ARG3_64: return "ARG3_64"; case Reg::ARG3_32: return "ARG3_32";
        case Reg::ARG3_16: return "ARG3_16"; case Reg::ARG3_8:  return "ARG3_8";
        case Reg::ARG4_64: return "ARG4_64"; case Reg::ARG4_32: return "ARG4_32";
        case Reg::ARG4_16: return "ARG4_16"; case Reg::ARG4_8:  return "ARG4_8";
        case Reg::ARG5_64: return "ARG5_64"; case Reg::ARG5_32: return "ARG5_32";
        case Reg::ARG5_16: return "ARG5_16"; case Reg::ARG5_8:  return "ARG5_8";
        case Reg::RET_64:  return "RET_64";  case Reg::RET_32: return "RET_32";
        case Reg::RET_16:  return "RET_16";  case Reg::RET_8:  return "RET_8";
    }
    throw std::invalid_argument("reg_name: unknown Reg");
}
