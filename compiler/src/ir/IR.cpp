#include "IR.h"

#include <cassert>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "../asm/arm64/AsmGeneratorARM64.h"
#include "../asm/x86_64/AsmGeneratorX86_64.h"

using namespace std;

// Global toggle for emitting IR as assembly comments
bool g_emit_ir_as_asm_comments = false;

// ============================================================
//  BasicBlock
// ============================================================

BasicBlock::BasicBlock(CFG* cfg, string entry_label, bool is_loop)
    : cfg(cfg), label(entry_label), functionName(cfg ? cfg->getCurrentFunction() : string()) {
    exit_true = nullptr;
    exit_false = nullptr;
    this->is_loop = is_loop;
}

void BasicBlock::reset_symbol_index() {
    if (cfg) cfg->setNextFreeSymbolIndex(-8);
}

std::vector<std::string> BasicBlock::get_symbol_names() const {
    std::vector<std::string> names;
    names.reserve(SymbolIndex.size());
    for (const auto& p : SymbolIndex) names.push_back(p.first);
    return names;
}

bool BasicBlock::remove_symbol(const std::string& name) {
    auto it = SymbolIndex.find(name);
    if (it == SymbolIndex.end()) return false;
    SymbolIndex.erase(it);
    SymbolType.erase(name);
    isArrayMap.erase(name);
    arrayElementType.erase(name);
    return true;
}

void BasicBlock::gen_asm(ostream& o) {
    o << label << ":\n";
    for (auto instr : instrs) cfg->gen_asm_instr(o, instr);
    cfg->gen_control_flow(o, this);
}

void BasicBlock::add_IRInstr(IRInstr* instr) { instrs.push_back(instr); }

IRType BasicBlock::operation_type_from_operand_types(const StackParam& lhs, const StackParam& rhs) {
    const bool isDouble = lhs.type == IRType::FLOAT64 || rhs.type == IRType::FLOAT64;
    if (isDouble) return IRType::FLOAT64;
    const bool isFloat = lhs.type == IRType::FLOAT32 || rhs.type == IRType::FLOAT32;
    if (isFloat) return IRType::FLOAT32;
    const bool isInt = lhs.type == IRType::INT32 || rhs.type == IRType::INT32;
    if (isInt) return IRType::INT32;
    const bool isChar = lhs.type == IRType::INT8 || rhs.type == IRType::INT8;
    if (isChar) return IRType::INT32;  // integer promotion
    throw std::runtime_error("Unknown operand types");
}

void BasicBlock::generate_conversion_instruction(Reg initial_register, IRType initial_type, Reg dest_register, IRType dest_type) {
    if (initial_type == dest_type) return;
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
        add_IRInstr(new I8ToI32Instr(this, initial_register, initial_register));
        add_IRInstr(new I32ToF64Instr(this, dest_register, initial_register));
    } else if (initial_type == IRType::INT32 && dest_type == IRType::POINTER) {
        // zero-extend 32->64 by copying the 32-bit source into the requested
        // destination register and treating that destination as POINTER-sized.
        auto* instr = new CopyRegInstr(this, dest_register, initial_register, IRType::INT32);
        instr->dest.type = IRType::POINTER;  // ensure destination register is treated as 64-bit
        add_IRInstr(instr);
    } else if (initial_type == IRType::POINTER && dest_type == IRType::INT8) {
        // Truncate pointer to char
        add_IRInstr(new I32ToI8Instr(this, dest_register, initial_register));
    } else if (initial_type == IRType::POINTER && dest_type == IRType::POINTER) {
        add_IRInstr(new CopyRegInstr(this, dest_register, initial_register, IRType::POINTER));
    } else {
        throw runtime_error("No conversion found");
    }
}

// ============================================================
//  Symbol table
// ============================================================

void BasicBlock::add_var_to_symbol_table(string name, IRType t) {
    if (name.substr(0, 4) != "!tmp") {
        if (this->get_var_index_or_none(name) != INT_MIN) {
            cerr << "Error: Variable " << name << " already defined in a former or current scope." << endl;
            exit(1);
        }
    }
    int size = irtype_size(t);
    int alloc = (size < 4) ? 4 : size;
    SymbolType[name] = t;
    SymbolType[name + "@" + this->label] = t;
    if (cfg) {
        // Ensure 8-byte types start on an 8-byte boundary to avoid overlapping
        // neighboring 4-byte slots when emitting 64-bit stores/loads.
        int cur = cfg->getNextFreeSymbolIndex();
        if (alloc >= 8) {
            if (cur % 8 != 0) {
                // insert 4-byte padding to align to 8 bytes
                cur -= 4;
                cfg->setNextFreeSymbolIndex(cur);
            }
        }
        int idx = cur - alloc;
        SymbolIndex[name] = idx;
        SymbolIndex[name + "@" + this->label] = idx;
        cfg->setNextFreeSymbolIndex(idx);
    } else {
        SymbolIndex[name] = -alloc;
        SymbolIndex[name + "@" + this->label] = -alloc;
    }
}

