#include "AsmGeneratorARM64.h"
#include "../../ir/IR.h"
#include "../../ir/IRInstr.h"
#include <iostream>
#include <stdexcept>

using namespace std; // Keep this line for consistency

AsmGeneratorARM64::AsmGeneratorARM64(CFG* cfg) : AsmGenerator(cfg) {}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

string AsmGeneratorARM64::reg_to_asm(const RegParam& p) {
    // For FLOAT64, map to ARM64 FP/SIMD double registers
    if (p.type == IRType::FLOAT64) {
        switch (p.reg) {
            case Reg::W0: case Reg::RET: return "d0";
            case Reg::ARG0:              return "d0";
            case Reg::ARG1:              return "d1";
            case Reg::ARG2:              return "d2";
            case Reg::ARG3:              return "d3";
            case Reg::ARG4:              return "d4";
            case Reg::ARG5:              return "d5";
            case Reg::W1:                return "d8";
            case Reg::W2:                return "d9";
            case Reg::W3:                return "d10";
            case Reg::W4:                return "d11";
            case Reg::W5:                return "d12";
        }
    }
    bool is64 = (p.type == IRType::INT64 || p.type == IRType::POINTER);
    const char* prefix = is64 ? "x" : "w";
    switch (p.reg) {
        case Reg::W0: case Reg::RET:   return string(prefix) + "0";
        case Reg::ARG0:                return string(prefix) + "0";
        case Reg::ARG1:                return string(prefix) + "1";
        case Reg::ARG2:                return string(prefix) + "2";
        case Reg::ARG3:                return string(prefix) + "3";
        case Reg::ARG4:                return string(prefix) + "4";
        case Reg::ARG5:                return string(prefix) + "5";
        case Reg::W1:                  return string(prefix) + "9";
        case Reg::W2:                  return string(prefix) + "10";
        case Reg::W3:                  return string(prefix) + "11";
        case Reg::W4:                  return string(prefix) + "12";
        case Reg::W5:                  return string(prefix) + "13";
    }
    throw std::invalid_argument("reg_to_asm: unknown Reg");
}

string AsmGeneratorARM64::var_to_asm(const string& varName) {
    int index = cfg->current_bb->get_var_index(varName);
    return "[fp, #" + to_string(index) + "]";
}

namespace {
std::string stack_mem_operand(CFG* cfg, std::ostream& o, const std::string& varName) {
    int index = cfg->current_bb->get_var_index(varName);
    if (index >= -256 && index <= 255) {
        return "[fp, #" + std::to_string(index) + "]";
    }
    if (index >= 0) {
        o << "    add x16, fp, #" << index << "\n";
    } else {
        o << "    sub x16, fp, #" << (-index) << "\n";
    }
    return "[x16]";
}
}

// ---------------------------------------------------------------------------
// Main generation functions
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::gen_asm(std::ostream& o) {
    if (!cfg->stringLiterals.empty()) {
        o << "    .section __TEXT,__cstring,cstring_literals\n";
        for (size_t i = 0; i < cfg->stringLiterals.size(); ++i) {
            o << ".LC" << i << ":\n";
            o << "    .asciz " << cfg->stringLiterals[i] << "\n";
        }
        o << "    .text\n";
    }

    // Generate .globl for all functions
    for (auto bb : cfg->getBBs()) {
        auto* sig = cfg->get_function(bb->label);
        if (sig) {
            o << ".globl _" << bb->label << "\n";
        }
    }
    bool isFirstBB = true;
    for (auto bb : cfg->getBBs()) {
        gen_asm_bb(o, bb, isFirstBB);
        isFirstBB = false;
    }
}

