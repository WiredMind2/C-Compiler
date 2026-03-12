#ifndef IR_H
#define IR_H

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <memory>
#include <climits>

#include "asm/AsmGenerator.h"
#include "Reg.h"
#include "type.h"

using namespace std;

class BasicBlock;
class CFG;
class AsmGenerator;

// ============================================================
//  Parameters
// ============================================================

enum class ParamKind { Const, Stack, Reg };

struct Param {
    virtual ~Param() = default;
    virtual ParamKind kind() const = 0;
    virtual string to_string() const = 0;
};

/** A compile-time constant with its IRType (replaces the old ConstParam) */
struct ConstParam : Param {
    TypedConst val;
    explicit ConstParam(TypedConst v)       : val(v) {}
    ParamKind kind() const override { return ParamKind::Const; }
    string to_string() const override { return val.to_string(); }
};

/** A variable or temporary allocated on the stack, identified by name */
struct StackParam : Param {
    string name;
    explicit StackParam(string n) : name(std::move(n)) {}
    ParamKind kind() const override { return ParamKind::Stack; }
    string to_string() const override { return name; }
};

/** A machine register, identified by the architecture-agnostic Reg enum */
struct RegParam : Param {
    Reg reg;
    explicit RegParam(Reg r) : reg(r) {}
    ParamKind kind() const override { return ParamKind::Reg; }
    string to_string() const override { return reg_name(reg); }
};

// ============================================================
//  Forward declarations
// ============================================================
struct LdConstInstr;
struct CopyRegInstr;
struct StoreStackInstr;
struct LoadStackInstr;
struct AddInstr;
struct SubInstr;
struct MulInstr;
struct DivInstr;
struct BitNotInstr;
struct BitAndInstr;
struct BitOrInstr;
struct BitXorInstr;
struct CmpEqInstr;
struct CmpLtInstr;
struct CmpLeInstr;
struct CallInstr;
struct RetInstr;

// ============================================================
//  IRInstr — abstract base
// ============================================================

class IRInstr {
public:
    virtual ~IRInstr() = default;

    virtual void accept(AsmGenerator& gen, ostream& o) = 0;
    virtual string to_string() const = 0;

    void gen_asm(ostream& o);

    BasicBlock* bb;
    IRType      type; ///< the type this instruction operates on

protected:
    explicit IRInstr(BasicBlock* bb_, IRType t = IRType::INT32)
        : bb(bb_), type(t) {}
};

// ============================================================
//  Concrete IRInstr subclasses
//  All register operands use Reg enum.
//  All stack operands use variable name strings (via StackParam).
// ============================================================

/** Load constant into register:  dest = value */
struct LdConstInstr : IRInstr {
    RegParam   dest;
    TypedConst val;
    LdConstInstr(BasicBlock* bb, Reg d, TypedConst v)
        : IRInstr(bb, v.type), dest(d), val(v) {}
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
        return "ldconst." + irtype_name(type) + " "
             + dest.to_string() + ", " + val.to_string();
    }
};

/** Copy register to register:  dest = src */
struct CopyRegInstr : IRInstr {
    RegParam dest, src;
    CopyRegInstr(BasicBlock* bb, Reg d, Reg s, IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), src(s) {}
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
        return "copy_reg." + irtype_name(type) + " "
             + dest.to_string() + ", " + src.to_string();
    }
};

/** Store register to stack slot:  stack[dest] = src */
struct StoreStackInstr : IRInstr {
    StackParam dest;
    RegParam   src;
    StoreStackInstr(BasicBlock* bb, const string& d, Reg s, IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), src(s) {}
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
        return "store_stack." + irtype_name(type) + " "
             + dest.to_string() + ", " + src.to_string();
    }
};

/** Load stack slot into register:  dest = stack[src] */
struct LoadStackInstr : IRInstr {
    RegParam   dest;
    StackParam src;
    LoadStackInstr(BasicBlock* bb, Reg d, const string& s, IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), src(s) {}
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
        return "load_stack." + irtype_name(type) + " "
             + dest.to_string() + ", " + src.to_string();
    }
};

