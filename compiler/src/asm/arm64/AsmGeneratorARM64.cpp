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

string AsmGeneratorARM64::reg_to_asm(Reg r) {
    switch (r) {
        // W0 → x0/w0 family  (ARM64: x0=64, w0=32, no named 16/8 variants — use w0 with mask)
        case Reg::W0_64: return "x0";  case Reg::W0_32: return "w0";
        case Reg::W0_16: return "w0";  case Reg::W0_8:  return "w0";
        // W1 → x9/w9 family
        case Reg::W1_64: return "x9";  case Reg::W1_32: return "w9";
        case Reg::W1_16: return "w9";  case Reg::W1_8:  return "w9";
        // W2 → x10/w10 family
        case Reg::W2_64: return "x10"; case Reg::W2_32: return "w10";
        case Reg::W2_16: return "w10"; case Reg::W2_8:  return "w10";
        // W3 → x11/w11 family
        case Reg::W3_64: return "x11"; case Reg::W3_32: return "w11";
        case Reg::W3_16: return "w11"; case Reg::W3_8:  return "w11";
        // ARG0 → x0/w0  (same physical as W0 on ARM64)
        case Reg::ARG0_64: return "x0"; case Reg::ARG0_32: return "w0";
        case Reg::ARG0_16: return "w0"; case Reg::ARG0_8:  return "w0";
        // ARG1 → x1/w1
        case Reg::ARG1_64: return "x1"; case Reg::ARG1_32: return "w1";
        case Reg::ARG1_16: return "w1"; case Reg::ARG1_8:  return "w1";
        // ARG2 → x2/w2
        case Reg::ARG2_64: return "x2"; case Reg::ARG2_32: return "w2";
        case Reg::ARG2_16: return "w2"; case Reg::ARG2_8:  return "w2";
        // ARG3 → x3/w3
        case Reg::ARG3_64: return "x3"; case Reg::ARG3_32: return "w3";
        case Reg::ARG3_16: return "w3"; case Reg::ARG3_8:  return "w3";
        // ARG4 → x4/w4
        case Reg::ARG4_64: return "x4"; case Reg::ARG4_32: return "w4";
        case Reg::ARG4_16: return "w4"; case Reg::ARG4_8:  return "w4";
        // ARG5 → x5/w5
        case Reg::ARG5_64: return "x5"; case Reg::ARG5_32: return "w5";
        case Reg::ARG5_16: return "w5"; case Reg::ARG5_8:  return "w5";
        // RET → x0/w0  (same physical as W0 and ARG0 on ARM64)
        case Reg::RET_64: return "x0"; case Reg::RET_32: return "w0";
        case Reg::RET_16: return "w0"; case Reg::RET_8:  return "w0";
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
    string dest = reg_to_asm(instr.dest.reg);
    int64_t val = (int64_t)instr.val.as_i32() & 0xFFFFFFFF;
    if (val < 65536) {
        o << "    mov " << dest << ", #" << val << "\n";
    } else {
        o << "    movz " << dest << ", #" << (val & 0xFFFF) << "\n";
        if (val >> 16)
            o << "    movk " << dest << ", #" << (val >> 16) << ", lsl #16\n";
    }
}

void AsmGeneratorARM64::visit(ostream& o, CopyRegInstr& instr) {
    string src  = reg_to_asm(instr.src.reg);
    string dest = reg_to_asm(instr.dest.reg);
    if (src != dest)
        o << "    mov " << dest << ", " << src << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, StoreStackInstr& instr) {
    o << "    str " << reg_to_asm(instr.src.reg)
      << ", "      << var_to_asm(instr.dest.name) << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, LoadStackInstr& instr) {
    o << "    ldr " << reg_to_asm(instr.dest.reg)
      << ", "      << var_to_asm(instr.src.name) << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, AddInstr& instr) {
    o << "    add " << reg_to_asm(instr.dest.reg) << ", "
                   << reg_to_asm(instr.lhs.reg)  << ", "
                   << reg_to_asm(instr.rhs.reg)  << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, SubInstr& instr) {
    o << "    sub " << reg_to_asm(instr.dest.reg) << ", "
                   << reg_to_asm(instr.lhs.reg)  << ", "
                   << reg_to_asm(instr.rhs.reg)  << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, MulInstr& instr) {
    o << "    mul " << reg_to_asm(instr.dest.reg) << ", "
                   << reg_to_asm(instr.lhs.reg)  << ", "
                   << reg_to_asm(instr.rhs.reg)  << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, DivInstr& instr) {
    o << "    sdiv " << reg_to_asm(instr.dest.reg) << ", "
                    << reg_to_asm(instr.lhs.reg)   << ", "
                    << reg_to_asm(instr.rhs.reg)   << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, BitNotInstr& instr) {
    string src  = reg_to_asm(instr.src.reg);
    string dest = reg_to_asm(instr.dest.reg);
    o << "    eor " << dest << ", " << src << ", #0xFFFFFFFF\n";
}

void AsmGeneratorARM64::visit(ostream& o, BitAndInstr& instr) {
    o << "    and " << reg_to_asm(instr.dest.reg) << ", "
                   << reg_to_asm(instr.lhs.reg)  << ", "
                   << reg_to_asm(instr.rhs.reg)  << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, BitOrInstr& instr) {
    o << "    orr " << reg_to_asm(instr.dest.reg) << ", "
                   << reg_to_asm(instr.lhs.reg)  << ", "
                   << reg_to_asm(instr.rhs.reg)  << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, BitXorInstr& instr) {
    o << "    eor " << reg_to_asm(instr.dest.reg) << ", "
                   << reg_to_asm(instr.lhs.reg)  << ", "
                   << reg_to_asm(instr.rhs.reg)  << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, CmpEqInstr& instr) {
    string dest = reg_to_asm(instr.dest.reg);
    o << "    cmp " << reg_to_asm(instr.lhs.reg)
      << ", "      << reg_to_asm(instr.rhs.reg) << "\n";
    o << "    cset " << dest << ", eq\n";
}

void AsmGeneratorARM64::visit(ostream& o, CmpLtInstr& instr) {
    string dest = reg_to_asm(instr.dest.reg);
    o << "    cmp " << reg_to_asm(instr.lhs.reg)
      << ", "      << reg_to_asm(instr.rhs.reg) << "\n";
    o << "    cset " << dest << ", lt\n";
}

void AsmGeneratorARM64::visit(ostream& o, CmpLeInstr& instr) {
    string dest = reg_to_asm(instr.dest.reg);
    o << "    cmp " << reg_to_asm(instr.lhs.reg)
      << ", "      << reg_to_asm(instr.rhs.reg) << "\n";
    o << "    cset " << dest << ", le\n";
}

void AsmGeneratorARM64::visit(ostream& o, CallInstr& instr) {
    // ARM64 AAPCS: args in w0..w7, return in w0
    int numArgs = (int)instr.args.size();
    for (int i = 0; i < numArgs && i < 6; i++) {
        string src = reg_to_asm(instr.args[i].reg);
        string dst = "w" + to_string(i); // w0..w5
        if (src != dst)
            o << "    mov " << dst << ", " << src << "\n";
    }
    o << "    bl " << instr.funcLabel << "\n";
    // result in w0 (Reg::RET); move to dest if different
    string dest = reg_to_asm(instr.dest.reg);
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
