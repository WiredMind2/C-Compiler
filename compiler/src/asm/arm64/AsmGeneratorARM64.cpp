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
// gen_asm / gen_asm_bb / gen_asm_instr
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::gen_asm(ostream& o) {
    // Generate .globl for all functions
    for (auto bb : cfg->getBBs()) {
        o << ".globl _" << bb->label << "\n";
    }
    cfg->gen_asm_prologue(o);
    bool isFirstBB = true;
    for (auto bb : cfg->getBBs()) {
        gen_asm_bb(o, bb, isFirstBB);
        isFirstBB = false;
    }
}

void AsmGeneratorARM64::gen_asm_bb(ostream& o, BasicBlock* bb, bool isFirstBB) {
    cfg->current_bb = bb;
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
    if (instr.type == IRType::FLOAT64) {
        // Load 64-bit IEEE754 bit pattern via an integer scratch register
        uint64_t bits = std::bit_cast<uint64_t>(instr.val.as_f64());
        o << "    mov x9, #" << (bits & 0xFFFF) << "\n";
        if ((bits >> 16) & 0xFFFF)
            o << "    movk x9, #" << ((bits >> 16) & 0xFFFF) << ", lsl #16\n";
        if ((bits >> 32) & 0xFFFF)
            o << "    movk x9, #" << ((bits >> 32) & 0xFFFF) << ", lsl #32\n";
        if ((bits >> 48) & 0xFFFF)
            o << "    movk x9, #" << ((bits >> 48) & 0xFFFF) << ", lsl #48\n";
        o << "    fmov " << dest << ", x9\n";
        return;
    }
    int64_t val = instr.val.raw_int();
    if (instr.type == IRType::INT64) {
        o << "    mov x9, #" << (val & 0xFFFF) << "\n";
        if ((val >> 16) & 0xFFFF)
            o << "    movk x9, #" << ((val >> 16) & 0xFFFF) << ", lsl #16\n";
        if ((val >> 32) & 0xFFFF)
            o << "    movk x9, #" << ((val >> 32) & 0xFFFF) << ", lsl #32\n";
        if ((val >> 48) & 0xFFFF)
            o << "    movk x9, #" << ((val >> 48) & 0xFFFF) << ", lsl #48\n";
        if (dest != "x9")
            o << "    mov " << dest << ", x9\n";
        return;
    }
    // INT32
    uint32_t uval = static_cast<uint32_t>(val & 0xFFFFFFFF);
    if (uval < 65536) {
        o << "    mov " << dest << ", #" << uval << "\n";
    } else {
        o << "    movz " << dest << ", #" << (uval & 0xFFFF) << "\n";
        if (uval >> 16)
            o << "    movk " << dest << ", #" << (uval >> 16) << ", lsl #16\n";
    }
}


void AsmGeneratorARM64::addINT32(std::ostream& o, string lhs_register, string rhs_register, string dest_register) {
    return ;
}


