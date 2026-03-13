#include "asm/arm64/AsmGeneratorARM64.h"
#include "asm/x86_64/AsmGeneratorX86_64.h"
#include "IR.h"
#include <iostream>
static int getTypeSize(Type t) { return 4; }
#include <algorithm>
#include <assert.h>

IRInstr::IRInstr(BasicBlock* bb_, Operation op, Type t, vector<string> params)
    : bb(bb_), op(op), t(t), params(params) {}

void IRInstr::gen_asm(ostream &o) {
    bb->cfg->asmGenerator->gen_asm_instr(o, this);
}

// BasicBlock implementation
BasicBlock::BasicBlock(CFG* cfg, string entry_label) : cfg(cfg), label(entry_label) {
     terminator = nullptr;
 }

void BasicBlock::gen_asm(ostream &o) {
    for (auto instr : instrs) {
        instr->gen_asm(o);
    }
    
    // Generate assembly for the terminator
    if (terminator) {
        switch (terminator->kind) {
            case TerminatorKind::FALLTHROUGH:
                // Implicit jump to next block, no assembly needed
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
            return nullptr;
    }
    return nullptr;
}

// CFG implementation
CFG::CFG(TargetArch arch) {
     nextBBnumber = 0;
     current_bb = nullptr;
     nextFreeSymbolIndex = 0;

     switch (arch) {
         case TargetArch::ARM64:
             asmGenerator = new AsmGeneratorARM64(this);
             break;
         case TargetArch::X86_64:
             asmGenerator = new AsmGeneratorX86_64(this);
             break;
     }
 }

int& CFG::getNextFreeSymbolIndex() {
     return nextFreeSymbolIndex;
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
    int size = getTypeSize(t);
    cfg->getNextFreeSymbolIndex() -= size;
    SymbolType[name] = t;
    SymbolIndex[name] = cfg->getNextFreeSymbolIndex();
}

string BasicBlock::create_new_tempvar(Type t) {
     int offset = cfg->getNextFreeSymbolIndex();
     string name = "!tmp" + to_string(-offset);
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
    return 0;
}

int CFG::calculateRequiredStackSpace() {
     int usedSpace = -nextFreeSymbolIndex;
     int alignedSpace = usedSpace;
     if (alignedSpace % 16 != 0) {
         alignedSpace = ((alignedSpace / 16) + 1) * 16;
     }
     if (alignedSpace < 16) {
         alignedSpace = 16;
     }
     return alignedSpace;
 }

bool CFG::validate() {
    for (auto bb : getBBs()) {
        if (!bb->hasTerminator()) {
            if (bb->instrs.empty()) {
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
    int size = getTypeSize(type);
    cfg->getNextFreeSymbolIndex() -= size;
    SymbolType[name] = type;
    SymbolIndex[name] = cfg->getNextFreeSymbolIndex();
}

void CFG::genOptimizedPrologue(ostream& o) {
    asmGenerator->gen_prologue(o);
}

BasicBlock* CFG::findBBByVariable(string var) {
    for (auto it = bbStack.rbegin(); it != bbStack.rend(); ++it) {
        if ((*it)->has_var(var)) return *it;
    }
    if (current_bb && current_bb->has_var(var)) return current_bb;
    for (auto bb : bbs) {
        if (bb->has_var(var)) return bb;
    }
    return nullptr;
}

void CFG::add_function(string name, Type returnType, vector<Type> paramTypes, vector<string> paramNames) {
    FunctionSignature sig;
    sig.name = name;
    sig.label = name;
    sig.returnType = returnType;
    sig.paramTypes = paramTypes;
    sig.paramNames = paramNames;
    sig.entryBB = getBBByName(name);
    functions.push_back(sig);
}

CFG::FunctionSignature* CFG::get_function(string name) {
    for (auto& sig : functions) {
        if (sig.name == name) return &sig;
    }
    return nullptr;
}

BasicBlock* CFG::create_function_declaration(string name, Type returnType, vector<Type> paramTypes, vector<string> paramNames) {
    add_function(name, returnType, paramTypes, paramNames);
    BasicBlock* entryBB = new BasicBlock(this, name);
    return entryBB;
}

BasicBlock* CFG::getBBByName(string name) {
    for (auto bb : bbs) {
        if (bb->label == name) return bb;
    }
    return nullptr;
}

void CFG::enter_function_definition(string name) {
    BasicBlock* bb = getBBByName(name);
    if (bb) current_bb = bb;
}

BasicBlock* CFG::getOrCreateFunctionEntryBB(string name, Type returnType, vector<Type> paramTypes, vector<string> paramNames) {
    BasicBlock* bb = getBBByName(name);
    if (!bb) {
        bb = new BasicBlock(this, name);
        add_bb(bb);
        add_function(name, returnType, paramTypes, paramNames);
    }
    return bb;
}
