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
    // For FLOAT64, map to ARM64 FP/SIMD double registers
    if (p.type == IRType::FLOAT64) {
        switch (p.reg) {
            case Reg::W0: case Reg::RET: case Reg::ARG0: return "d0";
            case Reg::ARG1:                               return "d1";
            case Reg::ARG2:                               return "d2";
            case Reg::ARG3:                               return "d3";
            case Reg::ARG4:                               return "d4";
            case Reg::ARG5:                               return "d5";
            case Reg::W1:                                 return "d8";
            case Reg::W2:                                 return "d9";
            case Reg::W3:                                 return "d10";
        }
    }
    bool is64 = (p.type == IRType::INT64);
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
// Main generation functions
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::gen_asm(ostream& o) {
    // Export only real function symbols (not internal BB labels).
    for (auto& fn : cfg->get_functions()) {
        o << ".globl _" << fn.label << "\n";
    }

    bool isFirstBB = true;
    for (auto bb : cfg->getBBs()) {
        gen_asm_bb(o, bb, isFirstBB);
        isFirstBB = false;
    }
}

void AsmGeneratorARM64::gen_asm_bb(ostream& o, BasicBlock* bb, bool isFirstBB) {
    cfg->current_bb = bb;

    auto* sig = cfg->get_function(bb->label);
    bool isFunctionEntry = (sig != nullptr);

    if (isFunctionEntry) {
        int stackSpace = cfg->calculateRequiredStackSpace();
        o << "_" << bb->label << ":\n";
        o << "    stp fp, lr, [sp, #-16]!\n";
        o << "    mov fp, sp\n";
        o << "    sub sp, sp, #" << stackSpace << "\n";
    } else if (!isFirstBB) {
        o << bb->label << ":\n";
    }

    for (auto instr : bb->instrs)
        cfg->gen_asm_instr(o, instr);

    bool hasReturn = false;
    if (!bb->instrs.empty() && dynamic_cast<RetInstr*>(bb->instrs.back()) != nullptr) {
        hasReturn = true;
    }

    if (!hasReturn)
        gen_control_flow(o, bb);
}

void AsmGeneratorARM64::gen_asm_instr(ostream& o, IRInstr* instr) {
    instr->accept(*this, o);
}

