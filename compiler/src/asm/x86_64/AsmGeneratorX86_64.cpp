#include "AsmGeneratorX86_64.h"
#include "../../ir/IR.h"
#include "../../ir/IRInstr.h"
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
    bool is64 = (p.type == IRType::INT64);
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
// Main generation functions
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

    // Check if this is a function entry block (has the same name as a function)
    auto* sig = cfg->get_function(bb->label);
    bool isFunctionEntry = (sig != nullptr);

    // Only generate prologue for function entry blocks
    if (isFunctionEntry) {
        // Generate prologue for this function entry block
        int stackSpace = cfg->calculateRequiredStackSpace();
        o << bb->label << ":\n";
        o << "    pushq %rbp\n";
        o << "    movq %rsp, %rbp\n";
        o << "    subq $" << stackSpace << ", %rsp\n";

        // Copy parameters from ABI registers to stack slots (System V ABI)
        // Parameters are stored at positive offsets: 16(%rbp), 24(%rbp), ...
        static const string argRegs64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
        if (sig) {
            int numParams = (int)sig->paramNames.size();
            for (int i = 0; i < numParams && i < 6; i++) {
                int offset = 16 + i * 8;
                o << "    movq " << argRegs64[i] << ", " << offset << "(%rbp)\n";
            }
        }
    } else {
        // For non-entry blocks, just output the label
        o << bb->label << ":\n";
    }

    // Generate instructions
    for (auto instr : bb->instrs) {
        gen_asm_instr(o, instr);
    }

    // Check if the last instruction is a return - if so, don't generate control flow
    bool hasReturn = false;
    if (!bb->instrs.empty()) {
        // Check if last instr is RetInstr
        IRInstr* lastInstr = bb->instrs.back();
        if (dynamic_cast<RetInstr*>(lastInstr) != nullptr) {
            hasReturn = true;
        }
    }

    // If there's no explicit return, generate control flow (jump or return)
    if (!hasReturn) {
        gen_control_flow(o, bb);
    }
}

void AsmGeneratorX86_64::gen_asm_instr(ostream& o, IRInstr* instr) {
    instr->accept(*this, o);
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static string reg32_to_reg8(const string& reg32) {
    if (reg32 == "%eax") return "%al";
    if (reg32 == "%ecx") return "%cl";
    if (reg32 == "%edx") return "%dl";
    if (reg32 == "%ebx") return "%bl";
    if (reg32 == "%edi") return "%dil";
    if (reg32 == "%esi") return "%sil";
    if (reg32 == "%r8d") return "%r8b";
    if (reg32 == "%r9d") return "%r9b";
    throw std::invalid_argument("reg32_to_reg8: unknown register " + reg32);
}

// ---------------------------------------------------------------------------
// Load Constants
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::ldConstInstrINT8(ostream& o, ConstParam constant, string dest) {
    o << "    movb $" << constant.raw_int() << ", " << reg32_to_reg8(dest) << "\n";
}
void AsmGeneratorX86_64::ldConstInstrINT32(ostream& o, ConstParam constant, string dest) {
    o << "    movl $" << constant.raw_int() << ", " << dest << "\n";
}
void AsmGeneratorX86_64::ldConstInstrINT64(ostream& o, ConstParam constant, string dest) {
    o << "    movq $" << constant.raw_int() << ", " << dest << "\n";
}
void AsmGeneratorX86_64::ldConstInstrFLOAT64(ostream& o, double constant, string dest) {
    uint64_t bits = std::bit_cast<uint64_t>(constant);
    o << "    movabsq $" << bits << ", %rax\n";
    o << "    movq %rax, " << dest << "\n";
}

// ---------------------------------------------------------------------------
// Register Copy
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::CopyRegINT8(ostream& o, string src, string dest) {
    if (src != dest)
        o << "    movb " << reg32_to_reg8(src) << ", " << reg32_to_reg8(dest) << "\n";
}
void AsmGeneratorX86_64::CopyRegINT32(ostream& o, string src, string dest) {
    if (src != dest) {
        o << "    movl " << src << ", " << dest << "\n";
    }
}
void AsmGeneratorX86_64::CopyRegFLOAT64(ostream& o, string src, string dest) {
    if (src != dest) {
        o << "    movsd " << src << ", " << dest << "\n";
    }
}

// ---------------------------------------------------------------------------
// Stack Operations (Store)
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::StoreStackInstrINT8(ostream& o, string src, string dest) {
    o << "    movb " << reg32_to_reg8(src) << ", " << dest << "\n";
}
void AsmGeneratorX86_64::StoreStackInstrINT32(ostream& o, string src, string dest) {
    o << "    movl " << src << ", " << dest << "\n";
}

void AsmGeneratorX86_64::StoreStackInstrFLOAT64(ostream& o, string src, string dest) {
    o << "    movsd " << src << ", " << dest << "\n";
}

// ---------------------------------------------------------------------------
// Stack Operations (Load)
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::LoadStackInstrINT8(ostream& o, string src, string dest) {
    o << "    movsbl " << var_to_asm(src) << ", " << dest << "\n";
}
void AsmGeneratorX86_64::LoadStackInstrINT32(ostream& o, string src, string dest) {
    o << "    movl " << var_to_asm(src) << ", " << dest << "\n";
}

void AsmGeneratorX86_64::LoadStackInstrFLOAT64(ostream& o, string src, string dest) {
    o << "    movsd " << var_to_asm(src) << ", " << dest << "\n";
}

// ---------------------------------------------------------------------------
// Arithmetic Operations
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::AddINT32(ostream& o, string lhs, string rhs, string dest) {
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    addl " << rhs << ", " << dest << "\n";
}
void AsmGeneratorX86_64::AddFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    o << "    movsd " << lhs << ", %xmm0\n";
    o << "    addsd " << rhs << ", %xmm0\n";
    o << "    movsd %xmm0, " << dest << "\n";
}

