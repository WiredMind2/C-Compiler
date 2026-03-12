#include "IR.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <cassert>

#include "asm/arm64/AsmGeneratorARM64.h"
#include "asm/x86_64/AsmGeneratorX86_64.h"

using namespace std;

// Helper function: get size in bytes for a given type
static int getTypeSize(Type t) {
    switch (t) {
        case INT:
            return 4;  // 4 bytes for int
        case CHAR:
            return 1;  // 1 byte for char
        case VOID:
            return 0;  // void has no size
        default:
            return 4;  // default to 4 bytes
    }
}

// IRInstr implementation
IRInstr::IRInstr(BasicBlock* bb_, Operation op, Type t, vector<string> params)
    : bb(bb_), op(op), t(t), params(params) {}

void IRInstr::gen_asm(ostream &o) {
    // Delegate to the CFG's AsmGenerator
    bb->cfg->asmGenerator->gen_asm_instr(o, this);
}

void IRInstr::gen_asm_instr(ostream &o) {
    // Delegate to the CFG's AsmGenerator
    bb->cfg->asmGenerator->gen_asm_instr(o, this);
}


// BasicBlock implementation
BasicBlock::BasicBlock(CFG* cfg, string entry_label) : cfg(cfg), label(entry_label) {
    terminator = nullptr;
    nextFreeSymbolIndex = -4;
}

void BasicBlock::gen_asm(ostream &o) {
    // Determine if this is a function entry block
    // A block is an entry block if its label matches a function name
    bool isEntryBlock = false;
    for (const auto& func : cfg->getFunctions()) {
        if (label == func.name) {
            isEntryBlock = true;
            break;
        }
    }
    
    // Generate label
    o << label << ":\n";

    if (isEntryBlock) {
        int stackSpace = calculateRequiredStackSpace();
        o << "    pushq %rbp\n";
        o << "    movq %rsp, %rbp\n";
        o << "    subq $" << stackSpace << ", %rsp\n";

        // Copy parameters from registers to stack (x86_64 System V ABI)
        // Parameters are passed in: %rdi, %rsi, %rdx, %rcx, %r8, %r9
        static const string argRegs64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
        for (const auto& pair : get_params(16)) {
            int offset = pair.second;
            if (offset >= 16) {
                int regIndex = (offset - 16) / 8;
                if (regIndex >= 0 && regIndex < 6) {
                    o << "    movq " << argRegs64[regIndex] << ", " << offset << "(%rbp)\n";
                }
            }
        }
    }

    // Generate instructions
    for (auto instr : instrs) {
        instr->gen_asm(o);
    }

    // Generate code for the terminator
    if (terminator) {
        switch (terminator->kind) {
            case TerminatorKind::FALLTHROUGH:
                // No assembly needed for fallthrough
                break;
            case TerminatorKind::JMP:
                o << "    jmp " << terminator->true_target->label << "\n";
                break;
            case TerminatorKind::BRANCH:
                o << "    movl " << cfg->IR_reg_to_asm(terminator->test_var_name) << ", %eax\n";
                o << "    cmpl $0, %eax\n";
                o << "    je " << terminator->false_target->label << "\n";
                o << "    jmp " << terminator->true_target->label << "\n";
                break;
        }
    }
}

void BasicBlock::add_IRInstr(IRInstr::Operation op, Type t, vector<string> params) {
    instrs.push_back(new IRInstr(this, op, t, params));
}

void BasicBlock::setTerminator(TerminatorInstr* t) {
    assert(!hasTerminator() && "BasicBlock already has a terminator!");
    terminator = t;
}

BasicBlock* BasicBlock::getSuccessor(bool is_true) const {
    assert(hasTerminator() && "BasicBlock has no terminator!");
    switch (terminator->kind) {
        case TerminatorKind::BRANCH:
            return is_true ? terminator->true_target : terminator->false_target;
        case TerminatorKind::JMP:
            return terminator->true_target;
        case TerminatorKind::FALLTHROUGH:
            // For fallthrough, the successor is determined by the CFG layout
            // This would need to be handled by the CFG during layout
            return nullptr;
    }
    return nullptr;
}

