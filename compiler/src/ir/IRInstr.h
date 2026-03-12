#pragma once
#include <utility>
#include <vector>

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
    explicit IRInstr(BasicBlock *bb_, const IRType t = IRType::INT32) : type(t), bb(bb_) {
    }
};

/** Load constant into register:  dest = value */
struct LdConstInstr : IRInstr {
    RegParam dest;
    TypedConst val;

    LdConstInstr(BasicBlock *bb, const Reg d, const TypedConst &v)
        : IRInstr(bb, v.type), dest(d), val(v) {
    }

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
        : IRInstr(bb, t), dest(d), src(s) {
    }

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "copy_reg." + irtype_name(type) + " "
               + dest.to_string() + ", " + src.to_string();
    }
};

/** Store register to stack slot:  stack[dest] = src */
struct StoreStackInstr : IRInstr {
    StackParam dest;
    RegParam src;

    StoreStackInstr(BasicBlock *bb, const string &d, const Reg s, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), src(s) {
    }

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "store_stack." + irtype_name(type) + " "
               + dest.to_string() + ", " + src.to_string();
    }
};

/** Load stack slot into register:  dest = stack[src] */
struct LoadStackInstr : IRInstr {
    RegParam dest;
    StackParam src;

    LoadStackInstr(BasicBlock *bb, const Reg d, const string &s, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), src(s) {
    }

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "load_stack." + irtype_name(type) + " "
               + dest.to_string() + ", " + src.to_string();
    }
};

/** dest = lhs + rhs  (all registers) */
struct AddInstr : IRInstr {
    RegParam dest, lhs, rhs;

    AddInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {
    }

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
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {
    }

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
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {
    }

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
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {
    }

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "div." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = ~src */
struct BitNotInstr : IRInstr {
    RegParam dest, src;

    BitNotInstr(BasicBlock *bb, const Reg d, const Reg s, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), src(s) {
    }

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
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {
    }

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
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {
    }

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
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {
    }

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "bit_xor." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = (lhs == rhs) */
struct CmpEqInstr : IRInstr {
    RegParam dest, lhs, rhs;

    CmpEqInstr(BasicBlock *bb, const Reg d, const Reg l, const Reg r, const IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {
    }

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
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {
    }

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
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {
    }

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "cmp_le." + irtype_name(type) + " "
               + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = funcLabel(args...)  — args and dest are registers */
struct CallInstr : IRInstr {
    string funcLabel;
    RegParam dest;
    vector<RegParam> args;

    CallInstr(BasicBlock *bb, string label, const Reg d,
              const vector<Reg> &argRegs, const IRType t = IRType::INT32)
        : IRInstr(bb, t), funcLabel(std::move(label)), dest(d) {
        for (Reg r: argRegs) args.emplace_back(r);
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

/** Return — the return value must already be in Reg::RET */
struct RetInstr : IRInstr {
    explicit RetInstr(BasicBlock *bb, const IRType t = IRType::INT32)
        : IRInstr(bb, t) {
    }

    void accept(AsmGenerator& g, ostream &o) override;

    [[nodiscard]] string to_string() const override {
        return "ret." + irtype_name(type);
    }
};
