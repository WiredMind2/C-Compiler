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
    // For FLOAT64, map working registers to XMM registers
    if (p.type == IRType::FLOAT64) {
        switch (p.reg) {
            case Reg::W0:  case Reg::RET:  return "%xmm0";
            case Reg::W1:                  return "%xmm1";
            case Reg::W2:                  return "%xmm2";
            case Reg::W3:                  return "%xmm3";
            case Reg::ARG0:                return "%xmm0";
            case Reg::ARG1:                return "%xmm1";
            case Reg::ARG2:                return "%xmm2";
            case Reg::ARG3:                return "%xmm3";
            case Reg::ARG4:                return "%xmm4";
            case Reg::ARG5:                return "%xmm5";
        }
    }
    // x86-64 GPR mappings
    bool is64 = (p.type == IRType::INT64 || p.type == IRType::POINTER);
    switch (p.reg) {
        case Reg::W0:   case Reg::RET:
            return is64 ? "%rax" : "%eax";
        case Reg::W1:
            return is64 ? "%rcx" : "%ecx";
        case Reg::W2:
            return is64 ? "%rdx" : "%edx";
        case Reg::W3:
            return is64 ? "%rbx" : "%ebx";
        case Reg::ARG0:
            return is64 ? "%rdi" : "%edi";
        case Reg::ARG1:
            return is64 ? "%rsi" : "%esi";
        case Reg::ARG2:
            return is64 ? "%rdx" : "%edx";
        case Reg::ARG3:
            return is64 ? "%rcx" : "%ecx";
        case Reg::ARG4:
            return is64 ? "%r8"  : "%r8d";
        case Reg::ARG5:
            return is64 ? "%r9"  : "%r9d";
        case Reg::FRAME_PTR:
            return is64 ? "%rbp" : "%ebp";
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
    // Generate .globl for all functions
    for (auto bb : cfg->getBBs()) {
        o << ".globl " << bb->label << "\n";
    }

    // Generate assembly for all basic blocks
    bool isFirstBB = true;
    for (auto bb : cfg->getBBs()) {
        gen_asm_bb(o, bb, isFirstBB);
        isFirstBB = false;
    }
}

void AsmGeneratorX86_64::gen_asm_bb(ostream& o, BasicBlock* bb, bool isFirstBB) {
    // Set current_bb for this BB so var_to_asm can find variable indices
    cfg->current_bb = bb;

    auto* sig = cfg->get_function(bb->label);
    if (sig) {
        int stackSpace = cfg->calculateRequiredStackSpace();
        o << bb->label << ":\n";
        o << "    pushq %rbp\n";
        o << "    movq %rsp, %rbp\n";
        o << "    subq $" << stackSpace << ", %rsp\n";

        // Copy parameters from ABI registers to stack slots (System V ABI)
        static const string argRegs64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
        int numParams = (int)sig->paramNames.size();
        for (int i = 0; i < numParams && i < 6; i++) {
            int offset = bb->get_var_index(sig->paramNames[i]);
            o << "    movq " << argRegs64[i] << ", " << offset << "(%rbp)\n";
        }
    } else {
        o << bb->label << ":\n";
    }

    // Generate instructions
    for (auto instr : bb->instrs) {
        gen_asm_instr(o, instr);
    }

    gen_control_flow(o, bb);
}

void AsmGeneratorX86_64::gen_control_flow(ostream& o, BasicBlock* bb) {
    if (bb->exit_true == nullptr) {
        o << "    # Jump to return (inlined epilogue)\n";
        o << "    leave\n";
        o << "    ret\n";
    } else if (bb->exit_false == nullptr) {
        o << "    jmp " << bb->exit_true->label << "\n";
    } else {
        string testVar = bb->test_var_name;
        int offset = bb->get_var_index(testVar);
        o << "    movl " << offset << "(%rbp), %eax\n";
        o << "    cmpl $0, %eax\n";
        o << "    jne " << bb->exit_true->label << "\n";
        o << "    jmp " << bb->exit_false->label << "\n";
    }
}