int BasicBlock::allocate_bytes_on_symbol_table(int size) {
    if (cfg) {
        // For allocations that will hold 8-byte values (or larger), ensure
        // the start is 8-byte aligned to avoid overlapping with adjacent
        // 4-byte slots when the backend emits 64-bit memory operations.
        if (size >= 8) {
            int cur = cfg->getNextFreeSymbolIndex();
            if (cur % 8 != 0) {
                cur -= 4;  // insert 4-byte padding to align
                cfg->setNextFreeSymbolIndex(cur);
            }
        }
        int idx = cfg->getNextFreeSymbolIndex() - size;
        cfg->setNextFreeSymbolIndex(idx);
        return idx;
    } else {
        return -size;
    }
}

string BasicBlock::create_new_tempvar(IRType t) { return cfg ? cfg->create_new_tempvar(t) : string("!tmp0"); }

int BasicBlock::get_var_index(string name) {
    if (SymbolIndex.find(name) != SymbolIndex.end()) return SymbolIndex[name];
    if (cfg) {
        // Search stack BBs in REVERSE order (innermost first)
        const auto& bbStack = cfg->getStackBBs();
        for (auto it = bbStack.rbegin(); it != bbStack.rend(); ++it) {
            BasicBlock* bb = *it;
            int idx = bb->get_var_index_or_none(name);
            if (idx != INT_MIN) return idx;
        }

        string funcName = functionName.empty() ? cfg->getCurrentFunction() : functionName;
        if (!funcName.empty()) {
            auto* sig = cfg->get_function(funcName);
            if (sig) {
                // Check all blocks in the function as a fallback (like before)
                for (auto bb : sig->bbs) {
                    int idx = bb->get_var_index_or_none(name);
                    if (idx != INT_MIN) return idx;
                }
            }
        }
        // Finally, check for globals in the entry basic block
        if (cfg->global_bb) {
            int idx = cfg->global_bb->get_var_index_or_none(name);
            if (idx != INT_MIN) return idx;
        }
    }
    cerr << "Error: Symbol " << name << " not found in symbol table." << endl;
    exit(1);
}

BasicBlock* BasicBlock::get_var_owner_bb(string name) {
    if (SymbolIndex.find(name) != SymbolIndex.end()) return this;
    if (cfg) {
        const auto& bbStack = cfg->getStackBBs();
        for (auto it = bbStack.rbegin(); it != bbStack.rend(); ++it) {
            BasicBlock* bb = *it;
            if (bb->get_var_index_or_none(name) != INT_MIN) return bb;
        }

        string funcName = functionName.empty() ? cfg->getCurrentFunction() : functionName;
        if (!funcName.empty()) {
            auto* sig = cfg->get_function(funcName);
            if (sig) {
                for (auto bb : sig->bbs) {
                    if (bb->get_var_index_or_none(name) != INT_MIN) return bb;
                }
            }
        }
        // Check entry basic block for globals
        if (cfg->global_bb && cfg->global_bb->get_var_index_or_none(name) != INT_MIN) return cfg->global_bb;
    }
    return nullptr;
}