// CFG implementation
CFG::CFG(TargetArch arch) {
    nextBBnumber = 0;
    current_bb = nullptr;

    switch (arch) {
        case TargetArch::ARM64:
            asmGenerator = new AsmGeneratorARM64(this);
            break;
        case TargetArch::X86_64:
            asmGenerator = new AsmGeneratorX86_64(this);
            break;
    }
}

void CFG::add_bb(BasicBlock* bb) {
    bbs.push_back(bb);
}

void CFG::gen_asm(ostream& o) {
    asmGenerator->gen_asm(o);
}

string CFG::IR_reg_to_asm(string reg) {
    return asmGenerator->IR_reg_to_asm(reg);
}

void CFG::gen_asm_prologue(ostream& o) {
    asmGenerator->gen_prologue(o);
}

void CFG::gen_asm_epilogue(ostream& o) {
    asmGenerator->gen_epilogue(o);
}

string CFG::new_BB_name() {
    return "BB" + to_string(nextBBnumber++);
}

void BasicBlock::add_var_to_symbol_table(string name, Type t) {
    if (has_var(name)) {
        cerr << "Error: Variable " << name << " already defined in this scope." << endl;
        exit(1);
    }
    SymbolType[name] = t;
    SymbolIndex[name] = nextFreeSymbolIndex;
    nextFreeSymbolIndex -= getTypeSize(t); // Update the next free index based on the size of the type
}

string BasicBlock::create_new_tempvar(Type t) {
    string name = "!tmp" + to_string(-nextFreeSymbolIndex);
    add_var_to_symbol_table(name, t);
    return name;
}

int BasicBlock::get_var_index(string name) {
    if (SymbolIndex.find(name) == SymbolIndex.end()) {
        cerr << "Error: Symbol " << name << " not found in symbol table." << endl;
        exit(1);
    }
    return SymbolIndex[name];
}

Type BasicBlock::get_var_type(string name) {
    return SymbolType[name];
}


int BasicBlock::calculateRequiredStackSpace() {
    // Calculate the exact stack space needed based on all variables
    // The nextFreeSymbolIndex is negative and represents the next free offset
    // We need to calculate how much space has been used (from -4 to nextFreeSymbolIndex)

    // Since nextFreeSymbolIndex starts at -4 and decrements by type size,
    // the total negative offset used is: abs(nextFreeSymbolIndex)
    // But we need to add padding for alignment

    int usedSpace = -nextFreeSymbolIndex;  // Convert to positive (e.g., -12 -> 12)

    // Add extra space for alignment and safety margin
    // Ensure 16-byte alignment
    int alignedSpace = usedSpace;
    if (alignedSpace % 16 != 0) {
        alignedSpace = ((alignedSpace / 16) + 1) * 16;
    }

    // Minimum 16 bytes for safety
    if (alignedSpace < 16) {
        alignedSpace = 16;
    }

    return alignedSpace;
}


int CFG::calculateRequiredStackSpace() {
    int space = 0;
    for(BasicBlock* bb: getBBs()){
        space += bb->calculateRequiredStackSpace();
    }
    return space;
}

bool CFG::validate() {
    // Every basic block must have a terminator
    for (auto bb : getBBs()) {
        if (!bb->hasTerminator()) {
            // Check if block is empty and could be removed or bridged
            if (bb->instrs.empty()) {
                // For now, let's just add a fallthrough to the next block if it exists
                // This handles cases where ANTLR creates empty blocks for merge points
                std::cerr << "Warning: Empty basic block " << bb->label << " has no terminator. Adding implicit ret 0." << std::endl;
                bb->add_IRInstr(IRInstr::ret, INT, {});
                continue;
            }
            cerr << "Error: Basic block " << bb->label << " has no terminator!" << endl;
            return false;
        }
    }
    return true;
}

