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

BasicBlock::BasicBlock(CFG* cfg, string entry_label, bool is_loop)
    : cfg(cfg), label(entry_label) {
    exit_true  = nullptr;
    exit_false = nullptr;
    this->is_loop = is_loop;
}

void BasicBlock::reset_symbol_index() {
    cfg->setNextFreeSymbolIndex(-4);
}

void BasicBlock::gen_asm(ostream& o) {
    o << label << ":\n";
    for (auto instr : instrs)
        cfg->gen_asm_instr(o, instr);
    cfg->gen_control_flow(o, this);
}

void BasicBlock::add_IRInstr(IRInstr* instr) {
    instrs.push_back(instr);
}

// ============================================================
//  Symbol table
// ============================================================

void BasicBlock::add_var_to_symbol_table(string name, IRType t) {
    // Allow duplicate temp variables (they're local to each BB's computation)
    // Temp variables start with "!tmp"
    if (name.substr(0, 4) != "!tmp") {
        if (cfg->findBBByVariable(name) != nullptr) {
            cerr << "Error: Variable " << name
                 << " already defined in a former or current scope." << endl;
            exit(1);
        }
    }
    SymbolType[name] = t;
    SymbolIndex[name] = cfg->getNextFreeSymbolIndex();
    cfg->setNextFreeSymbolIndex(cfg->getNextFreeSymbolIndex() - irtype_size(t));
}

string BasicBlock::create_new_tempvar(IRType t) {
    string name = "!tmp" + to_string(-cfg->getNextFreeSymbolIndex());
    add_var_to_symbol_table(name, t);
    return name;
}

int BasicBlock::get_var_index(string name) {
    // First check the current BB's symbol table
    if (SymbolIndex.find(name) != SymbolIndex.end()) {
        return SymbolIndex[name];
    }
    // Then check the scope stack (parent scopes)
    if (cfg) {
        for (auto bb : cfg->getStackBBs()) {
            if (bb != this && bb->get_var_index_or_none(name) != INT_MIN) {
                return bb->get_var_index_or_none(name);
            }
        }
        // Also check BBs in the current function
        if (!cfg->getCurrentFunction().empty()) {
            auto* sig = cfg->get_function(cfg->getCurrentFunction());
            if (sig) {
                for (auto bb : sig->bbs) {
                    if (bb != this && bb->get_var_index_or_none(name) != INT_MIN) {
                        return bb->get_var_index_or_none(name);
                    }
                }
            }
        }
    }
    // If not found, print error and exit
    cerr << "Error: Symbol " << name << " not found in symbol table." << endl;
    exit(1);
}

IRType BasicBlock::get_var_type(string name) {
    // First check the current BB's symbol table
    if (SymbolType.find(name) != SymbolType.end()) {
        return SymbolType[name];
    }
    // Then check the scope stack (parent scopes)
    if (cfg) {
        for (auto bb : cfg->getStackBBs()) {
            if (bb != this && bb->SymbolType.find(name) != bb->SymbolType.end()) {
                return bb->SymbolType[name];
            }
        }
        // Also check BBs in the current function
        if (!cfg->getCurrentFunction().empty()) {
            auto* sig = cfg->get_function(cfg->getCurrentFunction());
            if (sig) {
                for (auto bb : sig->bbs) {
                    if (bb != this && bb->SymbolType.find(name) != bb->SymbolType.end()) {
                        return bb->SymbolType[name];
                    }
                }
            }
        }
    }
    // Default to INT32 if not found (should be handled by index lookup error)
    return IRType::INT32;
}

int BasicBlock::calculateRequiredStackSpace() {
    return 0;
}

void BasicBlock::allocateVariable(string name, IRType type) {
    SymbolType[name]  = type;
    SymbolIndex[name] = cfg->getNextFreeSymbolIndex();
    cfg->setNextFreeSymbolIndex(cfg->getNextFreeSymbolIndex() - irtype_size(type));
}

// ============================================================
//  CFG
// ============================================================

CFG::CFG(TargetArch arch) {
    nextBBnumber = 0;
    nextFreeSymbolIndex = -4;
    current_bb   = new BasicBlock(this, new_BB_name());
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

void CFG::add_bb(BasicBlock* bb) { 
    bbs.push_back(bb); 
    // Also add to current function's bbs if we have a current function
    if (!currentFunctionName.empty()) {
        FunctionSignature* sig = get_function(currentFunctionName);
        if (sig) sig->bbs.push_back(bb);
    }
}

void CFG::gen_asm(ostream& o) { asmGenerator->gen_asm(o); }

void CFG::gen_control_flow(ostream& o, BasicBlock* bb) {
    asmGenerator->gen_control_flow(o, bb);
}

void CFG::gen_asm_instr(ostream& o, IRInstr* instr) {
    asmGenerator->gen_asm_instr(o, instr);
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

int CFG::calculateRequiredStackSpace() {
    int usedSpace = -nextFreeSymbolIndex;
    int aligned   = usedSpace;
    if (aligned % 16 != 0)
        aligned = ((aligned / 16) + 1) * 16;
    if (aligned < 16) aligned = 16;
    return aligned;
}

BasicBlock* CFG::findBBByVariable(const string& var) {
    // First search in the scope stack
    for (auto bb : getStackBBs())
        if (bb->get_var_index_or_none(var) != INT_MIN) return bb;
    
    // Then search in the current function's BBs
    if (!currentFunctionName.empty()) {
        FunctionSignature* sig = get_function(currentFunctionName);
        if (sig) {
            for (auto bb : sig->bbs)
                if (bb->get_var_index_or_none(var) != INT_MIN) return bb;
        }
    }
    
    return nullptr;
}

void CFG::add_function(string name, IRType returnType,
                       vector<IRType> paramTypes, vector<string> paramNames) {
    FunctionSignature sig;
    sig.name       = name;
    sig.label      = name;
    sig.returnType = returnType;
    sig.paramTypes = paramTypes;
    sig.paramNames = paramNames;
    functions.push_back(sig);
    functionIndex[name] = functions.size() - 1;
}

CFG::FunctionSignature* CFG::get_function(string name) {
    auto it = functionIndex.find(name);
    return (it != functionIndex.end()) ? &functions[it->second] : nullptr;
}

BasicBlock* CFG::create_function_entry(string name, IRType returnType,
                                       vector<IRType> paramTypes, vector<string> paramNames) {
    // Set current function name FIRST
    currentFunctionName = name;
    
    add_function(name, returnType, paramTypes, paramNames);
    
    // Get the function signature we just created
    FunctionSignature* sig = get_function(name);

    BasicBlock* entryBB = new BasicBlock(this, name);
    entryBB->reset_symbol_index();

    int paramOffset = 16;
    for (size_t i = 0; i < paramNames.size(); i++) {
        entryBB->add_param_to_symbol_table(paramNames[i], paramTypes[i], paramOffset);
        paramOffset += 8;
    }

    current_bb = entryBB;
    
    // Add to the function's bbs list (entryBB is the first BB)
    sig->bbs.push_back(entryBB);
    // Also add to global bbs list for now
    bbs.push_back(entryBB);
    
    // Store entryBB in the function signature
    sig->entryBB = entryBB;
    
    return entryBB;
}