void AsmGeneratorX86_64::gen_asm_instr(ostream& o, IRInstr* instr) {
    instr->accept(*this, o);
}

// ---------------------------------------------------------------------------
// Visitor implementations  (INT32 only for now)
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::visit(ostream& o, LdConstInstr& instr) {
    if (instr.type == IRType::INT32) {
        o << "    movl $" << instr.val.raw_int() << ", " << reg_to_asm(instr.dest) << "\n";
    } else if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        o << "    movq $" << instr.val.raw_int() << ", " << reg_to_asm(instr.dest) << "\n";
    } else if (instr.type == IRType::FLOAT64) {
        // Load 64-bit IEEE bit pattern via a scratch GPR, then transfer to XMM
        uint64_t bits = std::bit_cast<uint64_t>(instr.val.as_f64());
        string xmm = reg_to_asm(instr.dest); // e.g. %xmm0
        o << "    movabsq $" << bits << ", %rax\n";
        o << "    movq %rax, " << xmm << "\n";
    }
}

void AsmGeneratorX86_64::visit(ostream& o, CopyRegInstr& instr) {
    string src  = reg_to_asm(instr.src);
    string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::INT32) {
        if (src != dest) o << "    movl " << src << ", " << dest << "\n";
    } else if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        if (instr.src.type == IRType::INT32) {
            o << "    movslq " << src << ", " << dest << "\n";
        } else if (src != dest) {
            o << "    movq " << src << ", " << dest << "\n";
        }
    } else if (instr.type == IRType::FLOAT64) {
        if (src != dest) o << "    movsd " << src << ", " << dest << "\n";
    }
}

void AsmGeneratorX86_64::visit(ostream& o, StoreStackInstr& instr) {
    if (instr.type == IRType::FLOAT64) {
        o << "    movsd " << reg_to_asm(instr.src)
          << ", "        << var_to_asm(instr.dest.name) << "\n";
    } else if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        string src = reg_to_asm(instr.src);
        if (instr.src.type == IRType::INT32) {
            o << "    movslq " << src << ", %rax\n";
            o << "    movq %rax, " << var_to_asm(instr.dest.name) << "\n";
        } else {
            o << "    movq " << src << ", " << var_to_asm(instr.dest.name) << "\n";
        }
    } else {
        o << "    movl " << reg_to_asm(instr.src)
          << ", "       << var_to_asm(instr.dest.name) << "\n";
    }
}

void AsmGeneratorX86_64::visit(ostream& o, LoadStackInstr& instr) {
    if (instr.type == IRType::FLOAT64) {
        o << "    movsd " << var_to_asm(instr.src.name)
          << ", "        << reg_to_asm(instr.dest) << "\n";
    } else if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        if (instr.dest.type == IRType::INT32) {
            o << "    movl " << var_to_asm(instr.src.name)
              << ", "       << reg_to_asm(instr.dest) << "\n";
        } else {
            o << "    movq " << var_to_asm(instr.src.name)
              << ", "       << reg_to_asm(instr.dest) << "\n";
        }
    } else {
        o << "    movl " << var_to_asm(instr.src.name)
          << ", "       << reg_to_asm(instr.dest) << "\n";
    }
}

void AsmGeneratorX86_64::visit(ostream& o, AddressOfSymbolInstr& instr) {
    o << "    leaq " << var_to_asm(instr.src.name)
      << ", " << reg_to_asm(instr.dest) << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, LoadPointerInstr& instr) {
    if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        o << "    movq (" << reg_to_asm(instr.ptr) << "), " << reg_to_asm(instr.dest) << "\n";
    } else if (instr.type == IRType::FLOAT64) {
        o << "    movsd (" << reg_to_asm(instr.ptr) << "), " << reg_to_asm(instr.dest) << "\n";
    } else {
        o << "    movl (" << reg_to_asm(instr.ptr) << "), " << reg_to_asm(instr.dest) << "\n";
    }
}

