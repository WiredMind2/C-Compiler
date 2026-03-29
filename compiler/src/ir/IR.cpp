#include "IR.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <cassert>

#include "../asm/arm64/AsmGeneratorARM64.h"
#include "../asm/x86_64/AsmGeneratorX86_64.h"

using namespace std;

// ============================================================
//  BasicBlock
// ============================================================

BasicBlock::BasicBlock(CFG* cfg, string entry_label, bool is_loop)
    : cfg(cfg), label(entry_label), functionName(cfg->getCurrentFunction()) {
    exit_true  = nullptr;
    exit_false = nullptr;
    this->is_loop = is_loop;
}

void BasicBlock::reset_symbol_index() {
    cfg->setNextFreeSymbolIndex(-8);
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

IRType BasicBlock::operation_type_from_operand_types(const StackParam& lhs, const StackParam& rhs) {
    const bool isDouble = lhs.type == IRType::FLOAT64 || rhs.type == IRType::FLOAT64;
    if (isDouble) {
        return IRType::FLOAT64;
    }

    const bool isFloat = lhs.type == IRType::FLOAT32 || rhs.type == IRType::FLOAT32;
    if (isFloat) {
        return IRType::FLOAT32;
    }

    const bool isInt = lhs.type == IRType::INT32 || rhs.type == IRType::INT32;
    if (isInt) {
        return IRType::INT32;
    }

    const bool isChar = lhs.type == IRType::INT8 || rhs.type == IRType::INT8;
    if (isChar) {
        return IRType::INT32; // integer promotion
    }

    // fallback
    throw std::runtime_error("Unkown operand types");
}

void BasicBlock::generate_conversion_instruction(Reg initial_register, IRType initial_type, Reg dest_register, IRType dest_type) {
    // No conversion needed if types are the same
    if (initial_type == dest_type) {
        return;
    }

    // Add the appropriate conversion instruction based on source and target types
    if (initial_type == IRType::INT32 && dest_type == IRType::FLOAT64) {
        add_IRInstr(new I32ToF64Instr(this, dest_register, initial_register));
    } else if (initial_type == IRType::FLOAT64 && dest_type == IRType::INT32) {
        add_IRInstr(new F64ToI32Instr(this, dest_register, initial_register));
    } else if (initial_type == IRType::FLOAT64 && dest_type == IRType::INT8) {
        add_IRInstr(new F64ToI32Instr(this, initial_register, initial_register));
        add_IRInstr(new I32ToI8Instr(this, dest_register, initial_register));
    } else if (initial_type == IRType::INT32 && dest_type == IRType::INT8) {
        add_IRInstr(new I32ToI8Instr(this, dest_register, initial_register));
    } else if (initial_type == IRType::INT8 && dest_type == IRType::INT32) {
        add_IRInstr(new I8ToI32Instr(this, dest_register, initial_register));
    } else if (initial_type == IRType::INT8 && dest_type == IRType::FLOAT64) {
        // INT8 → INT32 (sign-extend) → FLOAT64
        add_IRInstr(new I8ToI32Instr(this, initial_register, initial_register));
        add_IRInstr(new I32ToF64Instr(this, dest_register, initial_register));
    } else {
        throw runtime_error("No conversion found");
    }
}

// ============================================================
//  Symbol table
// ============================================================

void BasicBlock::add_var_to_symbol_table(string name, IRType t) {
    BasicBlock* target = (cfg && cfg->decl_target_bb) ? cfg->decl_target_bb : this;

    if (name.substr(0, 4) != "!tmp") {
        if (target->SymbolIndex.find(name) != target->SymbolIndex.end()) {
            cerr << "Error: Variable " << name
                 << " already defined in the current scope." << endl;
            exit(1);
        }
    }
    int size = irtype_size(t);
    int alloc = (size < 4) ? 4 : size;
    target->SymbolType[name] = t;
    target->SymbolIndex[name] = cfg->getNextFreeSymbolIndex();
    cfg->setNextFreeSymbolIndex(cfg->getNextFreeSymbolIndex() - alloc);
}

string BasicBlock::create_new_tempvar(IRType t) {
    string name = "!tmp" + to_string(-cfg->getNextFreeSymbolIndex());
    add_var_to_symbol_table(name, t);
    return name;
}

int BasicBlock::get_var_index(string name) {
    // 1. Check this BB's own declarations first.
    if (SymbolIndex.find(name) != SymbolIndex.end())
        return SymbolIndex[name];

    // 2. Check the alias cache — populated on first resolution at IR-gen time
    //    so that asm-gen (when the scope stack is empty) finds the right slot.
    if (aliasIndex.find(name) != aliasIndex.end())
        return aliasIndex[name];

    // 3. Search the scope stack innermost-first so inner scopes shadow outer ones.
    if (cfg) {
        auto& stack = cfg->getStackBBs();
        for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
            auto* bb = *it;
            if (bb != this && bb->get_var_index_or_none(name) != INT_MIN) {
                int idx = bb->get_var_index_or_none(name);
                // Cache so asm-gen time works without the scope stack.
                aliasIndex[name] = idx;
                aliasType[name]  = bb->SymbolType.at(name);
                return idx;
            }
        }
        // Fallback: scan all BBs of the current function (catches params declared
        // before any scope push).
        string funcName = functionName;
        if (funcName.empty()) funcName = cfg->getCurrentFunction();
        if (!funcName.empty()) {
            auto* sig = cfg->get_function(funcName);
            if (sig) {
                for (auto bb : sig->bbs) {
                    if (bb != this && bb->get_var_index_or_none(name) != INT_MIN) {
                        int idx = bb->get_var_index_or_none(name);
                        aliasIndex[name] = idx;
                        aliasType[name]  = bb->SymbolType.at(name);
                        return idx;
                    }
                }
            }
        }
    }
    cerr << "Error: Symbol " << name << " not found in symbol table." << endl;
    exit(1);
}