void AsmGeneratorARM64::gen_asm_bb(std::ostream& o, BasicBlock* bb, bool isFirstBB) {
    cfg->current_bb = bb;

    auto* sig = cfg->get_function(bb->label);
    if (sig) {
        int stackSpace = cfg->calculateRequiredStackSpace();
        o << "_" << bb->label << ":\n";
        o << "    stp fp, lr, [sp, #-16]!\n";
        o << "    mov fp, sp\n";
        o << "    sub sp, sp, #" << stackSpace << "\n";

        // Note: Parameter storage is handled by IR instructions (StoreStackInstr),
        // not here in the prologue. The IR generation in CodeGenFunction.cpp
        // creates StoreStackInstr for each parameter.
    } else {
        o << bb->label << ":\n";
    }

    for (auto instr : bb->instrs)
        cfg->gen_asm_instr(o, instr);
    gen_control_flow(o, bb);
}

void AsmGeneratorARM64::gen_asm_instr(std::ostream& o, IRInstr* instr) {
    instr->accept(*this, o);
}

// ---------------------------------------------------------------------------
// Visitor implementations
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::visit(std::ostream& o, LdStringInstr& instr) {
    o << "    adrp " << reg_to_asm(instr.dest) << ", .LC" << instr.strIndex << "@PAGE\n";
    o << "    add " << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.dest) << ", .LC" << instr.strIndex << "@PAGEOFF\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, LdConstInstr& instr) {
    string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::FLOAT64) {
        // Load 64-bit IEEE754 bit pattern via an integer scratch register
        uint64_t bits = std::bit_cast<uint64_t>(instr.val.as_f64());
        o << "    mov x16, #" << (bits & 0xFFFF) << "\n";
        if ((bits >> 16) & 0xFFFF)
            o << "    movk x16, #" << ((bits >> 16) & 0xFFFF) << ", lsl #16\n";
        if ((bits >> 32) & 0xFFFF)
            o << "    movk x16, #" << ((bits >> 32) & 0xFFFF) << ", lsl #32\n";
        if ((bits >> 48) & 0xFFFF)
            o << "    movk x16, #" << ((bits >> 48) & 0xFFFF) << ", lsl #48\n";
        o << "    fmov " << dest << ", x16\n";
        return;
    }
    int64_t val = instr.val.raw_int();
    if (instr.type == IRType::INT64) {
        o << "    mov x16, #" << (val & 0xFFFF) << "\n";
        if ((val >> 16) & 0xFFFF)
            o << "    movk x16, #" << ((val >> 16) & 0xFFFF) << ", lsl #16\n";
        if ((val >> 32) & 0xFFFF)
            o << "    movk x16, #" << ((val >> 32) & 0xFFFF) << ", lsl #32\n";
        if ((val >> 48) & 0xFFFF)
            o << "    movk x16, #" << ((val >> 48) & 0xFFFF) << ", lsl #48\n";
        if (dest != "x16")
            o << "    mov " << dest << ", x16\n";
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

void AsmGeneratorARM64::visit(std::ostream& o, CopyRegInstr& instr) {
    string src  = reg_to_asm(instr.src);
    string dest = reg_to_asm(instr.dest);
    if (src != dest) {
        if (instr.type == IRType::FLOAT64) {
            o << "    fmov " << dest << ", " << src << "\n";
        } else if (!dest.empty() && !src.empty() && dest[0] == 'x' && src[0] == 'w') {
            // 32->64 explicit extension (used by INT32 -> POINTER conversions).
            o << "    sxtw " << dest << ", " << src << "\n";
        } else {
            o << "    mov " << dest << ", " << src << "\n";
        }
    }
}

void AsmGeneratorARM64::visit(std::ostream& o, StoreStackInstr& instr) {
    std::string mem = stack_mem_operand(cfg, o, instr.dest.name);
    o << "    str " << reg_to_asm(instr.src) << ", " << mem << "\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, LoadStackInstr& instr) {
    std::string mem = stack_mem_operand(cfg, o, instr.src.name);
    o << "    ldr " << reg_to_asm(instr.dest) << ", " << mem << "\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, AddressOfSymbolInstr& instr) {
    int index = cfg->current_bb->get_var_index(instr.src.name);
    if (index >= 0) {
        o << "    add " << reg_to_asm(instr.dest) << ", fp, #" << index << "\n";
    } else {
        o << "    sub " << reg_to_asm(instr.dest) << ", fp, #" << (-index) << "\n";
    }
}

void AsmGeneratorARM64::visit(std::ostream& o, LoadPointerInstr& instr) {
    if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        o << "    ldr " << reg_to_asm(instr.dest) << ", [" << reg_to_asm(instr.ptr) << "]\n";
    } else if (instr.type == IRType::FLOAT64) {
        o << "    ldr " << reg_to_asm(instr.dest) << ", [" << reg_to_asm(instr.ptr) << "]\n";
    } else if (instr.type == IRType::INT8) {
        // Note: We currently treat char/INT8 as signed (ldrsb).
        // If unsigned types are added later, this will need a UINT8 type and ldrb.
        o << "    ldrsb " << reg_to_asm(instr.dest) << ", [" << reg_to_asm(instr.ptr) << "]\n";
    } else {
        o << "    ldr " << reg_to_asm(instr.dest) << ", [" << reg_to_asm(instr.ptr) << "]\n";
    }
}

