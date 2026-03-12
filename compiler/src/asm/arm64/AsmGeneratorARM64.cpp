#include "AsmGeneratorARM64.h"
#include "../../ir/IR.h"
#include "../../ir/IRInstr.h"
#include <iostream>
#include <stdexcept>

using namespace std;

AsmGeneratorARM64::AsmGeneratorARM64(CFG* cfg) : AsmGenerator(cfg) {}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

string AsmGeneratorARM64::reg_to_asm(const RegParam& p) {
    // ARM64 physical mappings:
    //   W0/RET/ARG0 → x0/w0
    //   ARG1        → x1/w1
    //   ARG2        → x2/w2
    //   ARG3        → x3/w3
    //   ARG4        → x4/w4
    //   ARG5        → x5/w5
    //   W1          → x9/w9
    //   W2          → x10/w10
    //   W3          → x11/w11
    bool is64 = (p.type == IRType::INT64 || p.type == IRType::FLOAT64);
    const char* prefix = is64 ? "x" : "w";
    switch (p.reg) {
        case Reg::W0:   case Reg::RET:  case Reg::ARG0: return string(prefix) + "0";
        case Reg::ARG1:                                  return string(prefix) + "1";
        case Reg::ARG2:                                  return string(prefix) + "2";
        case Reg::ARG3:                                  return string(prefix) + "3";
        case Reg::ARG4:                                  return string(prefix) + "4";
        case Reg::ARG5:                                  return string(prefix) + "5";
        case Reg::W1:                                    return string(prefix) + "9";
        case Reg::W2:                                    return string(prefix) + "10";
        case Reg::W3:                                    return string(prefix) + "11";
    }
    throw std::invalid_argument("reg_to_asm: unknown Reg");
}

string AsmGeneratorARM64::var_to_asm(const string& varName) {
    int index = cfg->current_bb->get_var_index(varName);
    return "[fp, #" + to_string(index) + "]";
}

// ---------------------------------------------------------------------------
// gen_asm / gen_asm_bb / gen_asm_instr
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::gen_asm(ostream& o) {
    cfg->gen_asm_prologue(o);
    bool isFirstBB = true;
    for (auto bb : cfg->getBBs()) {
        gen_asm_bb(o, bb, isFirstBB);
        isFirstBB = false;
    }
}

void AsmGeneratorARM64::gen_asm_bb(ostream& o, BasicBlock* bb, bool isFirstBB) {
    if (!isFirstBB)
        o << bb->label << ":\n";
    for (auto instr : bb->instrs)
        cfg->gen_asm_instr(o, instr);
    gen_control_flow(o, bb);
}

void AsmGeneratorARM64::gen_asm_instr(ostream& o, IRInstr* instr) {
    instr->accept(*this, o);
}

// ---------------------------------------------------------------------------
// Visitor implementations  (INT32 only)
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::visit(ostream& o, LdConstInstr& instr) {
    string dest = reg_to_asm(instr.dest);
    int64_t val = instr.val.raw_int() & 0xFFFFFFFF;
    if (val < 65536) {
        o << "    mov " << dest << ", #" << val << "\n";
    } else {
        o << "    movz " << dest << ", #" << (val & 0xFFFF) << "\n";
        if (val >> 16)
            o << "    movk " << dest << ", #" << (val >> 16) << ", lsl #16\n";
    }
}

void AsmGeneratorARM64::visit(ostream& o, CopyRegInstr& instr) {
    string src  = reg_to_asm(instr.src);
    string dest = reg_to_asm(instr.dest);
    if (src != dest)
        o << "    mov " << dest << ", " << src << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, StoreStackInstr& instr) {
    o << "    str " << reg_to_asm(instr.src)
      << ", "      << var_to_asm(instr.dest.name) << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, LoadStackInstr& instr) {
    o << "    ldr " << reg_to_asm(instr.dest)
      << ", "      << var_to_asm(instr.src.name) << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, AddInstr& instr) {
    o << "    add " << reg_to_asm(instr.dest) << ", "
                   << reg_to_asm(instr.lhs)  << ", "
                   << reg_to_asm(instr.rhs)  << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, SubInstr& instr) {
    o << "    sub " << reg_to_asm(instr.dest) << ", "
                   << reg_to_asm(instr.lhs)  << ", "
                   << reg_to_asm(instr.rhs)  << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, MulInstr& instr) {
    o << "    mul " << reg_to_asm(instr.dest) << ", "
                   << reg_to_asm(instr.lhs)  << ", "
                   << reg_to_asm(instr.rhs)  << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, DivInstr& instr) {
    o << "    sdiv " << reg_to_asm(instr.dest) << ", "
                    << reg_to_asm(instr.lhs)   << ", "
                    << reg_to_asm(instr.rhs)   << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, BitNotInstr& instr) {
    string src  = reg_to_asm(instr.src);
    string dest = reg_to_asm(instr.dest);
    o << "    eor " << dest << ", " << src << ", #0xFFFFFFFF\n";
}

