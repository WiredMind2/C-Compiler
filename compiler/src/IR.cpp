#include "IR.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>

#include "asm/arm64/AsmGeneratorARM64.h"
#include "asm/x86_64/AsmGeneratorX86_64.h"

using namespace std;

static int getTypeSize(Type t) {
    switch (t) {
        case INT:  return 4;
        case CHAR: return 1;
        case VOID: return 0;
        default:   return 4;
    }
}

// ============================================================
//  IRInstr — accept() bodies
// ============================================================

void IRInstr::gen_asm(ostream& o) {
    bb->cfg->asmGenerator->gen_asm_instr(o, this);
}

void LdConstInstr   ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CopyRegInstr   ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void StoreStackInstr::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void LoadStackInstr ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void AddInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void SubInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void MulInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void DivInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void BitNotInstr    ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void BitAndInstr    ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void BitOrInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void BitXorInstr    ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CmpEqInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CmpLtInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CmpLeInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CallInstr      ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void RetInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }

// ============================================================
//  BasicBlock
// ============================================================

BasicBlock::BasicBlock(CFG* cfg, string entry_label)
    : cfg(cfg), label(entry_label) {
    exit_true  = nullptr;
    exit_false = nullptr;
    nextFreeSymbolIndex = -4;
}

void BasicBlock::gen_asm(ostream& o) {
    o << label << ":\n";
    for (auto instr : instrs)
        instr->gen_asm(o);
    cfg->gen_control_flow(o, this);
}

void BasicBlock::add_IRInstr(IRInstr* instr) {
    instrs.push_back(instr);
}

// ============================================================
//  Symbol table
// ============================================================

void BasicBlock::add_var_to_symbol_table(string name, Type t) {
    if (cfg->findBBByVariable(name) != nullptr) {
        cerr << "Error: Variable " << name
             << " already defined in a former or current scope." << endl;
        exit(1);
    }
    SymbolType[name] = t;
    SymbolIndex[name] = nextFreeSymbolIndex;
    nextFreeSymbolIndex -= getTypeSize(t);
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
    int usedSpace = -nextFreeSymbolIndex;
    int aligned   = usedSpace;
    if (aligned % 16 != 0)
        aligned = ((aligned / 16) + 1) * 16;
    if (aligned < 16) aligned = 16;
    return aligned;
}

void BasicBlock::allocateVariable(string name, Type type) {
    SymbolType[name]  = type;
    SymbolIndex[name] = nextFreeSymbolIndex;
    nextFreeSymbolIndex -= getTypeSize(type);
}

// ============================================================
//  CFG
// ============================================================

CFG::CFG(TargetArch arch) {
    nextBBnumber = 0;
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

void CFG::add_bb(BasicBlock* bb) { bbs.push_back(bb); }

void CFG::gen_asm(ostream& o) { asmGenerator->gen_asm(o); }

void CFG::gen_control_flow(ostream& o, BasicBlock* bb) {
    asmGenerator->gen_control_flow(o, bb);
}

void CFG::gen_asm_instr(ostream& o, IRInstr* instr) {
    cout << ";   " << instr->to_string() << endl;
    instr->accept(*asmGenerator, o);
}

void CFG::gen_asm_prologue(ostream& o) {
    cout << ";   Prologue:" << endl;
    asmGenerator->gen_prologue(o);
}

void CFG::gen_asm_epilogue(ostream& o) {
    cout << ";   Epilogue:" << endl;
    asmGenerator->gen_epilogue(o);
}

string CFG::new_BB_name() {
    return "BB" + to_string(nextBBnumber++);
}

int CFG::calculateRequiredStackSpace() {
    int space = 0;
    for (BasicBlock* bb : getBBs())
        space += bb->calculateRequiredStackSpace();
    return space;
}

BasicBlock* CFG::findBBByVariable(const string& var) {
    for (auto bb : getStackBBs())
        if (bb->get_var_index_or_none(var) != INT_MIN) return bb;
    for (auto bb : getBBs())
        if (bb->get_var_index_or_none(var) != INT_MIN) return bb;
    return nullptr;
}

void CFG::add_function(string name, Type returnType,
                       vector<Type> paramTypes, vector<string> paramNames) {
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

BasicBlock* CFG::create_function_entry(string name, Type returnType,
                                       vector<Type> paramTypes, vector<string> paramNames) {
    add_function(name, returnType, paramTypes, paramNames);
    bbs.clear();

    BasicBlock* entryBB = new BasicBlock(this, name);
    entryBB->reset_symbol_index();

    int paramOffset = 16;
    for (size_t i = 0; i < paramNames.size(); i++) {
        entryBB->add_param_to_symbol_table(paramNames[i], paramTypes[i], paramOffset);
        paramOffset += 8;
    }

    current_bb = entryBB;
    add_bb(entryBB);
    return entryBB;
}
