#pragma once
#include <utility>
#include <vector>
#include <string>

#include "IRType.h"
#include "Reg.h"
#include "Param.h"

using namespace std;

class BasicBlock;
class AsmGenerator;

class IRInstr {
public:
    virtual ~IRInstr() = default;

    virtual void accept(AsmGenerator &g, ostream &o) = 0;

    [[nodiscard]] virtual string to_string() const = 0;

    void gen_asm(ostream &o);

    IRType type;
    BasicBlock *bb;

protected:
    explicit IRInstr(BasicBlock *bb_, const IRType t = IRType::INT32) : type(t), bb(bb_) {}
};

/** Load constant into register:  dest = value */
struct LdStringInstr : IRInstr {
    RegParam dest;
    int strIndex;
    LdStringInstr(BasicBlock *bb, const Reg d, int idx) : IRInstr(bb, IRType::POINTER), dest(d, IRType::POINTER), strIndex(idx) {}
    void accept(AsmGenerator& g, std::ostream &o) override;
    std::string to_string() const override { return "ldstr " + irtype_name(IRType::POINTER) + " " + std::to_string(strIndex) + " -> " + reg_name(dest.reg); }
};

struct LdConstInstr : IRInstr {
    RegParam  dest;
    ConstParam val;

    LdConstInstr(BasicBlock *bb, const Reg d, const IRType t, int64_t v)
        : IRInstr(bb, t), dest(d, t), val(t, v) {}

    LdConstInstr(BasicBlock *bb, const Reg d, const IRType t, double v)
        : IRInstr(bb, t), dest(d, t), val(t, v) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "ldconst." + irtype_name(type) + " "
               + dest.to_string() + ", " + val.to_string();
    }
};

/** Copy register to register:  dest = src */
struct CopyRegInstr : IRInstr {
    RegParam dest, src;

    CopyRegInstr(BasicBlock *bb, const Reg d, const Reg s, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), src(s, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "copy_reg." + irtype_name(type) + " "
               + dest.to_string() + ", " + src.to_string();
    }
};

/** Store register to stack slot:  stack[dest] = src */
struct StoreStackInstr : IRInstr {
    StackParam dest;
    RegParam   src;

    StoreStackInstr(BasicBlock *bb, const string &d, const Reg s, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), src(s, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "store_stack." + irtype_name(type) + " "
               + dest.to_string() + ", " + src.to_string();
    }
};

/** Load stack slot into register:  dest = stack[src] */
struct LoadStackInstr : IRInstr {
    RegParam   dest;
    StackParam src;

    LoadStackInstr(BasicBlock *bb, const Reg d, const string &s, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), src(s, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "load_stack." + irtype_name(type) + " "
               + dest.to_string() + ", " + src.to_string();
    }
};

/** Get address of a stack variable: dest = &stack[src] */
struct AddressOfSymbolInstr : IRInstr {
    RegParam   dest;
    StackParam src;

    AddressOfSymbolInstr(BasicBlock *bb, const Reg d, const string &s, const IRType pointerType = IRType::POINTER)
        : IRInstr(bb, pointerType), dest(d, pointerType), src(s, pointerType) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "address_of." + irtype_name(type) + " "
               + dest.to_string() + ", " + src.to_string();
    }
};

/** Read from pointer: dest = *ptr */
struct LoadPointerInstr : IRInstr {
    RegParam dest, ptr;

    LoadPointerInstr(BasicBlock *bb, const Reg d, const Reg p, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), ptr(p, IRType::POINTER) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "load_ptr." + irtype_name(type) + " "
               + dest.to_string() + ", " + ptr.to_string();
    }
};

/** Write to pointer: *ptr = src */
struct StorePointerInstr : IRInstr {
    RegParam ptr, src;

    StorePointerInstr(BasicBlock *bb, const Reg p, const Reg s, const IRType t = IRType::INT32)
        : IRInstr(bb, t), ptr(p, IRType::POINTER), src(s, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "store_ptr." + irtype_name(type) + " "
               + ptr.to_string() + ", " + src.to_string();
    }
};

/** dest = lhs + rhs  (all registers) */
struct AddInstr : IRInstr {
    RegParam dest, lhs, rhs;

    AddInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "add." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = lhs - rhs */
struct SubInstr : IRInstr {
    RegParam dest, lhs, rhs;

    SubInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "sub." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = lhs * rhs */
struct MulInstr : IRInstr {
    RegParam dest, lhs, rhs;

    MulInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "mul." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = lhs / rhs */
struct DivInstr : IRInstr {
    RegParam dest, lhs, rhs;

    DivInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "div." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = lhs % rhs */
struct ModInstr : IRInstr {
    RegParam dest, lhs, rhs;

    ModInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "mod." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = ~src */
struct BitNotInstr : IRInstr {
    RegParam dest, src;

    BitNotInstr(BasicBlock *bb, const Reg d, const Reg s, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), src(s, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "bit_not." + irtype_name(type) + " "
               + dest.to_string() + ", " + src.to_string();
    }
};

/** dest = lhs & rhs */
struct BitAndInstr : IRInstr {
    RegParam dest, lhs, rhs;

    BitAndInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "bit_and." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = lhs | rhs */
struct BitOrInstr : IRInstr {
    RegParam dest, lhs, rhs;

    BitOrInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "bit_or." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = lhs ^ rhs */
struct BitXorInstr : IRInstr {
    RegParam dest, lhs, rhs;

    BitXorInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "bit_xor." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = lhs << rhs */
struct ShlInstr : IRInstr {
    RegParam dest, lhs, rhs;

    ShlInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "shl." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = lhs >> rhs */
struct ShrInstr : IRInstr {
    RegParam dest, lhs, rhs;

    ShrInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "shr." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = (lhs == rhs) */
struct CmpEqInstr : IRInstr {
    RegParam dest, lhs, rhs;

    CmpEqInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, IRType::INT32), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "cmp_eq." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = (lhs < rhs) */
struct CmpLtInstr : IRInstr {
    RegParam dest, lhs, rhs;

    CmpLtInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, IRType::INT32), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "cmp_lt." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = (lhs <= rhs) */
struct CmpLeInstr : IRInstr {
    RegParam dest, lhs, rhs;

    CmpLeInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, IRType::INT32), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "cmp_le." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = (lhs > rhs) */
struct CmpGtInstr : IRInstr {
    RegParam dest, lhs, rhs;
    CmpGtInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, IRType::INT32), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "cmp_gt." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = (lhs >= rhs) */
struct CmpGeInstr : IRInstr {
    RegParam dest, lhs, rhs;
    CmpGeInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, IRType::INT32), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "cmp_ge." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/* dest = lhs && rhs */
struct LogicalAndInstr : IRInstr {
    RegParam dest, lhs, rhs;
    LogicalAndInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "logical_and." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/* dest = lhs || rhs */
struct LogicalOrInstr : IRInstr {
    RegParam dest, lhs, rhs;
    LogicalOrInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d, t), lhs(l, t), rhs(r, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "logical_or." + irtype_name(type) + " " + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};


/** dest = funcLabel(args...)  — args and dest are registers */
struct CallInstr : IRInstr {
    string funcLabel;
    RegParam dest;
    vector<RegParam> args;

    CallInstr(BasicBlock *bb, string label, const Reg d,
              const vector<Reg> &argRegs, const vector<IRType> &argTypes, const IRType t = IRType::INT32)
        : IRInstr(bb, t), funcLabel(std::move(label)), dest(d, t) {
        for (size_t i = 0; i < argRegs.size(); ++i) {
            IRType at = (i < argTypes.size()) ? argTypes[i] : t;
            args.emplace_back(argRegs[i], at);
        }
    }

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        string s = "call." + irtype_name(type) + " "
                   + dest.to_string() + " = " + funcLabel + "(";
        for (size_t i = 0; i < args.size(); ++i) {
            if (i) s += ", ";
            s += args[i].to_string();
        }
        return s + ")";
    }
};

/** dest(INT32) = (int32_t) src(FLOAT64) — truncation toward zero */
struct F64ToI32Instr : IRInstr {
    RegParam dest; // INT32
    RegParam src;  // FLOAT64

    F64ToI32Instr(BasicBlock *bb, const Reg d, const Reg s)
        : IRInstr(bb, IRType::INT32), dest(d, IRType::INT32), src(s, IRType::FLOAT64) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "ftoi " + dest.to_string() + ", " + src.to_string();
    }
};

/** dest(FLOAT64) = (double) src(INT32) — integer to double conversion */
struct I32ToF64Instr : IRInstr {
    RegParam dest; // FLOAT64
    RegParam src;  // INT32

    I32ToF64Instr(BasicBlock *bb, const Reg d, const Reg s)
        : IRInstr(bb, IRType::FLOAT64), dest(d, IRType::FLOAT64), src(s, IRType::INT32) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "i32_to_f64 " + dest.to_string() + ", " + src.to_string();
    }
};

/** dest(INT32) = (int32_t) src(FLOAT64) — float to int conversion (existing but missing struct) */
struct FToIInstr : IRInstr {
    RegParam dest; // INT32
    RegParam src;  // FLOAT64

    FToIInstr(BasicBlock *bb, const Reg d, const Reg s)
        : IRInstr(bb, IRType::INT32), dest(d, IRType::INT32), src(s, IRType::FLOAT64) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "f_to_i " + dest.to_string() + ", " + src.to_string();
    }
};

/** dest(INT32) = (int32_t) src(INT8) — int8 to int32 conversion (sign extension) */
struct I8ToI32Instr : IRInstr {
    RegParam dest; // INT32
    RegParam src;  // INT8

    I8ToI32Instr(BasicBlock *bb, const Reg d, const Reg s)
        : IRInstr(bb, IRType::INT32), dest(d, IRType::INT32), src(s, IRType::INT8) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "i8_to_i32 " + dest.to_string() + ", " + src.to_string();
    }
};

/** dest(INT8) = (int8_t) src(INT32) — int32 to int8 conversion (truncation) */
struct I32ToI8Instr : IRInstr {
    RegParam dest; // INT8
    RegParam src;  // INT32

    I32ToI8Instr(BasicBlock *bb, const Reg d, const Reg s)
        : IRInstr(bb, IRType::INT8), dest(d, IRType::INT8), src(s, IRType::INT32) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "i32_to_i8 " + dest.to_string() + ", " + src.to_string();
    }
};

/** Return — the return value must already be in Reg::RET */
struct RetInstr : IRInstr {
    explicit RetInstr(BasicBlock *bb, const IRType t = IRType::INT32)
        : IRInstr(bb, t) {}

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "ret." + irtype_name(type);
    }
};
