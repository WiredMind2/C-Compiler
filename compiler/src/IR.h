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
// class DefFonction; // Removing as it's not defined yet, will use void* or simpler structure if needed


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
		cmp_lt,
		cmp_le,
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
	// if you subclass IRInstr, each IRInstr subclass has its parameters and the previous (very important) comment becomes useless: it would be a better design.
};






/**  The class for a basic block */

/* A few important comments.
	 IRInstr has no jump instructions.
	 cmp_* instructions behaves as an arithmetic two-operand instruction (add or mult),
	  returning a boolean value (as an int)

	 Assembly jumps are generated as follows:
	 BasicBlock::gen_asm() first calls IRInstr::gen_asm() on all its instructions, and then
		    if  exit_true  is a  nullptr,
            the epilogue is generated
        else if exit_false is a nullptr,
          an unconditional jmp to the exit_true branch is generated
				else (we have two successors, hence a branch)
          an instruction comparing the value of test_var_name to true is generated,
					followed by a conditional branch to the exit_false branch,
					followed by an unconditional branch to the exit_true branch
	 The attribute test_var_name itself is defined when converting
  the if, while, etc of the AST  to IR.

Possible optimization:
     a cmp_* comparison instructions, if it is the last instruction of its block,
       generates an actual assembly comparison
       followed by a conditional jump to the exit_false branch
*/

class BasicBlock {
 public:
	BasicBlock(CFG* cfg, string entry_label);
	void gen_asm(ostream &o); /**< x86 assembly code generation for this basic block (very simple) */

	void add_IRInstr(IRInstr::Operation op, Type t, vector<string> params);

	// Helper functions for automatic memory allocation
	int calculateRequiredStackSpace(); /**< Calculate exact stack space needed based on variables */
	void allocateVariable(string name, Type type); /**< Unified variable allocation with type-based sizing */

	// symbol table methods
	void add_var_to_symbol_table(string name, Type t);
	void add_function_to_symbol_table(string name, Type returnType, vector<Type> paramTypes);
	string create_new_tempvar(Type t);
	int get_var_index(string name);
	Type get_var_type(string name);

	// Public access methods for CFG
	void add_param_to_symbol_table(string name, Type t, int offset) {
		SymbolType[name] = t;
		SymbolIndex[name] = offset;
	}
	void reset_symbol_index() { nextFreeSymbolIndex = -4; }

	// Get parameters (symbols with positive offset >= minOffset)
	// Returns pairs of (name, offset) for parameters
	vector<pair<string, int>> get_params(int minOffset = 16) const {
		vector<pair<string, int>> params;
		for (const auto& pair : SymbolIndex) {
			if (pair.second >= minOffset) {
				params.push_back(pair);
			}
		}
		return params;
	}

	// No encapsulation whatsoever here. Feel free to do better.
	BasicBlock* exit_true;  /**< pointer to the next basic block, true branch. If nullptr, return from procedure */
	BasicBlock* exit_false; /**< pointer to the next basic block, false branch. If null_ptr, the basic block ends with an unconditional jump */
	string label; /**< label of the BB, also will be the label in the generated code */
	CFG* cfg; /** < the CFG where this block belongs */
	vector<IRInstr*> instrs; /** < the instructions themselves. */
	string test_var_name;  /** < when generating IR code for an if(expr) or while(expr) etc, */
	bool has_var(string name) const { return SymbolIndex.find(name) != SymbolIndex.end();}
 protected:
 	int nextFreeSymbolIndex; /**< to allocate new symbols in the symbol table */
	map <string, Type> SymbolType; /**< part of the symbol table  */
	map <string, int> SymbolIndex; /**< part of the symbol table  */


};




/** The class for the control flow graph, also includes the symbol table */

/* A few important comments:
	 The entry block is the one with the same label as the AST function name.
	   (it could be the first of bbs, or it could be defined by an attribute value)
	 The exit block is the one with both exit pointers equal to nullptr.
     (again it could be identified in a more explicit way)

 */
class CFG {
 public:
	CFG(TargetArch arch);

	// void* ast; /**< The AST this CFG comes from */

	void add_bb(BasicBlock* bb);

	// x86 code generation: could be encapsulated in a processor class in a retargetable compiler
	void gen_asm(ostream& o);
	string IR_reg_to_asm(string reg); /**< helper method: inputs a IR reg or input variable, returns e.g. "-24(%rbp)" for the proper value of 24 */
	void gen_asm_prologue(ostream& o);
	void gen_asm_epilogue(ostream& o);
	void gen_control_flow(ostream& o, BasicBlock* bb);
	void genOptimizedPrologue(ostream& o); /**< Generate optimized prologue with exact stack space (16-byte aligned) */


	int calculateRequiredStackSpace();

	// basic block management
	BasicBlock* findBBByVariable(string var);
	string new_BB_name();
	vector<BasicBlock*>& getBBs() { return bbs; } // return all the BBs of this CFG
	vector<BasicBlock*>& getStackBBs() { return bbStack; } // return the stack of BBs of this CFG, used when generating
	// IR code from the AST:when we enter an if, while, etc, we push the current BB on the stack, and when we exit it, we pop it from the stack.

	BasicBlock* getCurrentBB() {
		if (!bbStack.empty()) {
			return bbStack.back();
		}
		// Fall back to current_bb if bbStack is empty (e.g., during assembly generation)
		return current_bb;
	}

	void push_bb(BasicBlock* bb) {
		bbStack.push_back(bb);
	}

	BasicBlock* pop_bb() {
		if (bbStack.empty()) return nullptr;
		BasicBlock* bb = bbStack.back();
		bbStack.pop_back();
		return bb;
	}

	BasicBlock* current_bb;

	// Function support
	struct FunctionSignature {
		string name;
		string label;
		Type returnType;
		vector<Type> paramTypes;
		vector<string> paramNames;
	};

	void add_function(string name, Type returnType, vector<Type> paramTypes, vector<string> paramNames);
	FunctionSignature* get_function(string name);
	vector<FunctionSignature>& get_functions() { return functions; }
	BasicBlock* create_function_declaration(string name, Type returnType, vector<Type> paramTypes, vector<string> paramNames);
	BasicBlock* getBBByName(string name);
	void enter_function_definition(string name);
	BasicBlock* getOrCreateFunctionEntryBB(string name, Type returnType, vector<Type> paramTypes, vector<string> paramNames);



 protected:
	int nextBBnumber; /**< just for naming */

	vector <BasicBlock*> bbs; /**< all the basic blocks of this CFG*/
	vector <BasicBlock*> bbStack; /**< the stack of basic blocks, used when generating IR code
	from the AST:when we enter an if, while, etc, we push the current BB on the stack, and when we exit it, we pop it from the stack. */

public:
	AsmGenerator* asmGenerator; /**< Assembly generator for the target architecture */
private:
	vector<FunctionSignature> functions; /**< List of function signatures */
	map<string, int> functionIndex; /**< Map from function name to index in functions vector */
};


#endif
