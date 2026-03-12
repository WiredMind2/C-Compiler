#ifndef REG_H
#define REG_H

#include <string>
#include <stdexcept>

/**
 * Architecture-agnostic virtual register set.
 *
 * Registers are grouped by their *physical* identity on most ABIs
 * (x86-64 / ARM64).  Each register has four size variants:
 *
 *   W0_64 / W0_32 / W0_16 / W0_8   — 64 / 32 / 16 / 8-bit view of W0
 *   ...
 *
 * RET, ARG0..ARG5 are *distinct enum values* from the general-purpose
 * registers even though they may map to the same physical register on a
 * given architecture.  Use reg_physical() to test physical identity.
 *
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

// ── Convenience aliases for the most common sizes ───────────────────────────
// 32-bit (C int)
constexpr Reg W0   = Reg::W0_32;
constexpr Reg W1   = Reg::W1_32;
constexpr Reg W2   = Reg::W2_32;
constexpr Reg W3   = Reg::W3_32;
constexpr Reg ARG0 = Reg::ARG0_32;
constexpr Reg ARG1 = Reg::ARG1_32;
constexpr Reg ARG2 = Reg::ARG2_32;
constexpr Reg ARG3 = Reg::ARG3_32;
constexpr Reg ARG4 = Reg::ARG4_32;
constexpr Reg ARG5 = Reg::ARG5_32;
constexpr Reg RET  = Reg::RET_32;

// ── Physical register identity ───────────────────────────────────────────────

/**
 * Physical register index — architecture-independent.
 * Two Reg values with the same PhysReg map to the same hardware register.
 *
 * x86-64 mapping:
 *   PHYS_W0   → rax   PHYS_W1   → rcx   PHYS_W2   → rdx   PHYS_W3   → rbx
 *   PHYS_ARG0 → rdi   PHYS_ARG1 → rsi   PHYS_ARG2 → rdx   PHYS_ARG3 → rcx
 *   PHYS_ARG4 → r8    PHYS_ARG5 → r9    PHYS_RET  → rax
 *
 * ARM64 mapping:
 *   PHYS_W0   → x0/w0   PHYS_W1 → x9/w9   PHYS_W2 → x10/w10  PHYS_W3 → x11/w11
 *   PHYS_ARG0 → x0/w0   PHYS_ARG1 → x1    PHYS_ARG2 → x2
 *   PHYS_ARG3 → x3      PHYS_ARG4 → x4    PHYS_ARG5 → x5
 *   PHYS_RET  → x0/w0
 */
enum class PhysReg {
    PHYS_W0,
    PHYS_W1,
    PHYS_W2,
    PHYS_W3,
    PHYS_ARG0,
    PHYS_ARG1,
    PHYS_ARG2,
    PHYS_ARG3,
    PHYS_ARG4,
    PHYS_ARG5,
    PHYS_RET,
};

/** Return the physical register that backs a given Reg (any size variant). */
inline PhysReg reg_physical(Reg r) {
    switch (r) {
        case Reg::W0_64: case Reg::W0_32: case Reg::W0_16: case Reg::W0_8:
            return PhysReg::PHYS_W0;
        case Reg::W1_64: case Reg::W1_32: case Reg::W1_16: case Reg::W1_8:
            return PhysReg::PHYS_W1;
        case Reg::W2_64: case Reg::W2_32: case Reg::W2_16: case Reg::W2_8:
            return PhysReg::PHYS_W2;
        case Reg::W3_64: case Reg::W3_32: case Reg::W3_16: case Reg::W3_8:
            return PhysReg::PHYS_W3;
        case Reg::ARG0_64: case Reg::ARG0_32: case Reg::ARG0_16: case Reg::ARG0_8:
            return PhysReg::PHYS_ARG0;
        case Reg::ARG1_64: case Reg::ARG1_32: case Reg::ARG1_16: case Reg::ARG1_8:
            return PhysReg::PHYS_ARG1;
        case Reg::ARG2_64: case Reg::ARG2_32: case Reg::ARG2_16: case Reg::ARG2_8:
            return PhysReg::PHYS_ARG2;
        case Reg::ARG3_64: case Reg::ARG3_32: case Reg::ARG3_16: case Reg::ARG3_8:
            return PhysReg::PHYS_ARG3;
        case Reg::ARG4_64: case Reg::ARG4_32: case Reg::ARG4_16: case Reg::ARG4_8:
            return PhysReg::PHYS_ARG4;
        case Reg::ARG5_64: case Reg::ARG5_32: case Reg::ARG5_16: case Reg::ARG5_8:
            return PhysReg::PHYS_ARG5;
        case Reg::RET_64: case Reg::RET_32: case Reg::RET_16: case Reg::RET_8:
            return PhysReg::PHYS_RET;
    }
    throw std::invalid_argument("reg_physical: unknown Reg");
}

/** True when two Reg values refer to the same physical hardware register. */
inline bool reg_same_physical(Reg a, Reg b) {
    return reg_physical(a) == reg_physical(b);
}

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

#endif // REG_H