IRType BasicBlock::get_var_type(string name) {
    if (SymbolType.find(name) != SymbolType.end()) return SymbolType[name];
    if (cfg) {
        const auto& bbStack = cfg->getStackBBs();
        for (auto it = bbStack.rbegin(); it != bbStack.rend(); ++it) {
            BasicBlock* bb = *it;
            if (bb->SymbolType.find(name) != bb->SymbolType.end()) return bb->SymbolType[name];
        }

        string funcName = functionName.empty() ? cfg->getCurrentFunction() : functionName;
        if (!funcName.empty()) {
            auto* sig = cfg->get_function(funcName);
            if (sig) {
                for (auto bb : sig->bbs) {
                    if (bb->SymbolType.find(name) != bb->SymbolType.end()) return bb->SymbolType[name];
                }
            }
        }
        // Finally, check the entry basic block for globals
        if (cfg->global_bb) {
            if (cfg->global_bb->SymbolType.find(name) != cfg->global_bb->SymbolType.end()) {
                return cfg->global_bb->SymbolType[name];
            }
            std::string mangled = name + "@" + cfg->global_bb->label;
            if (cfg->global_bb->SymbolType.find(mangled) != cfg->global_bb->SymbolType.end()) {
                return cfg->global_bb->SymbolType[mangled];
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
    SymbolType[name] = type;
    SymbolIndex[name] = cfg->getNextFreeSymbolIndex();
    cfg->setNextFreeSymbolIndex(cfg->getNextFreeSymbolIndex() - irtype_size(type));
}

// ============================================================
//  CFG
// ============================================================

CFG::CFG(TargetArch arch) {
    nextBBnumber = 0;
    nextFreeSymbolIndex = -8;
    current_bb = new BasicBlock(this, new_BB_name());
    entry_bb = current_bb;
    global_bb = current_bb;
    add_bb(current_bb);
    nextTempVarNumber = 0;

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
    if (!currentFunctionName.empty()) {
        FunctionSignature* sig = get_function(currentFunctionName);
        if (sig) sig->bbs.push_back(bb);
    }
}

void CFG::gen_asm(ostream& o) { asmGenerator->gen_asm(o); }

void CFG::gen_control_flow(ostream& o, BasicBlock* bb) { asmGenerator->gen_control_flow(o, bb); }

void CFG::dump_symbol_table(std::ostream& o) {
    o << "Symbol table (global BB):\n";
    BasicBlock* bb = global_bb;
    for (auto& p : bb->SymbolIndex) {
        o << "  " << p.first << " -> index=" << p.second << " type=" << irtype_name(bb->SymbolType[p.first]) << "\n";
    }
}

void CFG::dump_instructions(std::ostream& o) {
    for (auto bb : getBBs()) {
        o << "BB " << bb->label << "\n";
        for (auto instr : bb->instrs) {
            o << "  " << instr->to_string() << "\n";
        }
    }
}

void CFG::gen_asm_instr(ostream& o, IRInstr* instr) {
    if (g_emit_ir_as_asm_comments) {
        o << ";   " << instr->to_string() << "\n";
    }
    asmGenerator->gen_asm_instr(o, instr);
}

void CFG::gen_asm_prologue(ostream& o) { asmGenerator->gen_prologue(o); }
void CFG::gen_asm_epilogue(ostream& o) { asmGenerator->gen_epilogue(o); }

string CFG::new_BB_name() { return "BB" + to_string(nextBBnumber++); }

string CFG::create_new_tempvar(IRType t) {
    string name = "!tmp" + to_string(nextTempVarNumber++);
    entry_bb->add_var_to_symbol_table(name, t);
    return name;
}

int CFG::calculateRequiredStackSpace(const string& funcName) {
    if (!funcName.empty()) {
        FunctionSignature* sig = get_function(funcName);
        if (sig) {
            if (sig->cachedStackSpace != -1) return sig->cachedStackSpace;
            int minIndex = 0;
            for (auto* bb : sig->bbs) {
                for (const auto& entry : bb->SymbolIndex) {
                    if (entry.second < minIndex) minIndex = entry.second;
                }
            }
            int usedSpace = -minIndex;
            int aligned = usedSpace;
            if (aligned % 16 != 0) aligned = ((aligned / 16) + 1) * 16;
            if (aligned < 16) aligned = 16;
            sig->cachedStackSpace = aligned;
            return aligned;
        }
    }
    int usedSpace = -nextFreeSymbolIndex;
    int aligned = usedSpace;
    if (aligned % 16 != 0) aligned = ((aligned / 16) + 1) * 16;
    if (aligned < 16) aligned = 16;
    return aligned;
}

BasicBlock* CFG::findBBByVariable(const string& var) {
    for (auto bb : getStackBBs())
        if (bb->get_var_index_or_none(var) != INT_MIN) return bb;
    string funcName = (current_bb && !current_bb->functionName.empty()) ? current_bb->functionName : currentFunctionName;
    if (funcName.empty()) funcName = currentFunctionName;
    if (!funcName.empty()) {
        FunctionSignature* sig = get_function(funcName);
        if (sig) {
            for (auto bb : sig->bbs)
                if (bb->get_var_index_or_none(var) != INT_MIN) return bb;
        }
    }
    return nullptr;
}

IRType CFG::get_array_element_type(const string& name) const {
    if (entry_bb && entry_bb->arrayElementType.find(name) != entry_bb->arrayElementType.end()) return entry_bb->arrayElementType.at(name);
    return IRType::INT32;
}

bool CFG::has_array_element_type(const string& name) const {
    if (!entry_bb) return false;
    return entry_bb->arrayElementType.find(name) != entry_bb->arrayElementType.end();
}

void CFG::set_emit_ir_comments(bool enabled) { g_emit_ir_as_asm_comments = enabled; }
bool CFG::get_emit_ir_comments() const { return g_emit_ir_as_asm_comments; }

void CFG::add_function(string name, IRType returnType, vector<IRType> paramTypes, vector<string> paramNames) {
    FunctionSignature sig;
    sig.name = name;
    sig.label = name;
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

std::vector<std::string> CFG::get_global_symbols() const { return globalSymbols; }

BasicBlock* CFG::create_function_entry(string name, IRType returnType, vector<IRType> paramTypes, vector<string> paramNames) {
    currentFunctionName = name;
    add_function(name, returnType, paramTypes, paramNames);
    FunctionSignature* sig = get_function(name);

    BasicBlock* entryBB = new BasicBlock(this, name);
    entryBB->reset_symbol_index();

    int paramOffset = 16;
    for (size_t i = 0; i < paramNames.size(); i++) {
        entryBB->add_param_to_symbol_table(paramNames[i], paramTypes[i], paramOffset);
        paramOffset += 8;
    }

    current_bb = entryBB;
    entry_bb = current_bb;

    // Add to data structures
    if (sig) {
        sig->bbs.push_back(entryBB);
        sig->entryBB = entryBB;
    }
    bbs.push_back(entryBB);

    return entryBB;
}