void AsmGeneratorARM64::visit(std::ostream& o, StorePointerInstr& instr) {
    if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        o << "    str " << reg_to_asm(instr.src) << ", [" << reg_to_asm(instr.ptr) << "]\n";
    } else if (instr.type == IRType::FLOAT64) {
        o << "    str " << reg_to_asm(instr.src) << ", [" << reg_to_asm(instr.ptr) << "]\n";
    } else if (instr.type == IRType::INT8) {
        // Truncates to 8-bit. Consistent with char being a signed 8-bit int.
        o << "    strb " << reg_to_asm(instr.src) << ", [" << reg_to_asm(instr.ptr) << "]\n";
    } else {
        o << "    str " << reg_to_asm(instr.src) << ", [" << reg_to_asm(instr.ptr) << "]\n";
    }
}

void AsmGeneratorARM64::visit(std::ostream& o, AddInstr& instr) {
    if (instr.type == IRType::FLOAT64)
        o << "    fadd " << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    else
        o << "    add "  << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, SubInstr& instr) {
    if (instr.type == IRType::FLOAT64)
        o << "    fsub " << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    else
        o << "    sub "  << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, MulInstr& instr) {
    if (instr.type == IRType::FLOAT64)
        o << "    fmul " << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    else
        o << "    mul "  << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, DivInstr& instr) {
    if (instr.type == IRType::FLOAT64)
        o << "    fdiv " << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    else
        o << "    sdiv " << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, ModInstr& instr) {
    // ARM64: no modulo instruction; use sdiv + msub: dest = lhs - (lhs/rhs)*rhs
    string dest = reg_to_asm(instr.dest);
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    // Use W2/W3 as scratch — but dest may be same as lhs, so use a scratch via W3
    string tmp  = reg_to_asm(RegParam(Reg::W3, instr.type));
    o << "    sdiv " << tmp  << ", " << lhs << ", " << rhs << "\n";
    o << "    msub " << dest << ", " << tmp << ", " << rhs << ", " << lhs << "\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, BitNotInstr& instr) {
    string src  = reg_to_asm(instr.src);
    string dest = reg_to_asm(instr.dest);
    o << "    mvn " << dest << ", " << src << "\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, BitAndInstr& instr) {
    o << "    and " << reg_to_asm(instr.dest) << ", "
                   << reg_to_asm(instr.lhs)  << ", "
                   << reg_to_asm(instr.rhs)  << "\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, BitOrInstr& instr) {
    o << "    orr " << reg_to_asm(instr.dest) << ", "
                   << reg_to_asm(instr.lhs)  << ", "
                   << reg_to_asm(instr.rhs)  << "\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, BitXorInstr& instr) {
    o << "    eor " << reg_to_asm(instr.dest) << ", "
                   << reg_to_asm(instr.lhs)  << ", "
                   << reg_to_asm(instr.rhs)  << "\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, ShlInstr& instr) {
    string dest = reg_to_asm(instr.dest);
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    o << "    lsl " << dest << ", " << lhs << ", " << rhs << "\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, ShrInstr& instr) {
    string dest = reg_to_asm(instr.dest);
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    o << "    asr " << dest << ", " << lhs << ", " << rhs << "\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, CmpEqInstr& instr) {
    string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::FLOAT64) {
        o << "    fcmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    } else {
        o << "    cmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    }
    o << "    cset " << dest << ", eq\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, CmpLtInstr& instr) {
    string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::FLOAT64) {
        o << "    fcmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    } else {
        o << "    cmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    }
    o << "    cset " << dest << ", lt\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, CmpLeInstr& instr) {
    string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::FLOAT64) {
        o << "    fcmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    } else {
        o << "    cmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    }
    o << "    cset " << dest << ", le\n";
}

