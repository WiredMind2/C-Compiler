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
    friend class CFG;
public:
    BasicBlock(CFG* cfg, string entry_label, bool is_loop = false);
    void gen_asm(ostream& o);

    void add_IRInstr(IRInstr* instr);

    template<class CmpInstr>
    StackParam emit_cmp_binop(const StackParam& lhs, const StackParam& rhs) {
        Reg lhs_register = Reg::W0;
        Reg rhs_register = Reg::W1;

        add_IRInstr(new LoadStackInstr(this, lhs_register, lhs.name, lhs.type));
        add_IRInstr(new LoadStackInstr(this, rhs_register, rhs.name, rhs.type));

        IRType target_operation_type = operation_type_from_operand_types(lhs, rhs);

        if (lhs.type != target_operation_type) {
            generate_conversion_instruction(lhs_register, lhs.type, Reg::W2, target_operation_type);
            lhs_register = Reg::W2;
        }

        if (rhs.type != target_operation_type) {
            generate_conversion_instruction(rhs_register, rhs.type, Reg::W3, target_operation_type);
            rhs_register = Reg::W3;
        }

        add_IRInstr(new CmpInstr(this, lhs_register, lhs_register, rhs_register, target_operation_type));
        string tmp = create_new_tempvar(IRType::INT32);
        add_IRInstr(new StoreStackInstr(this, tmp, Reg::W0, IRType::INT32));
        return StackParam(tmp, IRType::INT32);
    }

    template<class BinInstr>
    StackParam emit_binop(const StackParam& lhs, const StackParam& rhs) {
        Reg lhs_register = Reg::W0;
        Reg rhs_register = Reg::W1;

        add_IRInstr(new LoadStackInstr(this, lhs_register, lhs.name, lhs.type));
        add_IRInstr(new LoadStackInstr(this, rhs_register, rhs.name, rhs.type));

        IRType target_operation_type = operation_type_from_operand_types(lhs, rhs);

        if (lhs.type != target_operation_type) {
            generate_conversion_instruction(lhs_register, lhs.type, Reg::W2, target_operation_type);
            lhs_register = Reg::W2;
        }

        if (rhs.type != target_operation_type) {
            generate_conversion_instruction(rhs_register, rhs.type, Reg::W3, target_operation_type);
            rhs_register = Reg::W3;
        }

        add_IRInstr(new BinInstr(this, lhs_register, lhs_register, rhs_register, target_operation_type));
        string tmp = create_new_tempvar(target_operation_type);
        add_IRInstr(new StoreStackInstr(this, tmp, lhs_register, target_operation_type));
        return StackParam(tmp, target_operation_type);
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
    int    allocate_bytes_on_symbol_table(int size);
    string create_new_tempvar(IRType t);
    int    get_var_index(string name);
    BasicBlock* get_var_owner_bb(string name);
    int    get_var_index_or_none(const string& name) const {
        auto it = SymbolIndex.find(name);
        return (it != SymbolIndex.end()) ? it->second : INT_MIN;
    }
    
    string resolve_var_name(string name) {
        if (name.substr(0, 4) == "!tmp") return name;
        BasicBlock* owner = get_var_owner_bb(name);
        return owner ? name + "@" + owner->label : name;
    }

    IRType get_var_type(string name);
    
    bool is_array(const string& name) const {
        auto it = isArrayMap.find(name);
        return it != isArrayMap.end() && it->second;
    }

    void add_param_to_symbol_table(string name, IRType t, int offset) {
        SymbolType[name] = t;
        SymbolIndex[name] = offset;
        SymbolType[name + "@" + this->label] = t;
        SymbolIndex[name + "@" + this->label] = offset;
    }

    void set_is_array(const string& name, bool isArr) {
        isArrayMap[name] = isArr;
        isArrayMap[name + "@" + this->label] = isArr;
    }

    void reset_symbol_index();

    void generate_conversion_instruction(Reg initial_register, IRType initial_type, Reg dest_register, IRType dest_type);

    BasicBlock* exit_true;
    BasicBlock* exit_false;
    string      label;
    CFG*        cfg;
    vector<IRInstr*> instrs;
    string      test_var_name;
    bool        is_loop = false;
    BasicBlock* loop_continue_target = nullptr;
    BasicBlock* loop_break_target    = nullptr;
    string      functionName;

protected:
    int nextFreeSymbolIndex = 0;
    map<string, IRType> SymbolType;
    map<string, int>    SymbolIndex;
    map<string, bool>   isArrayMap;
    map<string, IRType> arrayElementType;

    // Return the desired operation type from the types of the operands
    IRType operation_type_from_operand_types(const StackParam& lhs, const StackParam& rhs);

    friend class CFG;
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
    void dump_symbol_table(std::ostream &o);
    void dump_instructions(std::ostream &o);

    // Control whether IR is emitted as assembly comments
    void set_emit_ir_comments(bool enabled);
    bool get_emit_ir_comments() const;

    string create_new_tempvar(IRType t);

    int  calculateRequiredStackSpace(const string& funcName = "");

    // Array element type helpers
    void set_array_element_type(const string& name, IRType t) { 
        entry_bb->arrayElementType[name] = t; 
        if (current_bb) entry_bb->arrayElementType[name + "@" + current_bb->label] = t;
    }
    IRType get_array_element_type(const string& name) const;
    bool has_array_element_type(const string& name) const;

    BasicBlock* findBBByVariable(const string& var);
    string      new_BB_name();

    int  getNextFreeSymbolIndex() const { return nextFreeSymbolIndex; }
    void setNextFreeSymbolIndex(int index) { nextFreeSymbolIndex = index; }

    vector<BasicBlock*>& getBBs()      { return bbs; }
    vector<BasicBlock*>& getStackBBs() { return bbStack; }

    BasicBlock* current_bb;
    BasicBlock* entry_bb;
    BasicBlock* decl_target_bb = nullptr;
    BasicBlock* current_break_bb = nullptr;
    BasicBlock* current_continue_bb = nullptr;

    struct FunctionSignature {
        string           name, label;
        IRType           returnType;
        vector<IRType>   paramTypes;
        vector<string>   paramNames;
        BasicBlock*      entryBB = nullptr;
        vector<BasicBlock*> bbs;  // Basic blocks for this function
        int              cachedStackSpace = -1; // -1 indicates not computed yet
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
    int                 nextTempVarNumber = 0;
    int                 nextFreeSymbolIndex = -8;
    vector<BasicBlock*> bbs;
    vector<BasicBlock*> bbStack;
    string              currentFunctionName;  // Track current function being processed

private:
    vector<FunctionSignature> functions;
    map<string, int>          functionIndex;
};