// ---------------------------------------------------------------------------
// Prologue / Epilogue / Control flow
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::gen_prologue(ostream& o) {
    int stackSpace = cfg->calculateRequiredStackSpace();
    string funcName = "main";
    if (!cfg->get_functions().empty()) {
        funcName = cfg->get_functions()[0].label;
    }
    // Skip .globl here since it's generated in gen_asm for all functions
    o <<  " " << funcName << ":\n";
    o << "    stp fp, lr, [sp, #-16]!\n";  // Save frame pointer and link register
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

// ---------------------------------------------------------------------------
// Logical Operations
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::LogicalAnd(ostream& o, string lhs, string rhs, string dest) {
    int thisLabel = getNextLabel();
    o << "    cmp " << lhs << ", #0\n";
    o << "    b.eq .Lend_and_" << thisLabel << "\n";
    o << "    cmp " << rhs << ", #0\n";
    o << "    b.eq .Lend_and_" << thisLabel << "\n";
    o << "    mov " << dest << ", #1\n";
    o << "    b .Ldone_and_" << thisLabel << "\n";
    o << ".Lend_and_" << thisLabel << ":\n";
    o << "    mov " << dest << ", #0\n";
    o << ".Ldone_and_" << thisLabel << ":\n";
}
void AsmGeneratorARM64::LogicalOr(ostream& o, string lhs, string rhs, string dest) {
    int thisLabel = getNextLabel();
    o << "    cmp " << lhs << ", #0\n";
    o << "    b.ne .Lend_or_" << thisLabel << "\n";
    o << "    cmp " << rhs << ", #0\n";
    o << "    b.ne .Lend_or_" << thisLabel << "\n";
    o << "    mov " << dest << ", #0\n";
    o << "    b .Ldone_or_" << thisLabel << "\n";
    o << ".Lend_or_" << thisLabel << ":\n";
    o << "    mov " << dest << ", #1\n";
    o << ".Ldone_or_" << thisLabel << ":\n";
}

// ---------------------------------------------------------------------------
// Function Call / Return
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::CallWithINT32Return(ostream& o, string funcLabel, vector<string> args, string dest) {
    int numArgs = (int) args.size();
    for (int i = 0; i < numArgs && i < 6; i++) {
        string src = args[i];
        // If argument is a floating register (dN / vN), move to FP arg register d{i}
        if (!src.empty() && (src[0] == 'd' || src.rfind("v", 0) == 0)) {
            string dst = "d" + to_string(i);
            if (src != dst)
                o << "    fmov " << dst << ", " << src << "\n";
        } else if (!src.empty() && src[0] == 'x') {
            string dst = "x" + to_string(i);
            if (src != dst)
                o << "    mov " << dst << ", " << src << "\n";
        } else {
            // default to 32-bit register move
            string dst = "w" + to_string(i);
            if (src != dst)
                o << "    mov " << dst << ", " << src << "\n";
        }
    }
    o << "    bl _" << funcLabel << "\n";
    // Move return value from w0/d0 to destination when needed
    if (!dest.empty() && dest[0] == 'd') {
        if (dest != "d0") o << "    fmov " << dest << ", d0\n";
    } else {
        if (dest != "w0") o << "    mov " << dest << ", w0\n";
    }
}

void AsmGeneratorARM64::CallWithFLOAT64Return(ostream& o, string funcLabel, vector<string> args, string dest) {
    int numArgs = (int) args.size();
    for (int i = 0; i < numArgs && i < 6; i++) {
        string src = args[i];
        if (!src.empty() && (src[0] == 'd' || src.rfind("v", 0) == 0)) {
            string dst = "d" + to_string(i);
            if (src != dst)
                o << "    fmov " << dst << ", " << src << "\n";
        } else if (!src.empty() && src[0] == 'x') {
            string dst = "x" + to_string(i);
            if (src != dst)
                o << "    mov " << dst << ", " << src << "\n";
        } else {
            string dst = "w" + to_string(i);
            if (src != dst)
                o << "    mov " << dst << ", " << src << "\n";
        }
    }
    o << "    bl _" << funcLabel << "\n";
    if (!dest.empty() && dest[0] == 'd') {
        if (dest != "d0") o << "    fmov " << dest << ", d0\n";
    } else {
        if (dest != "w0") o << "    mov " << dest << ", w0\n";
    }
}

void AsmGeneratorARM64::Ret(ostream& o) {
    o << "    mov sp, fp\n";
    o << "    ldp fp, lr, [sp], #16\n";
    o << "    ret\n";
}

// ---------------------------------------------------------------------------
// Load Constants
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::ldConstInstrINT8(std::ostream& o, ConstParam src, std::string dest) {
    o << "    mov " << dest << ", #" << src.raw_int() << "\n";
}
void AsmGeneratorARM64::ldConstInstrINT32(std::ostream& o, ConstParam src, std::string dest) {
    int64_t val = src.raw_int();
    uint32_t uval = static_cast<uint32_t>(val & 0xFFFFFFFF);
    if (uval < 65536) {
        o << "    mov " << dest << ", #" << uval << "\n";
    } else {
        o << "    movz " << dest << ", #" << (uval & 0xFFFF) << "\n";
        if (uval >> 16)
            o << "    movk " << dest << ", #" << (uval >> 16) << ", lsl #16\n";
    }
}
void AsmGeneratorARM64::ldConstInstrINT64(std::ostream& o, ConstParam src, std::string dest) {
    int64_t val = src.raw_int();
    o << "    mov x9, #" << (val & 0xFFFF) << "\n";
    if ((val >> 16) & 0xFFFF)
        o << "    movk x9, #" << ((val >> 16) & 0xFFFF) << ", lsl #16\n";
    if ((val >> 32) & 0xFFFF)
        o << "    movk x9, #" << ((val >> 32) & 0xFFFF) << ", lsl #32\n";
    if ((val >> 48) & 0xFFFF)
        o << "    movk x9, #" << ((val >> 48) & 0xFFFF) << ", lsl #48\n";
    if (dest != "x9")
        o << "    mov " << dest << ", x9\n";
}
void AsmGeneratorARM64::ldConstInstrFLOAT64(std::ostream& o, double src, std::string dest) {
    // Load 64-bit IEEE754 bit pattern via an integer scratch register
    uint64_t bits = std::bit_cast<uint64_t>(src);
    o << "    mov x9, #" << (bits & 0xFFFF) << "\n";
    if ((bits >> 16) & 0xFFFF)
        o << "    movk x9, #" << ((bits >> 16) & 0xFFFF) << ", lsl #16\n";
    if ((bits >> 32) & 0xFFFF)
        o << "    movk x9, #" << ((bits >> 32) & 0xFFFF) << ", lsl #32\n";
    if ((bits >> 48) & 0xFFFF)
        o << "    movk x9, #" << ((bits >> 48) & 0xFFFF) << ", lsl #48\n";
    o << "    fmov " << dest << ", x9\n";
}

// ---------------------------------------------------------------------------
// Register Copy
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::CopyRegINT8(ostream& o, string src, string dest) {
    if (src != dest)
        o << "    mov " << dest << ", " << src << "\n";
}
void AsmGeneratorARM64::CopyRegINT32(ostream& o, string src, string dest) {
    if (src != dest)
        o << "    mov " << dest << ", " << src << "\n";
}
void AsmGeneratorARM64::CopyRegFLOAT64(ostream& o, string src, string dest) {
    if (src != dest)
        o << "    fmov " << dest << ", " << src << "\n";
}

// ---------------------------------------------------------------------------
// Stack Operations (Load)
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::LoadStackInstrINT8(ostream& o, string src, string dest) {
    o << "    ldrsb " << dest << ", " << var_to_asm(src) << "\n";
}
void AsmGeneratorARM64::LoadStackInstrINT32(ostream& o, string src, string dest) {
    o << "    ldr " << dest << ", " << var_to_asm(src) << "\n";
}
void AsmGeneratorARM64::LoadStackInstrFLOAT64(ostream& o, string src, string dest) {
    o << "    ldr " << dest << ", " << var_to_asm(src) << "\n";
}

// ---------------------------------------------------------------------------
// Arithmetic Operations
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::AddINT32(ostream& o, string lhs, string rhs, string dest) {
    o << "    add " << dest << ", " << lhs << ", " << rhs << "\n";
}
void AsmGeneratorARM64::AddFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    o << "    fadd " << dest << ", " << lhs << ", " << rhs << "\n";
}
void AsmGeneratorARM64::MulINT32(ostream& o, string lhs, string rhs, string dest) {
    o << "    mul " << dest << ", " << lhs << ", " << rhs << "\n";
}
void AsmGeneratorARM64::MulFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    o << "    fmul " << dest << ", " << lhs << ", " << rhs << "\n";
}

