#include "AsmGeneratorX86_64.h"
#include "../../ir/IR.h"
#include <iostream>
#include <stdexcept>

using namespace std;

AsmGeneratorX86_64::AsmGeneratorX86_64(CFG* cfg) : AsmGenerator(cfg) {}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

string AsmGeneratorX86_64::reg_to_asm(const RegParam& p) {
    // Returns the register name for the given logical register and IRType.
    // x86-64 physical mappings:
    //   W0/RET  → rax/eax/ax/al
    //   W1      → rcx/ecx/cx/cl
    //   W2      → rdx/edx/dx/dl
    //   W3      → rbx/ebx/bx/bl
    //   ARG0    → rdi/edi/di/dil
    //   ARG1    → rsi/esi/si/sil
    //   ARG2    → rdx/edx/dx/dl
    //   ARG3    → rcx/ecx/cx/cl
    //   ARG4    → r8/r8d/r8w/r8b
    //   ARG5    → r9/r9d/r9w/r9b
    bool is64 = (p.type == IRType::INT64 || p.type == IRType::FLOAT64);
    switch (p.reg) {
        case Reg::W0:   case Reg::RET:
            return is64 ? "%rax" : "%eax";
        case Reg::W1:   case Reg::ARG3:
            return is64 ? "%rcx" : "%ecx";
        case Reg::W2:   case Reg::ARG2:
            return is64 ? "%rdx" : "%edx";
        case Reg::W3:
            return is64 ? "%rbx" : "%ebx";
        case Reg::ARG0:
            return is64 ? "%rdi" : "%edi";
        case Reg::ARG1:
            return is64 ? "%rsi" : "%esi";
        case Reg::ARG4:
            return is64 ? "%r8"  : "%r8d";
        case Reg::ARG5:
            return is64 ? "%r9"  : "%r9d";
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
    o << "    movl $" << instr.val.raw_int() << ", " << reg_to_asm(instr.dest) << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, CopyRegInstr& instr) {
    string src  = reg_to_asm(instr.src);
    string dest = reg_to_asm(instr.dest);
    if (src != dest)
        o << "    movl " << src << ", " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, StoreStackInstr& instr) {
    o << "    movl " << reg_to_asm(instr.src)
      << ", "       << var_to_asm(instr.dest.name) << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, LoadStackInstr& instr) {
    o << "    movl " << var_to_asm(instr.src.name)
      << ", "       << reg_to_asm(instr.dest) << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, AddInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    addl " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, SubInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    subl " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, MulInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    imull " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, DivInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (lhs != "%eax")
        o << "    movl " << lhs << ", %eax\n";
    o << "    cltd\n";
    o << "    idivl " << rhs << "\n";
    if (dest != "%eax")
        o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, BitNotInstr& instr) {
    string src  = reg_to_asm(instr.src);
    string dest = reg_to_asm(instr.dest);
    if (dest != src)
        o << "    movl " << src << ", " << dest << "\n";
    o << "    notl " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, BitAndInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    andl " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, BitOrInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    orl " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, BitXorInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    xorl " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, CmpEqInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl " << rhs << ", %eax\n";
    o << "    sete %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, CmpLtInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl " << rhs << ", %eax\n";
    o << "    setl %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, CmpLeInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl " << rhs << ", %eax\n";
    o << "    setle %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, CallInstr& instr) {
    static const string argRegs64[] = {"%rdi","%rsi","%rdx","%rcx","%r8","%r9"};
    int numArgs = (int)instr.args.size();
    for (int i = 0; i < numArgs && i < 6; i++) {
        string r32 = reg_to_asm(instr.args[i]);
        o << "    movslq " << r32 << ", " << argRegs64[i] << "\n";
    }
    o << "    call " << instr.funcLabel << "\n";
    string dest = reg_to_asm(instr.dest);
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