/** dest = lhs + rhs  (all registers) */
struct AddInstr : IRInstr {
    RegParam dest, lhs, rhs;
    AddInstr(BasicBlock* bb, Reg d, Reg l, Reg r, IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {}
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
        return "add." + irtype_name(type) + " "
             + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = lhs - rhs */
struct SubInstr : IRInstr {
    RegParam dest, lhs, rhs;
    SubInstr(BasicBlock* bb, Reg d, Reg l, Reg r, IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {}
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
        return "sub." + irtype_name(type) + " "
             + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = lhs * rhs */
struct MulInstr : IRInstr {
    RegParam dest, lhs, rhs;
    MulInstr(BasicBlock* bb, Reg d, Reg l, Reg r, IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {}
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
        return "mul." + irtype_name(type) + " "
             + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = lhs / rhs */
struct DivInstr : IRInstr {
    RegParam dest, lhs, rhs;
    DivInstr(BasicBlock* bb, Reg d, Reg l, Reg r, IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {}
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
        return "div." + irtype_name(type) + " "
             + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = ~src */
struct BitNotInstr : IRInstr {
    RegParam dest, src;
    BitNotInstr(BasicBlock* bb, Reg d, Reg s, IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), src(s) {}
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
        return "bit_not." + irtype_name(type) + " "
             + dest.to_string() + ", " + src.to_string();
    }
};

/** dest = lhs & rhs */
struct BitAndInstr : IRInstr {
    RegParam dest, lhs, rhs;
    BitAndInstr(BasicBlock* bb, Reg d, Reg l, Reg r, IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {}
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
        return "bit_and." + irtype_name(type) + " "
             + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = lhs | rhs */
struct BitOrInstr : IRInstr {
    RegParam dest, lhs, rhs;
    BitOrInstr(BasicBlock* bb, Reg d, Reg l, Reg r, IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {}
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
        return "bit_or." + irtype_name(type) + " "
             + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = lhs ^ rhs */
struct BitXorInstr : IRInstr {
    RegParam dest, lhs, rhs;
    BitXorInstr(BasicBlock* bb, Reg d, Reg l, Reg r, IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {}
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
        return "bit_xor." + irtype_name(type) + " "
             + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = (lhs == rhs) */
struct CmpEqInstr : IRInstr {
    RegParam dest, lhs, rhs;
    CmpEqInstr(BasicBlock* bb, Reg d, Reg l, Reg r, IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {}
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
        return "cmp_eq." + irtype_name(type) + " "
             + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = (lhs < rhs) */
struct CmpLtInstr : IRInstr {
    RegParam dest, lhs, rhs;
    CmpLtInstr(BasicBlock* bb, Reg d, Reg l, Reg r, IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {}
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
        return "cmp_lt." + irtype_name(type) + " "
             + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = (lhs <= rhs) */
struct CmpLeInstr : IRInstr {
    RegParam dest, lhs, rhs;
    CmpLeInstr(BasicBlock* bb, Reg d, Reg l, Reg r, IRType t = IRType::INT32)
        : IRInstr(bb, t), dest(d), lhs(l), rhs(r) {}
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
        return "cmp_le." + irtype_name(type) + " "
             + dest.to_string() + ", " + lhs.to_string() + ", " + rhs.to_string();
    }
};

/** dest = funcLabel(args...)  — args and dest are registers */
struct CallInstr : IRInstr {
    string           funcLabel;
    RegParam         dest;
    vector<RegParam> args;
    CallInstr(BasicBlock* bb, const string& label, Reg d,
              vector<Reg> argRegs, IRType t = IRType::INT32)
        : IRInstr(bb, t), funcLabel(label), dest(d) {
        for (Reg r : argRegs) args.emplace_back(r);
    }
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
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
    explicit RetInstr(BasicBlock* bb, IRType t = IRType::INT32)
        : IRInstr(bb, t) {}
    void accept(AsmGenerator& gen, ostream& o) override;
    string to_string() const override {
        return "ret." + irtype_name(type);
    }
};

// ============================================================
//  BasicBlock
// ============================================================

class BasicBlock {
public:
    BasicBlock(CFG* cfg, string entry_label);
    void gen_asm(ostream& o);

    void add_IRInstr(IRInstr* instr);


    /** Emit: load varName into R0, op with R1 loaded from rhs,
     *  store result into a new temp.  Returns the temp name.
     *  Template param is the instruction type (AddInstr, SubInstr, …). */
    template<class BinInstr>
    string emit_binop(const string& lhs_var, const string& rhs_var,
                      IRType t = IRType::INT32) {
        add_IRInstr(new LoadStackInstr(this, Reg::W0_32, lhs_var, t));
        add_IRInstr(new LoadStackInstr(this, Reg::W1_32, rhs_var, t));
        add_IRInstr(new BinInstr(this, Reg::W0_32, Reg::W0_32, Reg::W1_32, t));
        string tmp = create_new_tempvar(IRType_to_Type(t));
        add_IRInstr(new StoreStackInstr(this, tmp, Reg::W0_32, t));
        return tmp;
    }

    /** Emit: load src_var → R0, apply unary op, store → new temp. */
    template<class UnaryInstr>
    string emit_unop(const string& src_var, IRType t = IRType::INT32) {
        add_IRInstr(new LoadStackInstr(this, Reg::W0_32, src_var, t));
        add_IRInstr(new UnaryInstr(this, Reg::W0_32, Reg::W0_32, t));
        string tmp = create_new_tempvar(IRType_to_Type(t));
        add_IRInstr(new StoreStackInstr(this, tmp, Reg::W0_32, t));
        return tmp;
    }

    // Helper functions for automatic memory allocation
    int  calculateRequiredStackSpace();
    void allocateVariable(string name, Type type);

    // Symbol table methods
    void   add_var_to_symbol_table(string name, Type t);
    string create_new_tempvar(Type t);
    int    get_var_index(string name);
    int    get_var_index_or_none(const string& name) const {
        auto it = SymbolIndex.find(name);
        return (it != SymbolIndex.end()) ? it->second : INT_MIN;
    }
    Type   get_var_type(string name);

    void add_param_to_symbol_table(string name, Type t, int offset) {
        SymbolType[name] = t;
        SymbolIndex[name] = offset;
    }
    void reset_symbol_index() { nextFreeSymbolIndex = -4; }

    BasicBlock* exit_true;
    BasicBlock* exit_false;
    string      label;
    CFG*        cfg;
    vector<IRInstr*> instrs;
    string      test_var_name;

protected:
    int nextFreeSymbolIndex = -4;
    map<string, Type> SymbolType;
    map<string, int>  SymbolIndex;

private:
    /** Convert IRType back to parser Type for symbol table */
    static Type IRType_to_Type(IRType t) {
        switch (t) {
            case IRType::INT32:
            case IRType::INT64:   return INT;
            default:              return INT;
        }
    }
};

// ============================================================
//  CFG
// ============================================================

class CFG {
public:
    explicit CFG(TargetArch arch);

    void add_bb(BasicBlock* bb);

    void gen_asm(ostream& o);
    void gen_asm_instr(ostream& o, IRInstr* instr);
    void gen_asm_prologue(ostream& o);
    void gen_asm_epilogue(ostream& o);
    void gen_control_flow(ostream& o, BasicBlock* bb);

    int  calculateRequiredStackSpace();

    BasicBlock* findBBByVariable(const string& var);
    string      new_BB_name();

    vector<BasicBlock*>& getBBs()      { return bbs; }
    vector<BasicBlock*>& getStackBBs() { return bbStack; }

    BasicBlock* current_bb;

    struct FunctionSignature {
        string         name, label;
        Type           returnType;
        vector<Type>   paramTypes;
        vector<string> paramNames;
    };

    void               add_function(string name, Type returnType,
                                    vector<Type> paramTypes, vector<string> paramNames);
    FunctionSignature* get_function(string name);
    vector<FunctionSignature>& get_functions() { return functions; }
    BasicBlock*        create_function_entry(string name, Type returnType,
                                             vector<Type> paramTypes, vector<string> paramNames);

    AsmGenerator* asmGenerator;

protected:
    int                 nextBBnumber = 0;
    vector<BasicBlock*> bbs;
    vector<BasicBlock*> bbStack;

private:
    vector<FunctionSignature> functions;
    map<string, int>          functionIndex;
};

#endif // IR_H