// ---------------------------------------------------------------------------
// Subtraction
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::SubINT32(ostream& o, string lhs, string rhs, string dest) {
    if (dest != lhs) {
        o << "    movl " << lhs << ", " << dest << "\n";
    }
    o << "    subl " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::SubFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    o << "    movsd " << lhs << ", %xmm0\n";
    o << "    subsd " << rhs << ", %xmm0\n";
    o << "    movsd %xmm0, " << dest << "\n";
}

// ---------------------------------------------------------------------------
// Multiplication
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::MulINT32(ostream& o, string lhs, string rhs, string dest) {
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    imull " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::MulFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    if (dest != lhs)
        o << "    movsd " << lhs << ", " << dest << "\n";
    o << "    mulsd " << rhs << ", " << dest << "\n";
}

// ---------------------------------------------------------------------------
// Division
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::DivINT32(ostream& o, string lhs, string rhs, string dest) {
    if (lhs != "%eax")
        o << "    movl " << lhs << ", %eax\n";
    o << "    cltd\n";
    o << "    idivl " << rhs << "\n";
    if (dest != "%eax")
        o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::DivFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    o << "    movsd " << lhs << ", %xmm0\n";
    o << "    divsd " << rhs << ", %xmm0\n";
    o << "    movsd %xmm0, " << dest << "\n";
}

// ---------------------------------------------------------------------------
// Modulo
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::ModINT32(ostream& o, string lhs, string rhs, string dest) {
    if (lhs != "%eax") {
        o << "    movl " << lhs << ", %eax\n";
    }
    o << "    cltd\n";
    o << "    idivl " << rhs << "\n";
    if (dest != "%edx") {
        o << "    movl %edx, " << dest << "\n";
    }
}

// ---------------------------------------------------------------------------
// Bitwise Operations
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::BitNot(ostream& o, string src, string dest) {
    if (dest != src)
        o << "    movl " << src << ", " << dest << "\n";
    o << "    notl " << dest << "\n";
}

void AsmGeneratorX86_64::BitAnd(ostream& o, string lhs, string rhs, string dest) {
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    andl " << rhs << ", " << dest << "\n";
}

// ---------------------------------------------------------------------------
// Bitwise OR
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::BitOr(ostream& o, string lhs, string rhs, string dest) {
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    orl " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::BitXor(ostream& o, string lhs, string rhs, string dest) {
    if (dest != lhs)
        o << "    movl " << lhs << ", " << dest << "\n";
    o << "    xorl " << rhs << ", " << dest << "\n";
}

// ---------------------------------------------------------------------------
// Comparison Operations
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::CmpEqINT32(ostream& o, string lhs, string rhs, string dest) {
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl " << rhs << ", %eax\n";
    o << "    sete %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}
void AsmGeneratorX86_64::CmpEqFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    o << "    movsd " << lhs << ", %xmm0\n";
    o << "    ucomisd " << rhs << ", %xmm0\n";
    o << "    sete %al\n";
    o << "    setnp %cl\n";
    o << "    andb %cl, %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << dest << "\n";
}

// ---------------------------------------------------------------------------
// Less-Than Comparisons
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::CmpLtINT32(ostream& o, string lhs, string rhs, string dest) {
    if (lhs != "%eax") {
        o << "    movl " << lhs << ", %eax\n";
    }
    o << "    cmpl " << rhs << ", %eax\n";
    o << "    setl %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") {
        o << "    movl %eax, " << dest << "\n";
    }
}

void AsmGeneratorX86_64::CmpLtFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    if (lhs != "%xmm0") {
        o << "    movsd " << lhs << ", %xmm0\n";
    }
    o << "    comisd " << rhs << ", %xmm0\n";
    o << "    setb %al\n";
    o << "    setnp %cl\n";
    o << "    andb %cl, %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") {
        o << "    movl %eax, " << dest << "\n";
    }
}