void AsmGeneratorARM64::visit(std::ostream &o, CmpGtInstr &instr) {
    if (instr.type == IRType::FLOAT64) {
        o << "    fcmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    } else {
        o << "    cmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    }
    o << "    cset " << reg_to_asm(instr.dest) << ", gt\n";
}

void AsmGeneratorARM64::visit(std::ostream &o, CmpGeInstr &instr) {
    if (instr.type == IRType::FLOAT64) {
        o << "    fcmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    } else {
        o << "    cmp " << reg_to_asm(instr.lhs) << ", " << reg_to_asm(instr.rhs) << "\n";
    }
    o << "    cset " << reg_to_asm(instr.dest) << ", ge\n";
}


void AsmGeneratorARM64::visit(std::ostream &o, LogicalAndInstr &instr) {
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
void AsmGeneratorARM64::visit(std::ostream &o, LogicalOrInstr &instr) {
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

void AsmGeneratorARM64::visit(std::ostream& o, FToIInstr& instr) {
    // fcvtzs: convert double (src) to 32-bit integer (dest), truncating toward zero
    o << "    fcvtzs " << reg_to_asm(instr.dest) << ", " << reg_to_asm(instr.src) << "\n";
}

void AsmGeneratorARM64::visit(std::ostream& o, CallInstr& instr) {
    std::string callee = instr.funcLabel;
#ifdef __APPLE__
    if (!callee.empty() && callee[0] != '_') callee = "_" + callee;
#endif

    int numArgs = (int) instr.args.size();
    for (int i = 0; i < numArgs && i < 6; i++) {
        string src = reg_to_asm(instr.args[i]);
        string dst;
        if (instr.args[i].type == IRType::FLOAT64) {
            dst = "d" + to_string(i);
            if (src != dst) o << "    fmov " << dst << ", " << src << "\n";
        } else if (instr.args[i].type == IRType::INT64 || instr.args[i].type == IRType::POINTER) {
            dst = "x" + to_string(i);
            if (src != dst) o << "    mov " << dst << ", " << src << "\n";
        } else {
            dst = "w" + to_string(i);
            if (src != dst) o << "    mov " << dst << ", " << src << "\n";
        }
    }
    o << "    bl " << callee << "\n";
    string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::FLOAT64) {
        if (dest != "d0") o << "    fmov " << dest << ", d0\n";
    } else if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        if (dest != "x0") o << "    mov " << dest << ", x0\n";
    } else {
        if (dest != "w0") o << "    mov " << dest << ", w0\n";
    }
}

void AsmGeneratorARM64::visit(std::ostream& o, F64ToI32Instr& instr) {
    FToI(o, reg_to_asm(instr.src), reg_to_asm(instr.dest));
}

void AsmGeneratorARM64::visit(std::ostream& o, I32ToF64Instr& instr) {
    I32ToF64(o, reg_to_asm(instr.src), reg_to_asm(instr.dest));
}

void AsmGeneratorARM64::visit(std::ostream& o, I8ToI32Instr& instr) {
    I8ToI32(o, reg_to_asm(instr.src), reg_to_asm(instr.dest));
}

void AsmGeneratorARM64::visit(std::ostream& o, I32ToI8Instr& instr) {
    I32ToI8(o, reg_to_asm(instr.src), reg_to_asm(instr.dest));
}

void AsmGeneratorARM64::visit(std::ostream& o, RetInstr& instr) {
    (void)instr;
    Ret(o);
}

// ---------------------------------------------------------------------------
// Prologue / Epilogue / Control flow
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::gen_prologue(std::ostream& o) {
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

void AsmGeneratorARM64::gen_epilogue(std::ostream& o) {
    o << "    mov sp, fp\n";
    o << "    ldp fp, lr, [sp], #16\n";
    o << "    ret\n";
}

void AsmGeneratorARM64::gen_control_flow(std::ostream& o, BasicBlock* bb) {
    if (bb->exit_true == nullptr) {
        o << "    mov sp, fp\n";
        o << "    ldp fp, lr, [sp], #16\n";
        o << "    ret\n";
    } else if (bb->exit_false == nullptr) {
        o << "    b " << bb->exit_true->label << "\n";
    } else {
        // Use 64-bit load/compare for pointer tests, 32-bit otherwise
        if (bb->get_var_type(bb->test_var_name) == IRType::POINTER) {
            std::string mem = stack_mem_operand(cfg, o, bb->test_var_name);
            o << "    ldr x0, " << mem << "\n";
            o << "    cmp x0, #0\n";
        } else {
            std::string mem = stack_mem_operand(cfg, o, bb->test_var_name);
            o << "    ldr w0, " << mem << "\n";
            o << "    cmp w0, #0\n";
        }
        o << "    b.ne " << bb->exit_true->label << "\n";   // if non-zero (true), go to then-block
        o << "    b "    << bb->exit_false->label << "\n";  // else go to else-block
    }
}

// ---------------------------------------------------------------------------
// Logical Operations
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::LogicalAnd(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
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
void AsmGeneratorARM64::LogicalOr(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
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

void AsmGeneratorARM64::CallWithINT32Return(std::ostream& o, const std::string& funcLabel, const std::vector<std::string>& args, const std::string& dest) {
    std::string callee = funcLabel;
#ifdef __APPLE__
    if (!callee.empty() && callee[0] != '_') callee = "_" + callee;
#endif

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
    o << "    bl " << callee << "\n";
    // Move return value from w0/d0 to destination when needed
    if (!dest.empty() && dest[0] == 'd') {
        if (dest != "d0") o << "    fmov " << dest << ", d0\n";
    } else {
        if (dest != "w0") o << "    mov " << dest << ", w0\n";
    }
}

void AsmGeneratorARM64::CallWithFLOAT64Return(std::ostream& o, const std::string& funcLabel, const std::vector<std::string>& args, const std::string& dest) {
    std::string callee = funcLabel;
#ifdef __APPLE__
    if (!callee.empty() && callee[0] != '_') callee = "_" + callee;
#endif

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
    o << "    bl " << callee << "\n";
    if (!dest.empty() && dest[0] == 'd') {
        if (dest != "d0") o << "    fmov " << dest << ", d0\n";
    } else {
        if (dest != "w0") o << "    mov " << dest << ", w0\n";
    }
}

void AsmGeneratorARM64::Ret(std::ostream& o) {
    o << "    mov sp, fp\n";
    o << "    ldp fp, lr, [sp], #16\n";
    o << "    ret\n";
}

// ---------------------------------------------------------------------------
// Load Constants
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::ldConstInstrINT8(std::ostream& o, ConstParam src, const std::string& dest) {
    o << "    mov " << dest << ", #" << src.raw_int() << "\n";
}
void AsmGeneratorARM64::ldConstInstrINT32(std::ostream& o, ConstParam src, const std::string& dest) {
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
void AsmGeneratorARM64::ldConstInstrINT64(std::ostream& o, ConstParam src, const std::string& dest) {
    int64_t val = src.raw_int();
    o << "    mov x16, #" << (val & 0xFFFF) << "\n";
    if ((val >> 16) & 0xFFFF)
        o << "    movk x16, #" << ((val >> 16) & 0xFFFF) << ", lsl #16\n";
    if ((val >> 32) & 0xFFFF)
        o << "    movk x16, #" << ((val >> 32) & 0xFFFF) << ", lsl #32\n";
    if ((val >> 48) & 0xFFFF)
        o << "    movk x16, #" << ((val >> 48) & 0xFFFF) << ", lsl #48\n";
    if (dest != "x16")
        o << "    mov " << dest << ", x16\n";
}
void AsmGeneratorARM64::ldConstInstrFLOAT64(std::ostream& o, double src, const std::string& dest) {
    // Load 64-bit IEEE754 bit pattern via an integer scratch register
    uint64_t bits = std::bit_cast<uint64_t>(src);
    o << "    mov x16, #" << (bits & 0xFFFF) << "\n";
    if ((bits >> 16) & 0xFFFF)
        o << "    movk x16, #" << ((bits >> 16) & 0xFFFF) << ", lsl #16\n";
    if ((bits >> 32) & 0xFFFF)
        o << "    movk x16, #" << ((bits >> 32) & 0xFFFF) << ", lsl #32\n";
    if ((bits >> 48) & 0xFFFF)
        o << "    movk x16, #" << ((bits >> 48) & 0xFFFF) << ", lsl #48\n";
    o << "    fmov " << dest << ", x16\n";
}

// ---------------------------------------------------------------------------
// Register Copy
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::CopyRegINT8(std::ostream& o, const std::string& src, const std::string& dest) {
    if (src != dest)
        o << "    mov " << dest << ", " << src << "\n";
}
void AsmGeneratorARM64::CopyRegINT32(std::ostream& o, const std::string& src, const std::string& dest) {
    if (src != dest)
        o << "    mov " << dest << ", " << src << "\n";
}
void AsmGeneratorARM64::CopyRegFLOAT64(std::ostream& o, const std::string& src, const std::string& dest) {
    if (src != dest)
        o << "    fmov " << dest << ", " << src << "\n";
}

// ---------------------------------------------------------------------------
// Stack Operations (Load)
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::LoadStackInstrINT8(std::ostream& o, const std::string& src, const std::string& dest) {
    std::string mem = stack_mem_operand(cfg, o, src);
    o << "    ldrsb " << dest << ", " << mem << "\n";
}
void AsmGeneratorARM64::LoadStackInstrINT32(std::ostream& o, const std::string& src, const std::string& dest) {
    std::string mem = stack_mem_operand(cfg, o, src);
    o << "    ldr " << dest << ", " << mem << "\n";
}
void AsmGeneratorARM64::LoadStackInstrFLOAT64(std::ostream& o, const std::string& src, const std::string& dest) {
    std::string mem = stack_mem_operand(cfg, o, src);
    o << "    ldr " << dest << ", " << mem << "\n";
}

// ---------------------------------------------------------------------------
// Arithmetic Operations
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::AddINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    add " << dest << ", " << lhs << ", " << rhs << "\n";
}
void AsmGeneratorARM64::AddFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    fadd " << dest << ", " << lhs << ", " << rhs << "\n";
}
void AsmGeneratorARM64::MulINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    mul " << dest << ", " << lhs << ", " << rhs << "\n";
}
void AsmGeneratorARM64::MulFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    fmul " << dest << ", " << lhs << ", " << rhs << "\n";
}

// ---------------------------------------------------------------------------
// Division / Modulo
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::DivINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    sdiv " << dest << ", " << lhs << ", " << rhs << "\n";
}
void AsmGeneratorARM64::DivFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    fdiv " << dest << ", " << lhs << ", " << rhs << "\n";
}

