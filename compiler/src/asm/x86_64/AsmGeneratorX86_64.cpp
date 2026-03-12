#include "AsmGeneratorX86_64.h"
#include "../../IR.h"
#include <iostream>
#include <stdexcept>

using namespace std;

AsmGeneratorX86_64::AsmGeneratorX86_64(CFG* cfg) : AsmGenerator(cfg) {}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

string AsmGeneratorX86_64::reg_to_asm(Reg r) {
    switch (r) {
        // W0 → rax family
        case Reg::W0_64:  return "%rax"; case Reg::W0_32: return "%eax";
        case Reg::W0_16:  return "%ax";  case Reg::W0_8:  return "%al";
        // W1 → rcx family
        case Reg::W1_64:  return "%rcx"; case Reg::W1_32: return "%ecx";
        case Reg::W1_16:  return "%cx";  case Reg::W1_8:  return "%cl";
        // W2 → rdx family
        case Reg::W2_64:  return "%rdx"; case Reg::W2_32: return "%edx";
        case Reg::W2_16:  return "%dx";  case Reg::W2_8:  return "%dl";
        // W3 → rbx family
        case Reg::W3_64:  return "%rbx"; case Reg::W3_32: return "%ebx";
        case Reg::W3_16:  return "%bx";  case Reg::W3_8:  return "%bl";
        // ARG0 → rdi family
        case Reg::ARG0_64: return "%rdi"; case Reg::ARG0_32: return "%edi";
        case Reg::ARG0_16: return "%di";  case Reg::ARG0_8:  return "%dil";
        // ARG1 → rsi family
        case Reg::ARG1_64: return "%rsi"; case Reg::ARG1_32: return "%esi";
        case Reg::ARG1_16: return "%si";  case Reg::ARG1_8:  return "%sil";
        // ARG2 → rdx family  (same physical as W2 on x86-64)
        case Reg::ARG2_64: return "%rdx"; case Reg::ARG2_32: return "%edx";
        case Reg::ARG2_16: return "%dx";  case Reg::ARG2_8:  return "%dl";
        // ARG3 → rcx family  (same physical as W1 on x86-64)
        case Reg::ARG3_64: return "%rcx"; case Reg::ARG3_32: return "%ecx";
        case Reg::ARG3_16: return "%cx";  case Reg::ARG3_8:  return "%cl";
        // ARG4 → r8 family
        case Reg::ARG4_64: return "%r8";  case Reg::ARG4_32: return "%r8d";
        case Reg::ARG4_16: return "%r8w"; case Reg::ARG4_8:  return "%r8b";
        // ARG5 → r9 family
        case Reg::ARG5_64: return "%r9";  case Reg::ARG5_32: return "%r9d";
        case Reg::ARG5_16: return "%r9w"; case Reg::ARG5_8:  return "%r9b";
        // RET → rax family  (same physical as W0 on x86-64)
        case Reg::RET_64:  return "%rax"; case Reg::RET_32: return "%eax";
        case Reg::RET_16:  return "%ax";  case Reg::RET_8:  return "%al";
    }
    throw std::invalid_argument("reg_to_asm: unknown Reg");
}

string AsmGeneratorX86_64::var_to_asm(const string& varName) {
    int index = cfg->current_bb->get_var_index(varName);
    return to_string(index) + "(%rbp)";
}

// ---------------------------------------------------------------------------
// gen_asm / gen_asm_bb / gen_asm_instr
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::gen_asm(ostream& o) {
    gen_prologue(o);
    bool isFirstBB = true;
    for (auto bb : cfg->getBBs()) {
        gen_asm_bb(o, bb, isFirstBB);
        isFirstBB = false;
    }
}

void AsmGeneratorX86_64::gen_asm_bb(ostream& o, BasicBlock* bb, bool isFirstBB) {
    if (!isFirstBB)
        o << bb->label << ":\n";
    for (auto instr : bb->instrs)
        gen_asm_instr(o, instr);
    gen_control_flow(o, bb);
}

void AsmGeneratorX86_64::gen_asm_instr(ostream& o, IRInstr* instr) {
    instr->accept(*this, o);
}