// ---------------------------------------------------------------------------
// Less-Equal Comparisons
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::CmpLeINT32(ostream& o, string lhs, string rhs, string dest) {
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl " << rhs << ", %eax\n";
    o << "    setle %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::CmpLeFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    o << "    movsd " << lhs << ", %xmm0\n";
    o << "    ucomisd " << rhs << ", %xmm0\n";
    o << "    setle %al\n";
    o << "    setnp %cl\n";
    o << "    andb %cl, %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << dest << "\n";
}

// ---------------------------------------------------------------------------
// Greater-Than Comparisons
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::CmpGtINT32(ostream& o, string lhs, string rhs, string dest) {
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl " << rhs << ", %eax\n";
    o << "    setg %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::CmpGtFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    o << "    movsd " << lhs << ", %xmm0\n";
    o << "    ucomisd " << rhs << ", %xmm0\n";
    o << "    setg %al\n";
    o << "    setnp %cl\n";
    o << "    andb %cl, %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << dest << "\n";
}

// ---------------------------------------------------------------------------
// Greater-Equal Comparison
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::CmpGeINT32(ostream& o, string lhs, string rhs, string dest) {
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl " << rhs << ", %eax\n";
    o << "    setge %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::CmpGeFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    if (lhs != "%xmm0") {
        o << "    movsd " << lhs << ", %xmm0\n";
    }
    o << "    comisd " << rhs << ", %xmm0\n";
    o << "    setae %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") {
        o << "    movl %eax, " << dest << "\n";
    }
}


// ---------------------------------------------------------------------------
// Logical Operations
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::LogicalAnd(ostream& o, string lhs, string rhs, string dest) {
    int thisLabel = getNextLabel();
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

void AsmGeneratorX86_64::LogicalOr(ostream& o, string lhs, string rhs, string dest) {
    int thisLabel = getNextLabel();
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

// ---------------------------------------------------------------------------
// Type Conversion
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::FToI(ostream& o, string src, string dest) {
    // cvttsd2si: convert double (src XMM) to 32-bit int (dest GPR), truncating
    o << "    cvttsd2sil " << src << ", " << dest << "\n";
}

void AsmGeneratorX86_64::I32ToF64(ostream& o, string src, string dest) {
    // cvtsi2sdl: convert int32 to double (low)
    o << "    cvtsi2sdl " << src << ", " << dest << "\n";
}

void AsmGeneratorX86_64::I8ToI32(ostream& o, string src, string dest) {
    // movsbl: sign-extend byte to doubleword; source must be an 8-bit register
    o << "    movsbl " << reg32_to_reg8(src) << ", " << dest << "\n";
}

void AsmGeneratorX86_64::I32ToI8(ostream& o, string src, string dest) {
    // Truncate int32 to int8: keep low byte and sign-extend back to 32-bit
    if (src != dest)
        o << "    movl " << src << ", " << dest << "\n";
    o << "    movsbl " << reg32_to_reg8(dest) << ", " << dest << "\n";
}

// ---------------------------------------------------------------------------
// Function Call / Return
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::Call(ostream& o, string funcLabel, vector<string> args, string dest) {
    static const string argRegs64[] = {"%rdi","%rsi","%rdx","%rcx","%r8","%r9"};
    for (int i = 0; i < args.size() && i < 6; i++) {
        string r32 = args[i];
        o << "    movslq " << r32 << ", " << argRegs64[i] << "\n";
    }
    o << "    call " << funcLabel << "\n";
    if (dest != "%eax")
        o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::Ret(ostream& o) {
    o << "    leave\n";
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

void AsmGeneratorX86_64::gen_control_flow(ostream& o, BasicBlock* bb) {
    if (bb->exit_true == nullptr) {
        gen_epilogue(o);
    } else if (bb->exit_false == nullptr) {
        // Unconditional jump
        o << "    jmp " << bb->exit_true->label << "\n";
    } else {
        // Both exit_true and exit_false set: conditional jump
        if (!bb->test_var_name.empty()) {
            string test_asm = var_to_asm(bb->test_var_name);
            o << "    movl " << test_asm << ", %eax\n";
        } else {
            // Both exits are set but no condition variable provided.
            // This is a bug in original CFG construction.
            cerr << "Internal Error: Conditional branch in " << bb->label
                 << " has no test_var_name." << endl;
            exit(1);
        }
        // If condition FALSE (eax == 0), jump to exit_false
        o << "    cmpl $0, %eax\n";
        o << "    je "  << bb->exit_false->label << "\n";
        // Always emit explicit jump to exit_true (do not rely on physical BB ordering)
        o << "    jmp " << bb->exit_true->label << "\n";
    }
}
