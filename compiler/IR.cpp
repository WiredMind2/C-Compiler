#include "IR.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

using namespace std;

// IRInstr implementation
IRInstr::IRInstr(BasicBlock* bb_, Operation op, Type t, vector<string> params)
    : bb(bb_), op(op), t(t), params(params) {}

void IRInstr::gen_asm(ostream &o) {
    if (op == ldconst) {
        o << "    movl $" << params[1] << ", " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == copy) {
        if (params[0] != params[1]) {
            string src_asm = bb->cfg->IR_reg_to_asm(params[1]);
            string dest_asm = bb->cfg->IR_reg_to_asm(params[0]);
            if (dest_asm != src_asm) {
                o << "    movl " << src_asm << ", %eax\n";
                o << "    movl %eax, " << dest_asm << "\n";
            }
        }
    } else if (op == add) {
        o << "    movl " << bb->cfg->IR_reg_to_asm(params[1]) << ", %eax\n";
        o << "    addl " << bb->cfg->IR_reg_to_asm(params[2]) << ", %eax\n";
        o << "    movl %eax, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == sub) {
        o << "    movl " << bb->cfg->IR_reg_to_asm(params[1]) << ", %eax\n";
        o << "    subl " << bb->cfg->IR_reg_to_asm(params[2]) << ", %eax\n";
        o << "    movl %eax, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == mul) {
        o << "    movl " << bb->cfg->IR_reg_to_asm(params[1]) << ", %eax\n";
        o << "    imull " << bb->cfg->IR_reg_to_asm(params[2]) << ", %eax\n";
        o << "    movl %eax, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == div) {
        o << "    movl " << bb->cfg->IR_reg_to_asm(params[1]) << ", %eax\n";
        o << "    cltd\n";
        o << "    idivl " << bb->cfg->IR_reg_to_asm(params[2]) << "\n";
        o << "    movl %eax, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    }
}

// BasicBlock implementation
BasicBlock::BasicBlock(CFG* cfg, string entry_label) : cfg(cfg), label(entry_label) {
    exit_true = nullptr;
    exit_false = nullptr;
}

void BasicBlock::gen_asm(ostream &o) {
    o << label << ":\n";
    for (auto instr : instrs) {
        instr->gen_asm(o);
    }
    if (exit_true == nullptr) {
        cfg->gen_asm_epilogue(o);
    } else if (exit_false == nullptr) {
        o << "    jmp " << exit_true->label << "\n";
    } else {
        o << "    movl " << cfg->IR_reg_to_asm(test_var_name) << ", %eax\n";
        o << "    cmpl $0, %eax\n";
        o << "    je " << exit_false->label << "\n";
        o << "    jmp " << exit_true->label << "\n";
    }
}

void BasicBlock::add_IRInstr(IRInstr::Operation op, Type t, vector<string> params) {
    instrs.push_back(new IRInstr(this, op, t, params));
}

// CFG implementation
CFG::CFG() {
    nextFreeSymbolIndex = -4;
    nextBBnumber = 0;
    current_bb = new BasicBlock(this, new_BB_name());
    add_bb(current_bb);
}

void CFG::add_bb(BasicBlock* bb) {
    bbs.push_back(bb);
}

void CFG::gen_asm(ostream& o) {
    gen_asm_prologue(o);
    for (auto bb : bbs) {
        bb->gen_asm(o);
    }
}

string CFG::IR_reg_to_asm(string reg) {
    if (reg == "!eax") return "%eax";
    int index = get_var_index(reg);
    return to_string(index) + "(%rbp)";
}

void CFG::gen_asm_prologue(ostream& o) {
    o << ".globl main\n";
    o << "main:\n";
    o << "    pushq %rbp\n";
    o << "    movq %rsp, %rbp\n";
    o << "    subq $1024, %rsp\n"; 
}

void CFG::gen_asm_epilogue(ostream& o) {
    o << "    leave\n";
    o << "    ret\n";
}

void CFG::add_to_symbol_table(string name, Type t) {
    SymbolType[name] = t;
    SymbolIndex[name] = nextFreeSymbolIndex;
    nextFreeSymbolIndex -= 4;
}

string CFG::create_new_tempvar(Type t) {
    string name = "!tmp" + to_string(-nextFreeSymbolIndex);
    add_to_symbol_table(name, t);
    return name;
}

int CFG::get_var_index(string name) {
    if (SymbolIndex.find(name) == SymbolIndex.end()) {
        cerr << "Error: Symbol " << name << " not found in symbol table." << endl;
        exit(1);
    }
    return SymbolIndex[name];
}

Type CFG::get_var_type(string name) {
    return SymbolType[name];
}

string CFG::new_BB_name() {
    return "BB" + to_string(nextBBnumber++);
}