IRType BasicBlock::get_var_type(string name) {
    // 1. Check this BB's own declarations first.
    if (SymbolType.find(name) != SymbolType.end())
        return SymbolType[name];

    // 2. Check the alias cache.
    if (aliasType.find(name) != aliasType.end())
        return aliasType[name];

    // 3. Search the scope stack innermost-first.
    if (cfg) {
        auto& stack = cfg->getStackBBs();
        for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
            auto* bb = *it;
            if (bb != this && bb->SymbolType.find(name) != bb->SymbolType.end()) {
                IRType t = bb->SymbolType.at(name);
                aliasType[name]  = t;
                aliasIndex[name] = bb->SymbolIndex.at(name);
                return t;
            }
        }
        // Fallback: scan all BBs of the current function.
        string funcName = functionName;
        if (funcName.empty()) funcName = cfg->getCurrentFunction();
        if (!funcName.empty()) {
            auto* sig = cfg->get_function(funcName);
            if (sig) {
                for (auto bb : sig->bbs) {
                    if (bb != this && bb->SymbolType.find(name) != bb->SymbolType.end()) {
                        IRType t = bb->SymbolType.at(name);
                        aliasType[name]  = t;
                        aliasIndex[name] = bb->SymbolIndex.at(name);
                        return t;
                    }
                }
            }
        }
    }
    cerr << "Error: Symbol " << name << " not found in symbol table (type lookup)." << endl;
    exit(1);
}

int BasicBlock::calculateRequiredStackSpace() {
    assert(cfg && "BasicBlock has no parent CFG; use CFG::calculateRequiredStackSpace instead");

    string funcName = functionName.empty() ? cfg->getCurrentFunction() : functionName;
    return cfg->calculateRequiredStackSpace(funcName);
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
    nextFreeSymbolIndex = -8;
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
    string funcName = !bb->functionName.empty() ? bb->functionName : currentFunctionName;
    if (!funcName.empty()) {
        FunctionSignature* sig = get_function(funcName);
        if (sig) sig->bbs.push_back(bb);
    }
}

void CFG::gen_asm(ostream& o) { asmGenerator->gen_asm(o); }

void CFG::gen_control_flow(ostream& o, BasicBlock* bb) {
    asmGenerator->gen_control_flow(o, bb);
}

void CFG::gen_asm_instr(ostream& o, IRInstr* instr) {
    // std::cerr << ";   " << instr->to_string() << std::endl;
    instr->accept(*asmGenerator, o);
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

int CFG::calculateRequiredStackSpace(const string& funcName) {
    if (!funcName.empty()) {
        FunctionSignature* sig = get_function(funcName);
        if (sig) {
            if (sig->cachedStackSpace != -1) {
                return sig->cachedStackSpace;
            }
            int minIndex = 0;
            for (auto* bb : sig->bbs) {
                for (const auto& entry : bb->SymbolIndex) {
                    if (entry.second < minIndex) {
                        minIndex = entry.second;
                    }
                }
            }
            int usedSpace = -minIndex;
            int aligned = usedSpace;
            if (aligned % 16 != 0)
                aligned = ((aligned / 16) + 1) * 16;
            if (aligned < 16) aligned = 16;
            sig->cachedStackSpace = aligned;
            return aligned;
        }
    }

    int usedSpace = -nextFreeSymbolIndex;
    int aligned   = usedSpace;
    if (aligned % 16 != 0)
        aligned = ((aligned / 16) + 1) * 16;
    if (aligned < 16) aligned = 16;
    return aligned;
}

BasicBlock* CFG::findBBByVariable(const string& var) {
    // Search innermost-first so inner scopes properly shadow outer ones.
    auto& stack = getStackBBs();
    for (auto it = stack.rbegin(); it != stack.rend(); ++it)
        if ((*it)->get_var_index_or_none(var) != INT_MIN) return *it;

    // Fallback: scan all BBs of the current function (catches params / pre-scope BBs)
    string funcName = (current_bb && !current_bb->functionName.empty())
                          ? current_bb->functionName
                          : currentFunctionName;
    if (!funcName.empty()) {
        FunctionSignature* sig = get_function(funcName);
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

    FunctionSignature* sig = get_function(name);
    if (sig == nullptr) {
        add_function(name, returnType, paramTypes, paramNames);
        sig = get_function(name);
    } else {
        // Definition is authoritative over prior declaration/pre-scan entry.
        sig->returnType = returnType;
        sig->paramTypes = paramTypes;
        sig->paramNames = paramNames;
    }

    BasicBlock* entryBB = new BasicBlock(this, name);
    entryBB->reset_symbol_index();

    // Parameters are treated as regular local symbols.
    // The prologue in visitFunction_definition stores incoming arg registers
    // into these stack slots.
    for (size_t i = 0; i < paramNames.size(); i++) {
        entryBB->add_var_to_symbol_table(paramNames[i], paramTypes[i]);
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
