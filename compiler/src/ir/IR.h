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
    /** Like emit_binop but the result is always INT32 (comparisons). */
    template<class CmpInstr>
    StackParam emit_cmp_binop(const StackParam& lhs, const StackParam& rhs) {
        add_IRInstr(new LoadStackInstr(this, Reg::W0, lhs.name, lhs.type));
        add_IRInstr(new LoadStackInstr(this, Reg::W1, rhs.name, rhs.type));
        add_IRInstr(new CmpInstr(this, Reg::W0, Reg::W0, Reg::W1, lhs.type));
        string tmp = create_new_tempvar(IRType::INT32);
        add_IRInstr(new StoreStackInstr(this, tmp, Reg::W0, IRType::INT32));
        return StackParam(tmp, IRType::INT32);
    }

    template<class BinInstr>
    StackParam emit_binop(const StackParam& lhs, const StackParam& rhs) {
        add_IRInstr(new LoadStackInstr(this, Reg::W0, lhs.name, lhs.type));
        add_IRInstr(new LoadStackInstr(this, Reg::W1, rhs.name, rhs.type));
        add_IRInstr(new BinInstr(this, Reg::W0, Reg::W0, Reg::W1, lhs.type));
        string tmp = create_new_tempvar(lhs.type);
        add_IRInstr(new StoreStackInstr(this, tmp, Reg::W0, lhs.type));
        return StackParam(tmp, lhs.type);
    }

    template<class UnaryInstr>
    StackParam emit_unop(const StackParam& src) {
        add_IRInstr(new LoadStackInstr(this, Reg::W0, src.name, src.type));
        add_IRInstr(new UnaryInstr(this, Reg::W0, Reg::W0, src.type));
        string tmp = create_new_tempvar(src.type);
        add_IRInstr(new StoreStackInstr(this, tmp, Reg::W0, src.type));
        return StackParam(tmp, src.type);
    }

    // Helper functions for automatic memory allocation
    int  calculateRequiredStackSpace();
    void allocateVariable(string name, IRType type);

    // Symbol table methods
    void   add_var_to_symbol_table(string name, IRType t);
    string create_new_tempvar(IRType t);
    int    get_var_index(string name);
    int    get_var_index_or_none(const string& name) const {
        auto it = SymbolIndex.find(name);
        return (it != SymbolIndex.end()) ? it->second : INT_MIN;
    }
    IRType get_var_type(string name);

    void add_param_to_symbol_table(string name, IRType t, int offset) {
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
    map<string, IRType> SymbolType;
    map<string, int>    SymbolIndex;
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
        string           name, label;
        IRType           returnType;
        vector<IRType>   paramTypes;
        vector<string>   paramNames;
        BasicBlock*      entryBB = nullptr;
        vector<BasicBlock*> bbs;  // Basic blocks for this function
    };

    void               add_function(string name, IRType returnType,
                                    vector<IRType> paramTypes, vector<string> paramNames);
    FunctionSignature* get_function(string name);
    vector<FunctionSignature>& get_functions() { return functions; }
    BasicBlock*        create_function_entry(string name, IRType returnType,
                                             vector<IRType> paramTypes, vector<string> paramNames);
    
    // Methods for multi-function support
    void               setCurrentFunction(string name) { currentFunctionName = name; }
    string             getCurrentFunction() const { return currentFunctionName; }

    AsmGenerator* asmGenerator;

protected:
    int                 nextBBnumber = 0;
    vector<BasicBlock*> bbs;
    vector<BasicBlock*> bbStack;
    string              currentFunctionName;  // Track current function being processed

private:
    vector<FunctionSignature> functions;
    map<string, int>          functionIndex;
};