// ---------------------------------------------------------------------------
// Bitwise Operations
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::BitNot(std::ostream& o, const std::string& src, const std::string& dest) {
    o << "    mvn " << dest << ", " << src << "\n";
}
void AsmGeneratorARM64::BitAnd(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    and " << dest << ", " << lhs << ", " << rhs << "\n";
}
void AsmGeneratorARM64::BitXor(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    eor " << dest << ", " << lhs << ", " << rhs << "\n";
}

// ---------------------------------------------------------------------------
// Comparison Operations
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::CmpEqINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    cmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", eq\n";
}
void AsmGeneratorARM64::CmpEqFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    fcmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", eq\n";
}

// ---------------------------------------------------------------------------
// Less-Than / Less-Equal Comparisons
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::CmpLeINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    cmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", le\n";
}
void AsmGeneratorARM64::CmpLeFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    fcmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", le\n";
}

// ---------------------------------------------------------------------------
// Greater-Than / Greater-Equal Comparisons
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::CmpGtINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    cmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", gt\n";
}
void AsmGeneratorARM64::CmpGtFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    fcmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", gt\n";
}

// ---------------------------------------------------------------------------
// Stack Operations (Store)
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::StoreStackInstrINT8(std::ostream& o, const std::string& src, const std::string& dest) {
    o << "    strb " << src << ", " << dest << "\n";
}
void AsmGeneratorARM64::StoreStackInstrINT32(std::ostream& o, const std::string& src, const std::string& dest) {
    o << "    str " << src << ", " << dest << "\n";
}
void AsmGeneratorARM64::StoreStackInstrFLOAT64(std::ostream& o, const std::string& src, const std::string& dest) {
    o << "    str " << src << ", " << dest << "\n";
}

