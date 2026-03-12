#pragma once

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <climits>

#include "../asm/AsmGenerator.h"
#include "Reg.h"
#include "IRInstr.h"
#include "IRType.h"

using namespace std;

class CFG;

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