void AsmGeneratorX86_64::visit(ostream& o, StorePointerInstr& instr) {
    if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        o << "    movq " << reg_to_asm(instr.src) << ", (" << reg_to_asm(instr.ptr) << ")\n";
    } else if (instr.type == IRType::FLOAT64) {
        o << "    movsd " << reg_to_asm(instr.src) << ", (" << reg_to_asm(instr.ptr) << ")\n";
    } else {
        o << "    movl " << reg_to_asm(instr.src) << ", (" << reg_to_asm(instr.ptr) << ")\n";
    }
}

void AsmGeneratorX86_64::visit(ostream& o, AddInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::INT64) {
        if (dest != lhs)
            o << "    movq " << lhs << ", " << dest << "\n";
        o << "    addq " << rhs << ", " << dest << "\n";
    } else if (instr.type == IRType::POINTER) {
        // Pointer addition: assume both operands are already in pointer-sized
        // registers (index should be scaled earlier). Just perform a 64-bit add.
        if (dest != lhs)
            o << "    movq " << lhs << ", " << dest << "\n";
        o << "    addq " << rhs << ", " << dest << "\n";
    } else if (instr.type == IRType::INT32) {
        if (dest != lhs)
            o << "    movl " << lhs << ", " << dest << "\n";
        o << "    addl " << rhs << ", " << dest << "\n";
    }else if (instr.type == IRType::FLOAT64) {
        o << "    movsd " << lhs << ", %xmm0\n";
        o << "    addsd " << rhs << ", %xmm0\n";
        o << "    movsd %xmm0, " << dest << "\n";
    }
}

void AsmGeneratorX86_64::visit(ostream& o, SubInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        if (dest != lhs)
            o << "    movq " << lhs << ", " << dest << "\n";
        o << "    subq " << rhs << ", " << dest << "\n";
    } else if (instr.type == IRType::INT32) {
        if (dest != lhs)
            o << "    movl " << lhs << ", " << dest << "\n";
        o << "    subl " << rhs << ", " << dest << "\n";
    }else if (instr.type == IRType::FLOAT64) {
        o << "    movsd " << lhs << ", %xmm0\n";
        o << "    subsd " << rhs << ", %xmm0\n";
        o << "    movsd %xmm0, " << dest << "\n";
    }
}

void AsmGeneratorX86_64::visit(ostream& o, MulInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::INT32) {
        o << "    movl " << lhs << ", %eax\n";
        o << "    imull " << rhs << ", %eax\n";
        o << "    movl %eax, " << dest << "\n";
    } else if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        o << "    movq " << lhs << ", %rax\n";
        o << "    imulq " << rhs << ", %rax\n";
        o << "    movq %rax, " << dest << "\n";
    } else if (instr.type == IRType::FLOAT64) {
        o << "    movsd " << lhs << ", %xmm0\n";
        o << "    mulsd " << rhs << ", %xmm0\n";
        o << "    movsd %xmm0, " << dest << "\n";
    }
}

void AsmGeneratorX86_64::visit(ostream& o, DivInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::INT32) {
        if (lhs != "%eax")
            o << "    movl " << lhs << ", %eax\n";
        o << "    cltd\n";
        o << "    idivl " << rhs << "\n";
        if (dest != "%eax")
            o << "    movl %eax, " << dest << "\n";
    }else if (instr.type == IRType::FLOAT64){
        o << "    movsd " << lhs << ", %xmm0\n";
        o << "    divsd " << rhs << ", %xmm0\n";
        o << "    movsd %xmm0, " << dest << "\n";
    }
}

