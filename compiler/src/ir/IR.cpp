#include "IR.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>

#include "../asm/arm64/AsmGeneratorARM64.h"
#include "../asm/x86_64/AsmGeneratorX86_64.h"

using namespace std;

// ============================================================
//  BasicBlock
// ============================================================

BasicBlock::BasicBlock(CFG *cfg, string entry_label)
    : cfg(cfg), label(entry_label) {
    exit_true = nullptr;
    exit_false = nullptr;
    nextFreeSymbolIndex = -4;
    endsWithReturn = false;
}

void BasicBlock::gen_asm(ostream &o) {
    o << label << ":\n";
    for (auto instr: instrs)
        cfg->gen_asm_instr(o, instr);
    cfg->gen_control_flow(o, this);
}

void BasicBlock::add_IRInstr(IRInstr *instr) {
    instrs.push_back(instr);
}

// ============================================================
//  Symbol table
// ============================================================

void BasicBlock::add_var_to_symbol_table(string name, IRType t) {
    if (cfg->findBBByVariable(name) != nullptr) {
        cerr << "Error: Variable " << name
                << " already defined in a former or current scope." << endl;
        exit(1);
    }
    SymbolType[name] = t;
    nextFreeSymbolIndex -= irtype_size(t);
    // Align to 8 bytes if it's an 8 byte type for better safety
    if (irtype_size(t) == 8 && (nextFreeSymbolIndex % 8) != 0) {
        nextFreeSymbolIndex &= ~7; 
    }
    SymbolIndex[name] = nextFreeSymbolIndex;
}

int BasicBlock::allocate_bytes_on_symbol_table(int size) {
    nextFreeSymbolIndex -= size;
    if (size == 8 && (nextFreeSymbolIndex % 8) != 0) {
        nextFreeSymbolIndex &= ~7;
    }
    int index = nextFreeSymbolIndex;
    return index;
}

string BasicBlock::create_new_tempvar(IRType t) {
    return cfg->create_new_tempvar(t);
}

int BasicBlock::get_var_index(string name) {
    if (SymbolIndex.find(name) != SymbolIndex.end()) {
        return SymbolIndex[name];
    }
    if (cfg->entry_bb && cfg->entry_bb != this) {
        return cfg->entry_bb->get_var_index(name);
    }
    cerr << "Error: Symbol " << name << " not found in symbol table." << endl;
    exit(1);
}

IRType BasicBlock::get_var_type(string name) {
    if (SymbolType.find(name) != SymbolType.end()) {
        return SymbolType[name];
    }
    if (cfg->entry_bb && cfg->entry_bb != this) {
        return cfg->entry_bb->get_var_type(name);
    }
    return IRType::INT32;
}

int BasicBlock::calculateRequiredStackSpace() {
    int usedSpace = -nextFreeSymbolIndex;
    int aligned = usedSpace;
    if (aligned % 16 != 0)
        aligned = ((aligned / 16) + 1) * 16;
    if (aligned < 16) aligned = 16;
    return aligned;
}

void BasicBlock::allocateVariable(string name, IRType type) {
    SymbolType[name] = type;
    nextFreeSymbolIndex -= irtype_size(type);
    if (irtype_size(type) == 8 && (nextFreeSymbolIndex % 8) != 0) {
        nextFreeSymbolIndex &= ~7;
    }
    SymbolIndex[name] = nextFreeSymbolIndex;
}

// ============================================================
//  CFG
// ============================================================

CFG::CFG(TargetArch arch) {
    nextBBnumber = 0;
    current_bb = new BasicBlock(this, new_BB_name());
    entry_bb = current_bb;
    add_bb(current_bb);

    switch (arch) {
        case TargetArch::ARM64:
            asmGenerator = new AsmGeneratorARM64(this);
            break;
        case TargetArch::X86_64:
            asmGenerator = new AsmGeneratorX86_64(this);
            break;
    }
}

void CFG::add_bb(BasicBlock *bb) { bbs.push_back(bb); }

void CFG::gen_asm(ostream &o) { asmGenerator->gen_asm(o); }

void CFG::gen_control_flow(ostream &o, BasicBlock *bb) {
    asmGenerator->gen_control_flow(o, bb);
}

void CFG::gen_asm_instr(ostream &o, IRInstr *instr) {
    cout << ";   " << instr->to_string() << endl;
    asmGenerator->gen_asm_instr(o, instr);
}

void CFG::gen_asm_prologue(ostream &o) {
    cout << ";   Prologue:" << endl;
    asmGenerator->gen_prologue(o);
}

void CFG::gen_asm_epilogue(ostream &o) {
    cout << ";   Epilogue:" << endl;
    asmGenerator->gen_epilogue(o);
}

string CFG::new_BB_name() {
    return "BB" + to_string(nextBBnumber++);
}

string CFG::create_new_tempvar(IRType t) {
    string name = "!tmp" + to_string(nextBBnumber++); // Using BB number for uniqueness
    entry_bb->add_var_to_symbol_table(name, t);
    return name;
}

int CFG::calculateRequiredStackSpace() {
    int space = 0;
    for (BasicBlock *bb: getBBs())
        space += bb->calculateRequiredStackSpace();
    return space;
}

BasicBlock *CFG::findBBByVariable(const string &var) {
    for (auto bb: getBBs()) {
        if (bb->SymbolIndex.find(var) != bb->SymbolIndex.end()) return bb;
    }
    return nullptr;
}

void CFG::add_function(string name, IRType returnType,
                       vector<IRType> paramTypes, vector<string> paramNames) {
    FunctionSignature sig;
    sig.name = name;
    sig.label = name;
    sig.returnType = returnType;
    sig.paramTypes = paramTypes;
    sig.paramNames = paramNames;
    functions.push_back(sig);
    functionIndex[name] = functions.size() - 1;
}

CFG::FunctionSignature *CFG::get_function(string name) {
    auto it = functionIndex.find(name);
    return (it != functionIndex.end()) ? &functions[it->second] : nullptr;
}

BasicBlock *CFG::create_function_entry(string name, IRType returnType,
                                        vector<IRType> paramTypes, vector<string> paramNames) {
    add_function(name, returnType, paramTypes, paramNames);

    BasicBlock *entryBB = new BasicBlock(this, name);
    entryBB->reset_symbol_index();

    int paramOffset = 16;
    for (size_t i = 0; i < paramNames.size(); i++) {
        entryBB->add_param_to_symbol_table(paramNames[i], paramTypes[i], paramOffset);
        paramOffset += 8;
    }

    current_bb = entryBB;
    entry_bb = current_bb;
    add_bb(entryBB);
    return entryBB;
}