void AsmGeneratorARM64::addFLOAT64(std::ostream& o, string lhs_register, string rhs_register, string dest_register) {
    return ;
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
    if (instr.type == IRType::FLOAT64)
        o << "    fadd " << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    else
        o << "    add "  << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, SubInstr& instr) {
    if (instr.type == IRType::FLOAT64)
        o << "    fsub " << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    else
        o << "    sub "  << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, MulInstr& instr) {
    if (instr.type == IRType::FLOAT64)
        o << "    fmul " << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    else
        o << "    mul "  << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, DivInstr& instr) {
    if (instr.type == IRType::FLOAT64)
        o << "    fdiv " << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    else
        o << "    sdiv " << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, ModInstr& instr) {
    // ARM64: no modulo instruction; use sdiv + msub: dest = lhs - (lhs/rhs)*rhs
    string dest = reg_to_asm(instr.dest);
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    // Use W2/W3 as scratch — but dest may be same as lhs, so use a scratch via W3
    string tmp  = reg_to_asm(RegParam(Reg::W3, instr.type));
    o << "    sdiv " << tmp  << ", " << lhs << ", " << rhs << "\n";
    o << "    msub " << dest << ", " << tmp << ", " << rhs << ", " << lhs << "\n";
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
    if (instr.type == IRType::FLOAT64) {
        o << "    fcmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    } else {
        o << "    cmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    }
    o << "    cset " << dest << ", eq\n";
}

void AsmGeneratorARM64::visit(ostream& o, CmpLtInstr& instr) {
    string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::FLOAT64) {
        o << "    fcmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    } else {
        o << "    cmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    }
    o << "    cset " << dest << ", lt\n";
}

void AsmGeneratorARM64::visit(ostream& o, CmpLeInstr& instr) {
    string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::FLOAT64) {
        o << "    fcmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    } else {
        o << "    cmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    }
    o << "    cset " << dest << ", le\n";
}

void AsmGeneratorARM64::visit(ostream &o, CmpGtInstr &instr) {
    if (instr.type == IRType::FLOAT64) {
        o << "    fcmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    } else {
        o << "    cmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    }
    o << "    cset " << reg_to_asm(instr.dest) << ", gt\n";
}

void AsmGeneratorARM64::visit(ostream &o, CmpGeInstr &instr) {
    if (instr.type == IRType::FLOAT64) {
        o << "    fcmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    } else {
        o << "    cmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    }
    o << "    cset " << reg_to_asm(instr.dest) << ", ge\n";
}


void AsmGeneratorARM64::visit(ostream &o, LogicalAndInstr &instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    static int labelCount = 0;
    int thisLabel = labelCount++;
    o << "    cbz " << lhs << ", .Ldone_and_" << thisLabel << "\n";
    o << ".Lend_and_" << thisLabel << ":\n";
    // Sets lhs to 1 if rhs is nonzero, else 0
    o << "    subs	" << rhs << ", " << rhs << ", #0\n";
    o << "    cset	" << lhs << ", ne\n";
    o << ".Ldone_and_" << thisLabel << ":\n";
    // Sets dest to 1 if lhs is 1 else set it to 0
    o << "    and	" << dest << ", " << lhs << ", #0x1\n";
}
void AsmGeneratorARM64::visit(ostream &o, LogicalOrInstr &instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    static int labelCount = 0;
    int thisLabel = labelCount++;
    o << "    cbz " << lhs << ", .Lend_or_" << thisLabel << "\n";
    o << "    mov " << dest << ", #1\n";
    o << "    b .Ldone_or_" << thisLabel << "\n";
    o << ".Lend_or_" << thisLabel << ":\n";
    // Sets dest to 1 if rhs is nonzero, else 0
    o << "    subs	" << rhs << ", " << rhs << ", #0\n";
    o << "    cset	" << dest << ", ne\n";
    o << ".Ldone_or_" << thisLabel << ":\n";
}

void AsmGeneratorARM64::visit(ostream& o, FToIInstr& instr) {
    // fcvtzs: convert double (src) to 32-bit integer (dest), truncating toward zero
    o << "    fcvtzs " << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.src) << "\n";
}

void AsmGeneratorARM64::visit(ostream& o, CallInstr& instr) {
    int numArgs = (int) instr.args.size();
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
    // Get the function name from the first basic block's label
    string funcName = "_main";
    if (!cfg->getBBs().empty()) {
        funcName = "_" + cfg->getBBs()[0]->label;
    }
    // Skip .globl here since it's generated in gen_asm for all functions
    o << funcName << ":\n";
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

void AsmGeneratorARM64::LogicalAnd(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::LogicalOr(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::Call(ostream& o, string funcLabel, vector<string> args, string dest) {}
void AsmGeneratorARM64::Ret(ostream& o, string src) {}
void AsmGeneratorARM64::ldConstInstrINT8(std::ostream& o, std::string src, std::string dest) {}
void AsmGeneratorARM64::ldConstInstrINT32(std::ostream& o, std::string src, std::string dest) {}
void AsmGeneratorARM64::ldConstInstrFLOAT64(std::ostream& o, std::string src, std::string dest) {}
void AsmGeneratorARM64::CopyRegINT8(ostream& o, string src, string dest) {}
void AsmGeneratorARM64::CopyRegINT32(ostream& o, string src, string dest) {}
void AsmGeneratorARM64::CopyRegFLOAT64(ostream& o, string src, string dest) {}
void AsmGeneratorARM64::LoadStackInstrINT8(ostream& o, string src, string dest) {}
void AsmGeneratorARM64::LoadStackInstrINT32(ostream& o, string src, string dest) {}
void AsmGeneratorARM64::LoadStackInstrFLOAT64(ostream& o, string src, string dest) {}
void AsmGeneratorARM64::AddINT32(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::AddFLOAT64(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::AddINT32_by_FLOAT64(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::AddFLOAT64_by_INT32(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::MulINT32_by_INT32(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::MulFLOAT64_by_FLOAT64(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::MulINT32_by_FLOAT64(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::MulFLOAT64_by_INT32(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::DivINT32_by_INT32(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::DivFLOAT64_by_FLOAT64(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::DivINT32_by_FLOAT64(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::DivFLOAT64_by_INT32(ostream& o, string lhs, string rhs, string dest) {} 
void AsmGeneratorARM64::BitNot(ostream& o, string src, string dest) {}
void AsmGeneratorARM64::BitAnd(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::BitXor(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::CmpEqINT32_with_INT32(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::CmpEqFLOAT64_with_FLOAT64(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::CmpEqFLOAT64_with_INT32(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::CmpEqINT32_with_FLOAT64(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::CmpLeINT32_with_INT32(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::CmpLeFLOAT64_with_FLOAT64(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::CmpLeFLOAT64_with_INT32(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::CmpLeINT32_with_FLOAT64(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::CmpGtINT32_with_INT32(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::CmpGtFLOAT64_with_FLOAT64(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::CmpGtFLOAT64_with_INT32(ostream& o, string lhs, string rhs, string dest) {}
void AsmGeneratorARM64::CmpGtINT32_with_FLOAT64(ostream& o, string lhs, string rhs, string dest) {}