void AsmGeneratorX86_64::visit(ostream& o, ModInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    // idivl: dividend in %eax, sign-extended into %edx:%eax; remainder in %edx
    if (lhs != "%eax")
        o << "    movl " << lhs << ", %eax\n";
    o << "    cltd\n";
    o << "    idivl " << rhs << "\n";
    // remainder is in %edx
    if (dest != "%edx")
        o << "    movl %edx, " << dest << "\n";
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
    if (instr.type == IRType::INT32) {
        if (rhs != "%ecx") o << "    movl " << rhs << ", %ecx\n";
        if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
        o << "    cmpl %ecx, %eax\n";
        o << "    sete %al\n";
        o << "    movzbl %al, %eax\n";
        if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
    }else if (instr.type == IRType::FLOAT64) {
        o << "    movsd " << lhs << ", %xmm0\n";
        o << "    ucomisd " << rhs << ", %xmm0\n";
        o << "    sete %al\n";
        o << "    setnp %cl\n";
        o << "    andb %cl, %al\n";
        o << "    movzbl %al, %eax\n";
        o << "    movl %eax, " << dest << "\n";
    }
}

void AsmGeneratorX86_64::visit(ostream& o, CmpLtInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (rhs != "%ecx") o << "    movl " << rhs << ", %ecx\n";
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl %ecx, %eax\n";
    o << "    setl %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, CmpLeInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (rhs != "%ecx") o << "    movl " << rhs << ", %ecx\n";
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl %ecx, %eax\n";
    o << "    setle %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, CmpGtInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (rhs != "%ecx") o << "    movl " << rhs << ", %ecx\n";
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl %ecx, %eax\n";
    o << "    setg %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, CmpGeInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (rhs != "%ecx") o << "    movl " << rhs << ", %ecx\n";
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl %ecx, %eax\n";
    o << "    setge %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, LogicalAndInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    static int labelCount = 0;
    int thisLabel = labelCount++;
    o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl $0, %eax\n";
    o << "    je .Lend_and_" << thisLabel << "\n";
    o << "    movl " << rhs << ", %eax\n";
    o << "    cmpl $0, %eax\n";
    o << "    je .Lend_and_" << thisLabel << "\n";
    o << "    movl $1, %eax\n";
    o << "    jmp .Ldone_and_" << thisLabel << "\n";
    o << ".Lend_and_" << thisLabel << ":\n";
    o << "    movl $0, %eax\n";
    o << ".Ldone_and_" << thisLabel << ":\n";
    o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, LogicalOrInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    static int labelCount = 0;
    int thisLabel = labelCount++;
    o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl $0, %eax\n";
    o << "    jne .Lend_or_" << thisLabel << "\n";
    o << "    movl " << rhs << ", %eax\n";
    o << "    cmpl $0, %eax\n";
    o << "    jne .Lend_or_" << thisLabel << "\n";
    o << "    movl $0, %eax\n";
    o << "    jmp .Ldone_or_" << thisLabel << "\n";
    o << ".Lend_or_" << thisLabel << ":\n";
    o << "    movl $1, %eax\n";
    o << ".Ldone_or_" << thisLabel << ":\n";
    o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::visit(ostream& o, FToIInstr& instr) {
    // cvttsd2si: convert double (src XMM) to 32-bit int (dest GPR), truncating
    o << "    cvttsd2sil " << reg_to_asm(instr.src) << ", " << reg_to_asm(instr.dest) << "\n";
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
    o << "    ret\n";
}

// ---------------------------------------------------------------------------
// Prologue / Epilogue / Control flow
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::gen_prologue(ostream& o) {
    int stackSpace = cfg->calculateRequiredStackSpace();
    // Get the function name from the first basic block's label
    string funcName = "main";
    if (!cfg->getBBs().empty()) {
        funcName = cfg->getBBs()[0]->label;
    }
    // Skip .globl here since it's generated in gen_asm for all functions
    o << funcName << ":\n";
    o << "    pushq %rbp\n";
    o << "    movq %rsp, %rbp\n";
    o << "    subq $" << stackSpace << ", %rsp\n";
}

void AsmGeneratorX86_64::gen_epilogue(ostream& o) {
    o << "    leave\n";
    o << "    ret\n";
}