void BasicBlock::allocateVariable(string name, Type type) {
    // Get the size for this type
    int size = getTypeSize(type);

    // Add to symbol table
    SymbolType[name] = type;
    SymbolIndex[name] = nextFreeSymbolIndex;

    // Update next free index based on type size
    nextFreeSymbolIndex -= size;
}

void CFG::genOptimizedPrologue(ostream& o) {
    asmGenerator->gen_prologue(o);
}


BasicBlock* CFG::findBBByVariable(string var) {
    // 1. Search in the active BB stack (most recent/nested first)
    for (auto it = bbStack.rbegin(); it != bbStack.rend(); ++it) {
        if ((*it)->has_var(var)) {
            return *it;
        }
    }
    
    // 2. Search in the current_bb if it's not in the stack
    if (current_bb && current_bb->has_var(var)) {
        return current_bb;
    }

    // 3. Last resort: search ALL blocks (for variables from other functions, which is an error in C but helps debug)
    for (auto bb : bbs) {
        if (bb->has_var(var)) {
            return bb;
        }
    }
    
    return nullptr;
}


void CFG::add_function(string name, Type returnType, vector<Type> paramTypes, vector<string> paramNames) {
    FunctionSignature sig;
    sig.name = name;
    sig.label = name;  // Use function name as label
    sig.returnType = returnType;
    sig.paramTypes = paramTypes;
    sig.paramNames = paramNames;
    functions.push_back(sig);
    functionIndex[name] = functions.size() - 1;
}

CFG::FunctionSignature* CFG::get_function(string name) {
    if (functionIndex.find(name) != functionIndex.end()) {
        return &functions[functionIndex[name]];
    }
    return nullptr;
}

BasicBlock* CFG::create_function_declaration(string name, Type returnType, vector<Type> paramTypes, vector<string> paramNames) {
    // Firstly check if the function is already declared
    if (get_function(name) != nullptr) {
        // Function already declared, but we might be defining it now.
        // If it was just a declaration, the BB hasn't been created yet.
    } else {
        // First add the function signature
        add_function(name, returnType, paramTypes, paramNames);
    }
    
    // Create a new entry basic block with the function name as label
    BasicBlock* entryBB = new BasicBlock(this, name);
    // Add parameters to the symbol table
    // In x86_64, parameters are passed in registers (rdi, rsi, rdx, rcx, r8, r9)
    // For simplicity, we'll store them in memory at positive offsets
    int paramOffset = 16;  // Start at 16(%rbp) - after return addr and saved rbp
    for (size_t i = 0; i < paramNames.size(); i++) {
        entryBB->add_param_to_symbol_table(paramNames[i], paramTypes[i], paramOffset);
        paramOffset += 8;  // Each parameter is 8 bytes (pointer size)
    }
    
    // Set the current basic block to the entry block
    current_bb = entryBB;
    add_bb(entryBB);
    
    return entryBB;
}

BasicBlock *CFG::getBBByName(string name) {
    for (auto bb : getBBs()) {
        if (bb->label == name) {
            return bb;
        }
    }
    return nullptr;
}

void CFG::enter_function_definition(string name)
// This method is called when we start generating IR for a function definition.
// It sets the current basic block to the entry block of the function.
{
    BasicBlock* entryBB = getBBByName(name);
    if (entryBB == nullptr) {
        cerr << "Error: Function " << name << " not declared." << endl;
        exit(1);
    }
    current_bb = entryBB;
    bbStack.push_back(entryBB);
}


BasicBlock* CFG::getOrCreateFunctionEntryBB(string name, Type returnType, vector<Type> paramTypes, vector<string> paramNames) {
    BasicBlock* entryBB = getBBByName(name);
    if (entryBB == nullptr) {
        // If the function entry block doesn't exist, create it
        entryBB = create_function_declaration(name, returnType, paramTypes, paramNames);
    }
    return entryBB;
}