// ---------------------------------------------------------------------------
// Visitor implementations  (INT32 only for now)
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::visit(ostream& o, LdConstInstr& instr) {
    o << "    movl $" << instr.val.as_i32()
      << ", " << reg_to_asm(instr.dest.reg) << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, CopyRegInstr& instr) {
    string src  = reg_to_asm(instr.src.reg);
    string dest = reg_to_asm(instr.dest.reg);
    if (src != dest)
        o << "    movl " << src << ", " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, StoreStackInstr& instr) {
    o << "    movl " << reg_to_asm(instr.src.reg)
      << ", "       << var_to_asm(instr.dest.name) << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, LoadStackInstr& instr) {
    o << "    movl " << var_to_asm(instr.src.name)
      << ", "       << reg_to_asm(instr.dest.reg) << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, AddInstr& instr) {
    // dest = lhs + rhs   (dest may alias lhs — both map to a reg)
    string lhs  = reg_to_asm(instr.lhs.reg);
    string rhs  = reg_to_asm(instr.rhs.reg);
    string dest = reg_to_asm(instr.dest.reg);
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    addl " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, SubInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs.reg);
    string rhs  = reg_to_asm(instr.rhs.reg);
    string dest = reg_to_asm(instr.dest.reg);
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    subl " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, MulInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs.reg);
    string rhs  = reg_to_asm(instr.rhs.reg);
    string dest = reg_to_asm(instr.dest.reg);
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    imull " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, DivInstr& instr) {
    // idiv: dividend in %eax:%edx, divisor in a reg
    string lhs  = reg_to_asm(instr.lhs.reg);
    string rhs  = reg_to_asm(instr.rhs.reg);
    string dest = reg_to_asm(instr.dest.reg);
    if (lhs != "%eax")
        o << "    movl " << lhs << ", %eax\n";
    o << "    cltd\n";                       // sign-extend %eax into %edx:%eax
    o << "    idivl " << rhs << "\n";        // quotient → %eax
    if (dest != "%eax")
        o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, BitNotInstr& instr) {
    string src  = reg_to_asm(instr.src.reg);
    string dest = reg_to_asm(instr.dest.reg);
    if (dest != src)
        o << "    movl " << src << ", " << dest << "\n";
    o << "    notl " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, BitAndInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs.reg);
    string rhs  = reg_to_asm(instr.rhs.reg);
    string dest = reg_to_asm(instr.dest.reg);
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    andl " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, BitOrInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs.reg);
    string rhs  = reg_to_asm(instr.rhs.reg);
    string dest = reg_to_asm(instr.dest.reg);
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    orl " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, BitXorInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs.reg);
    string rhs  = reg_to_asm(instr.rhs.reg);
    string dest = reg_to_asm(instr.dest.reg);
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    xorl " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, CmpEqInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs.reg);
    string rhs  = reg_to_asm(instr.rhs.reg);
    string dest = reg_to_asm(instr.dest.reg);
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl " << rhs << ", %eax\n";
    o << "    sete %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, CmpLtInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs.reg);
    string rhs  = reg_to_asm(instr.rhs.reg);
    string dest = reg_to_asm(instr.dest.reg);
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl " << rhs << ", %eax\n";
    o << "    setl %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, CmpLeInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs.reg);
    string rhs  = reg_to_asm(instr.rhs.reg);
    string dest = reg_to_asm(instr.dest.reg);
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl " << rhs << ", %eax\n";
    o << "    setle %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, CallInstr& instr) {
    // x86_64 System V ABI: args in rdi, rsi, rdx, rcx, r8, r9 (64-bit)
    static const string argRegs64[] = {"%rdi","%rsi","%rdx","%rcx","%r8","%r9"};
    int numArgs = (int)instr.args.size();
    for (int i = 0; i < numArgs && i < 6; i++) {
        string r32 = reg_to_asm(instr.args[i].reg);
        o << "    movslq " << r32 << ", " << argRegs64[i] << "\n";
    }
    o << "    call " << instr.funcLabel << "\n";
    // result in %eax; move to dest if different
    string dest = reg_to_asm(instr.dest.reg);
    if (dest != "%eax")
        o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, RetInstr& instr) {
    // Return value must already be in Reg::RET (%eax) — just emit ret
    // (callers are responsible for loading the right value before RetInstr)
    o << "    ret\n";
}

// ---------------------------------------------------------------------------
// Prologue / Epilogue / Control flow
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::gen_prologue(ostream& o) {
    int stackSpace = cfg->calculateRequiredStackSpace();
    string funcName = cfg->getBBs().empty() ? "main" : cfg->getBBs()[0]->label;
    o << ".globl " << funcName << "\n";
    o << funcName << ":\n";
    o << "    pushq %rbp\n";
    o << "    movq %rsp, %rbp\n";
    o << "    subq $" << stackSpace << ", %rsp\n";
}

void AsmGeneratorX86_64::gen_epilogue(ostream& o) {
    o << "    leave\n";
    o << "    ret\n";
}

void AsmGeneratorX86_64::gen_control_flow(ostream& o, BasicBlock* bb) {
    if (bb->exit_true == nullptr) {
        gen_epilogue(o);
    } else if (bb->exit_false == nullptr) {
        o << "    jmp " << bb->exit_true->label << "\n";
    } else {
        o << "    movl " << var_to_asm(bb->test_var_name) << ", %eax\n";
        o << "    cmpl $0, %eax\n";
        o << "    je "  << bb->exit_false->label << "\n";
        o << "    jmp " << bb->exit_true->label  << "\n";
    }
}