void AsmGeneratorARM64::visit(ostream& o, BitAndInstr& instr) {
    o << "    and " << reg_to_asm(instr.dest) << ", "
                   << reg_to_asm(instr.lhs)  << ", "
                   << reg_to_asm(instr.rhs)  << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, BitOrInstr& instr) {
    o << "    orr " << reg_to_asm(instr.dest) << ", "
                   << reg_to_asm(instr.lhs)  << ", "
                   << reg_to_asm(instr.rhs)  << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, BitXorInstr& instr) {
    o << "    eor " << reg_to_asm(instr.dest) << ", "
                   << reg_to_asm(instr.lhs)  << ", "
                   << reg_to_asm(instr.rhs)  << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, CmpEqInstr& instr) {
    string dest = reg_to_asm(instr.dest);
    o << "    cmp " << reg_to_asm(instr.lhs)
      << ", "      << reg_to_asm(instr.rhs) << "\n";
    o << "    cset " << dest << ", eq\n";
}

void AsmGeneratorARM64::visit(ostream& o, CmpLtInstr& instr) {
    string dest = reg_to_asm(instr.dest);
    o << "    cmp " << reg_to_asm(instr.lhs)
      << ", "      << reg_to_asm(instr.rhs) << "\n";
    o << "    cset " << dest << ", lt\n";
}

void AsmGeneratorARM64::visit(ostream& o, CmpLeInstr& instr) {
    string dest = reg_to_asm(instr.dest);
    o << "    cmp " << reg_to_asm(instr.lhs)
      << ", "      << reg_to_asm(instr.rhs) << "\n";
    o << "    cset " << dest << ", le\n";
}

void AsmGeneratorARM64::visit(ostream& o, CallInstr& instr) {
    int numArgs = (int)instr.args.size();
    for (int i = 0; i < numArgs && i < 6; i++) {
        string src = reg_to_asm(instr.args[i]);
        string dst = "w" + to_string(i);
        if (src != dst)
            o << "    mov " << dst << ", " << src << "\n";
    }
    o << "    bl " << instr.funcLabel << "\n";
    string dest = reg_to_asm(instr.dest);
    if (dest != "w0")
        o << "    mov " << dest << ", w0\n";
}

void AsmGeneratorARM64::visit(ostream& o, RetInstr& instr) {
    // Return value must already be in Reg::RET (w0)
    o << "    ret\n";
}

// ---------------------------------------------------------------------------
// Prologue / Epilogue / Control flow
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::gen_prologue(ostream& o) {
    int stackSpace = cfg->calculateRequiredStackSpace();
    string funcName = cfg->getBBs().empty() ? "_main" : "_" + cfg->getBBs()[0]->label;
    o << ".globl " << funcName << "\n";
    o << funcName << ":\n";
    o << "    stp fp, lr, [sp, #-16]!\n";
    o << "    mov fp, sp\n";
    o << "    sub sp, sp, #" << stackSpace << "\n";
}

void AsmGeneratorARM64::gen_epilogue(ostream& o) {
    o << "    mov sp, fp\n";
    o << "    ldp fp, lr, [sp], #16\n";
    o << "    ret\n";
}

void AsmGeneratorARM64::gen_control_flow(ostream& o, BasicBlock* bb) {
    if (bb->exit_true == nullptr) {
        gen_epilogue(o);
    } else if (bb->exit_false == nullptr) {
        o << "    b " << bb->exit_true->label << "\n";
    } else {
        o << "    ldr w0, " << var_to_asm(bb->test_var_name) << "\n";
        o << "    cmp w0, #0\n";
        o << "    b.eq " << bb->exit_false->label << "\n";
        o << "    b "    << bb->exit_true->label  << "\n";
    }
}
