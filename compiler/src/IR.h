#ifndef IR_H
#define IR_H

#include <map>
#include <vector>
#include <string>
#include <iostream>

#include "asm/AsmGenerator.h"

using namespace std;

// Declarations from the parser -- replace with your own
#include "type.h"

class BasicBlock;
class CFG;
class AsmGenerator;

//! The class for one 3-address instruction
class IRInstr {

   public:
        /** The instructions themselves -- feel free to subclass instead */
        typedef enum {
                ldconst,
                copy,
                add,
                sub,
                mul,
                div,
                bit_not,
                bit_and,
                bit_or,
                bit_xor,
                rmem,
                wmem,
                call,
                cmp_eq,
                cmp_ne,
                cmp_lt,
                cmp_le,
                cmp_gt,
                cmp_ge,
                cmp_mod,
                logical_and,
                logical_or,
                ret
        } Operation;


        /**  constructor */
        IRInstr(BasicBlock* bb_, Operation op, Type t, vector<string> params);

        /** Actual code generation */
        void gen_asm(ostream &o); /**< x86 assembly code generation for this IR instruction */
        void gen_asm_instr(ostream &o); /**< Delegate to AsmGenerator for instruction generation */
public:
        Operation op; /**< The operation type */
        vector<string> params; /**< For 3-op instrs: d, x, y; for ldconst: d, c;  For call: label, d, params;  for wmem and rmem: choose yourself */
private:
        BasicBlock* bb; /**< The BB this instruction belongs to, which provides a pointer to the CFG this instruction belong to */
        Type t;
};

//! Terminator kinds for basic blocks
enum class TerminatorKind {
    BRANCH,      // conditional: test_var !=0 ? true_target : false_target
    JMP,         // unconditional to target
    FALLTHROUGH  // implicit to sequential next block (no asm jmp needed)
};

//! Terminator instruction - every basic block must end with exactly one terminator
class TerminatorInstr : public IRInstr {
public:
    TerminatorKind kind;
    std::string test_var_name;  // for BRANCH only
    BasicBlock* true_target;    // for BRANCH/JMP
    BasicBlock* false_target;   // for BRANCH only

    // Constructor for BRANCH terminator
    TerminatorInstr(BasicBlock* bb_, const std::string& test_var, BasicBlock* true_target, BasicBlock* false_target)
        : IRInstr(bb_, IRInstr::Operation::ret, Type::INT, {}),
          kind(TerminatorKind::BRANCH),
          test_var_name(test_var),
          true_target(true_target),
          false_target(false_target) {}

    // Constructor for JMP terminator
    TerminatorInstr(BasicBlock* bb_, BasicBlock* target)
        : IRInstr(bb_, IRInstr::Operation::ret, Type::INT, {}),
          kind(TerminatorKind::JMP),
          true_target(target),
          false_target(nullptr) {}

    // Constructor for FALLTHROUGH terminator
    TerminatorInstr(BasicBlock* bb_)
        : IRInstr(bb_, IRInstr::Operation::ret, Type::INT, {}),
          kind(TerminatorKind::FALLTHROUGH),
          true_target(nullptr),
          false_target(nullptr) {}
};

/**  The class for a basic block */
class BasicBlock {
  public:
         BasicBlock(CFG* cfg, string entry_label);
         void gen_asm(ostream &o); /**< x86 assembly code generation for this basic block */
         void add_IRInstr(IRInstr::Operation op, Type t, vector<string> params);
         int calculateRequiredStackSpace();
         void allocateVariable(string name, Type type);
         void add_var_to_symbol_table(string name, Type t);
         void add_function_to_symbol_table(string name, Type returnType, vector<Type> paramTypes);
         string create_new_tempvar(Type t);
         int get_var_index(string name);
         Type get_var_type(string name);
 
         void add_param_to_symbol_table(string name, Type t, int offset) {
                 SymbolType[name] = t;
                 SymbolIndex[name] = offset;
         }
         void reset_symbol_index();
 
         vector<pair<string, int>> get_params(int minOffset = 16) const {
                 vector<pair<string, int>> params;
                 for (const auto& pair : SymbolIndex) {
                         if (pair.second >= minOffset) {
                                 params.push_back(pair);
                         }
                 }
                 return params;
         }
 
         bool hasTerminator() const { 
             if (terminator != nullptr) return true;
             for (auto instr : instrs) {
                 if (instr->op == IRInstr::ret) return true;
             }
             return false;
         }
         void setTerminator(TerminatorInstr* t);
         BasicBlock* getSuccessor(bool is_true) const;
         
         TerminatorInstr* terminator;
         string label;
         CFG* cfg;
         vector<IRInstr*> instrs;
         string test_var_name;
         bool has_var(string name) const { return SymbolIndex.find(name) != SymbolIndex.end();}
         
         // Added to share symbols across blocks
         map<string, Type>& getSymbolType() { return SymbolType; }
         map<string, int>& getSymbolIndex() { return SymbolIndex; }
         int& getNextFreeSymbolIndex();
 
  protected:
         map <string, Type> SymbolType;
         map <string, int> SymbolIndex;
 };

/** The class for the control flow graph, also includes the symbol table */
class CFG {
  public:
         CFG(TargetArch arch);
         void add_bb(BasicBlock* bb);
         void gen_asm(ostream& o);
         string IR_reg_to_asm(string reg);
         void gen_asm_prologue(ostream& o);
         void gen_asm_epilogue(ostream& o);
         void genOptimizedPrologue(ostream& o);
         int calculateRequiredStackSpace();
         BasicBlock* findBBByVariable(string var);
         string new_BB_name();
         vector<BasicBlock*>& getBBs() { return bbs; }
         vector<BasicBlock*>& getStackBBs() { return bbStack; }
         BasicBlock* getCurrentBB() {
                 if (!bbStack.empty()) return bbStack.back();
                 return current_bb;
         }
         void push_bb(BasicBlock* bb) { bbStack.push_back(bb); }
         BasicBlock* pop_bb() {
                 if (bbStack.empty()) return nullptr;
                 BasicBlock* bb = bbStack.back();
                 bbStack.pop_back();
                 return bb;
         }
         BasicBlock* current_bb;

         int& getNextFreeSymbolIndex();

         struct FunctionSignature {
                 string name;
                 string label;
                 Type returnType;
                 vector<Type> paramTypes;
                 vector<string> paramNames;
                 BasicBlock* entryBB;
         };

         void add_function(string name, Type returnType, vector<Type> paramTypes, vector<string> paramNames);
         FunctionSignature* get_function(string name);
         vector<FunctionSignature>& get_functions() { return functions; }
         BasicBlock* create_function_declaration(string name, Type returnType, vector<Type> paramTypes, vector<string> paramNames);
         BasicBlock* getBBByName(string name);
         void enter_function_definition(string name);
         BasicBlock* getOrCreateFunctionEntryBB(string name, Type returnType, vector<Type> paramTypes, vector<string> paramNames);
         const vector<FunctionSignature>& getFunctions() const { return functions; }

  protected:
         int nextBBnumber;
         int nextFreeSymbolIndex;
         vector <BasicBlock*> bbs;
         vector <BasicBlock*> bbStack;
  public:
         AsmGenerator* asmGenerator;
         bool validate();
 private:
         vector<FunctionSignature> functions;
         map<string, int> functionIndex;
};

#endif