// ---------------------------------------------------------------------------
// Subtraction
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::SubINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    sub " << dest << ", " << lhs << ", " << rhs << "\n";
}
void AsmGeneratorARM64::SubFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    fsub " << dest << ", " << lhs << ", " << rhs << "\n";
}

// ---------------------------------------------------------------------------
// Modulo
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::ModINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    // ARM64: no modulo instruction; use sdiv + msub: dest = lhs - (lhs/rhs)*rhs
    std::string tmp = "w3";
    o << "    sdiv " << tmp << ", " << lhs << ", " << rhs << "\n";
    o << "    msub " << dest << ", " << tmp << ", " << rhs << ", " << lhs << "\n";
}

// ---------------------------------------------------------------------------
// Bitwise OR
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::BitOr(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    orr " << dest << ", " << lhs << ", " << rhs << "\n";
}

// ---------------------------------------------------------------------------
// Less-Than Comparison
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::CmpLtINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    cmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", lt\n";
}
void AsmGeneratorARM64::CmpLtFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    fcmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", lt\n";
}

// ---------------------------------------------------------------------------
// Greater-Equal Comparison
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::CmpGeINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    cmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", ge\n";
}


void AsmGeneratorARM64::CmpGeFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    fcmp " << lhs << ", " << rhs << "\n";
    o << "    cset " << dest << ", ge\n";
}

// ---------------------------------------------------------------------------
// Type Conversion
// ---------------------------------------------------------------------------

void AsmGeneratorARM64::FToI(std::ostream& o, const std::string& src, const std::string& dest) {
    // fcvtzs: convert double (src) to 32-bit integer (dest), truncating toward zero
    o << "    fcvtzs " << dest << ", " << src << "\n";
}

void AsmGeneratorARM64::I32ToF64(std::ostream& o, const std::string& src, const std::string& dest) {
    // scvtf: convert signed int32 to double
    o << "    scvtf " << dest << ", " << src << "\n";
}

void AsmGeneratorARM64::I8ToI32(std::ostream& o, const std::string& src, const std::string& dest) {
    // sxtb: sign extend byte to 32-bit
    o << "    sxtb " << dest << ", " << src << "\n";
}

void AsmGeneratorARM64::I32ToI8(std::ostream& o, const std::string& src, const std::string& dest) {
    // Truncate int32 to int8: sxtb sign-extends the low byte back to 32-bit
    o << "    sxtb " << dest << ", " << src << "\n";
}