// ---------------------------------------------------------------------------
// Division / Modulo
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::DivINT32(ostream& o, string lhs, string rhs, string dest) {
    o << "    sdiv " << dest << ", " << lhs << ", " << rhs << "\n";
}
void AsmGeneratorARM64::DivFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    o << "    fdiv " << dest << ", " << lhs << ", " << rhs << "\n";
}

// ---------------------------------------------------------------------------
// Bitwise Operations
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::BitNot(ostream& o, string src, string dest) {
    o << "    eor " << dest << ", " << src << ", #0xFFFFFFFF\n";
}
void AsmGeneratorARM64::BitAnd(ostream& o, string lhs, string rhs, string dest) {
    o << "    and " << dest << ", " << lhs << ", " << rhs << "\n";
}
void AsmGeneratorARM64::BitXor(ostream& o, string lhs, string rhs, string dest) {
    o << "    eor " << dest << ", " << lhs << ", " << rhs << "\n";
}

// ---------------------------------------------------------------------------
// Comparison Operations
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::CmpEqINT32(ostream& o, string lhs, string rhs, string dest) {
    o << "    cmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", eq\n";
}
void AsmGeneratorARM64::CmpEqFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    o << "    fcmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", eq\n";
}

// ---------------------------------------------------------------------------
// Less-Than / Less-Equal Comparisons
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::CmpLeINT32(ostream& o, string lhs, string rhs, string dest) {
    o << "    cmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", le\n";
}
void AsmGeneratorARM64::CmpLeFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    o << "    fcmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", le\n";
}

// ---------------------------------------------------------------------------
// Greater-Than / Greater-Equal Comparisons
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::CmpGtINT32(ostream& o, string lhs, string rhs, string dest) {
    o << "    cmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", gt\n";
}
void AsmGeneratorARM64::CmpGtFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    o << "    fcmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", gt\n";
}

// ---------------------------------------------------------------------------
// Stack Operations (Store)
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::StoreStackInstrINT8(ostream& o, string src, string dest) {
    o << "    strb " << src << ", " << dest << "\n";
}
void AsmGeneratorARM64::StoreStackInstrINT32(ostream& o, string src, string dest) {
    o << "    str " << src << ", " << dest << "\n";
}
void AsmGeneratorARM64::StoreStackInstrFLOAT64(ostream& o, string src, string dest) {
    o << "    str " << src << ", " << dest << "\n";
}

// ---------------------------------------------------------------------------
// Subtraction
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::SubINT32(ostream& o, string lhs, string rhs, string dest) {
    o << "    sub " << dest << ", " << lhs << ", " << rhs << "\n";
}
void AsmGeneratorARM64::SubFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    o << "    fsub " << dest << ", " << lhs << ", " << rhs << "\n";
}

// ---------------------------------------------------------------------------
// Modulo
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::ModINT32(ostream& o, string lhs, string rhs, string dest) {
    // ARM64: no modulo instruction; use sdiv + msub: dest = lhs - (lhs/rhs)*rhs
    string tmp = "w3";
    o << "    sdiv " << tmp << ", " << lhs << ", " << rhs << "\n";
    o << "    msub " << dest << ", " << tmp << ", " << rhs << ", " << lhs << "\n";
}

// ---------------------------------------------------------------------------
// Bitwise OR
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::BitOr(ostream& o, string lhs, string rhs, string dest) {
    o << "    orr " << dest << ", " << lhs << ", " << rhs << "\n";
}

// ---------------------------------------------------------------------------
// Less-Than Comparison
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::CmpLtINT32(ostream& o, string lhs, string rhs, string dest) {
    o << "    cmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", lt\n";
}
void AsmGeneratorARM64::CmpLtFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    o << "    fcmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", lt\n";
}

// ---------------------------------------------------------------------------
// Greater-Equal Comparison
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::CmpGeINT32(ostream& o, string lhs, string rhs, string dest) {
    o << "    cmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", ge\n";
}


void AsmGeneratorARM64::CmpGeFLOAT64(ostream& o, string lhs, string rhs, string dest) {
    o << "    fcmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", ge\n";
}

// ---------------------------------------------------------------------------
// Type Conversion
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::FToI(ostream& o, string src, string dest) {
    // fcvtzs: convert double (src) to 32-bit integer (dest), truncating toward zero
    o << "    fcvtzs " << dest << ", " << src << "\n";
}

void AsmGeneratorARM64::I32ToF64(ostream& o, string src, string dest) {
    // scvtf: convert signed int32 to double
    o << "    scvtf " << dest << ", " << src << "\n";
}

void AsmGeneratorARM64::I8ToI32(ostream& o, string src, string dest) {
    // sxtb: sign extend byte to 32-bit
    o << "    sxtb " << dest << ", " << src << "\n";
}

void AsmGeneratorARM64::I32ToI8(ostream& o, string src, string dest) {
    // Truncate int32 to int8: sxtb sign-extends the low byte back to 32-bit
    o << "    sxtb " << dest << ", " << src << "\n";
}
