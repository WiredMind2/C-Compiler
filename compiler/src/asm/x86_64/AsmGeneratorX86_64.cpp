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

// Small helper: map 32-bit register name to its low byte name
static string reg32_to_reg8(const string& reg32) {
    if (reg32 == "%eax" || reg32 == "%rax")  return "%al";
    if (reg32 == "%ecx" || reg32 == "%rcx")  return "%cl";
    if (reg32 == "%edx" || reg32 == "%rdx")  return "%dl";
    if (reg32 == "%ebx" || reg32 == "%rbx")  return "%bl";
    if (reg32 == "%edi" || reg32 == "%rdi")  return "%dil";
    if (reg32 == "%esi" || reg32 == "%rsi")  return "%sil";
    if (reg32 == "%ebp" || reg32 == "%rbp")  return "%bpl";
    if (reg32 == "%esp" || reg32 == "%rsp")  return "%spl";
    if (reg32 == "%r8d"  || reg32 == "%r8")  return "%r8b";
    if (reg32 == "%r9d"  || reg32 == "%r9")  return "%r9b";
    if (reg32 == "%r10d" || reg32 == "%r10") return "%r10b";
    if (reg32 == "%r11d" || reg32 == "%r11") return "%r11b";
    if (reg32 == "%r12d" || reg32 == "%r12") return "%r12b";
    if (reg32 == "%r13d" || reg32 == "%r13") return "%r13b";
    if (reg32 == "%r14d" || reg32 == "%r14") return "%r14b";
    if (reg32 == "%r15d" || reg32 == "%r15") return "%r15b";
    
    if (reg32 == "%al" || reg32 == "%cl" || reg32 == "%dl" || reg32 == "%bl" ||
        reg32 == "%dil" || reg32 == "%sil" || reg32 == "%bpl" || reg32 == "%spl" ||
        reg32 == "%r8b" || reg32 == "%r9b" || reg32 == "%r10b" || reg32 == "%r11b" ||
        reg32 == "%r12b" || reg32 == "%r13b" || reg32 == "%r14b" || reg32 == "%r15b") return reg32;

    throw std::invalid_argument("reg32_to_reg8: unknown register " + reg32);
}

// ---------------------------------------------------------------------------
// Main generation functions
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::gen_asm(std::ostream& o) {
    if (!cfg->stringLiterals.empty()) {
        o << "    .section .rodata\n";
        for (size_t i = 0; i < cfg->stringLiterals.size(); ++i) {
            o << ".LC" << i << ":\n";
            o << "    .string " << cfg->stringLiterals[i] << "\n";
        }
        o << "    .text\n";
    }

    for (auto bb : cfg->getBBs()) {
        o << ".globl " << bb->label << "\n";
    }

    bool isFirstBB = true;
    for (auto bb : cfg->getBBs()) {
        gen_asm_bb(o, bb, isFirstBB);
        isFirstBB = false;
    }
}

void AsmGeneratorX86_64::gen_asm_bb(std::ostream& o, BasicBlock* bb, bool /*isFirstBB*/) {
    cfg->current_bb = bb;
    auto* sig = cfg->get_function(bb->label);
    bool isFunctionEntry = (sig != nullptr);

    if (isFunctionEntry) {
        int stackSpace = bb->calculateRequiredStackSpace();
        o << bb->label << ":\n";
        o << "    pushq %rbp\n";
        o << "    movq %rsp, %rbp\n";
        o << "    subq $" << stackSpace << ", %rsp\n";

        static const string argRegs64[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
        static const string argRegsXMM[] = {"%xmm0", "%xmm1", "%xmm2", "%xmm3", "%xmm4", "%xmm5", "%xmm6", "%xmm7"};
        int numParams = (sig) ? (int)sig->paramNames.size() : 0;
        int intIdx = 0, floatIdx = 0;
        for (int i = 0; i < numParams; i++) {
            int offset = bb->get_var_index(sig->paramNames[i]);
            if (sig->paramTypes[i] == IRType::FLOAT32 || sig->paramTypes[i] == IRType::FLOAT64) {
                if (floatIdx < 8) {
                    o << "    movsd " << argRegsXMM[floatIdx++] << ", " << offset << "(%rbp)\n";
                }
            } else {
                if (intIdx < 6) {
                    o << "    movq " << argRegs64[intIdx++] << ", " << offset << "(%rbp)\n";
                }
            }
        }
    } else {
        o << bb->label << ":\n";
    }

    for (auto instr : bb->instrs) {
        gen_asm_instr(o, instr);
    }

    // If the last instruction wasn't a return, emit control flow
    bool hasReturn = false;
    if (!bb->instrs.empty()) {
        IRInstr* last = bb->instrs.back();
        if (dynamic_cast<RetInstr*>(last) != nullptr) hasReturn = true;
    }
    if (!hasReturn) gen_control_flow(o, bb);
}

void AsmGeneratorX86_64::gen_asm_instr(std::ostream& o, IRInstr* instr) {
    instr->accept(*this, o);
}

// ---------------------------------------------------------------------------
// Visitor wrappers: delegate to typed helpers
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::visit(std::ostream& o, LdStringInstr& instr) {
    o << "    leaq .LC" << instr.strIndex << "(%rip), " << reg_to_asm(instr.dest) << "\n";
}

void AsmGeneratorX86_64::visit(std::ostream& o, LdConstInstr& instr) {
    string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::INT8) {
        ldConstInstrINT8(o, instr.val, dest);
    } else if (instr.type == IRType::INT32) {
        ldConstInstrINT32(o, instr.val, dest);
    } else if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        ldConstInstrINT64(o, instr.val, dest);
    } else if (instr.type == IRType::FLOAT64) {
        ldConstInstrFLOAT64(o, instr.val.as_f64(), dest);
    } else {
        throw std::runtime_error("Unsupported LdConstInstr type in x86_64 backend");
    }
}

void AsmGeneratorX86_64::visit(std::ostream& o, CopyRegInstr& instr) {
    // Choose assembly register names according to the copy type
    if (instr.type == IRType::INT8) {
        string src8  = reg32_to_reg8(reg_to_asm(instr.src));
        string dest8 = reg32_to_reg8(reg_to_asm(instr.dest));
        CopyRegINT8(o, src8, dest8);
    } else if (instr.type == IRType::INT32) {
        RegParam src32(instr.src.reg, IRType::INT32);
        RegParam dest32(instr.dest.reg, IRType::INT32);
        CopyRegINT32(o, reg_to_asm(src32), reg_to_asm(dest32));
    } else if (instr.type == IRType::FLOAT64) {
        RegParam srcF(instr.src.reg, IRType::FLOAT64);
        RegParam destF(instr.dest.reg, IRType::FLOAT64);
        CopyRegFLOAT64(o, reg_to_asm(srcF), reg_to_asm(destF));
    } else if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        // 64-bit copy: if source register actually holds a 32-bit value,
        // write the 32-bit alias into the 64-bit destination to zero-extend.
        string src64 = reg_to_asm(instr.src);
        string dest64 = reg_to_asm(instr.dest);
        if (instr.src.type == IRType::INT32) {
            RegParam dest32(instr.dest.reg, IRType::INT32);
            string dest32asm = reg_to_asm(dest32);
            if (reg_to_asm(instr.src) != dest32asm)
                o << "    movl " << reg_to_asm(instr.src) << ", " << dest32asm << "\n";
        } else if (src64 != dest64) {
            o << "    movq " << src64 << ", " << dest64 << "\n";
        }
    }
}

void AsmGeneratorX86_64::visit(std::ostream& o, StoreStackInstr& instr) {
    if (instr.type == IRType::FLOAT64) {
        o << "    movsd " << reg_to_asm(instr.src)
          << ", "        << var_to_asm(instr.dest.name) << "\n";
    } else if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        string src = reg_to_asm(instr.src);
        if (instr.src.type == IRType::INT32) {
            // Zero-extend 32->64 by moving the 32-bit alias into %eax (which
            // zero-extends %rax), then store the full 64-bit register to memory.
            o << "    movl " << src << ", %eax\n";
            o << "    movq %rax, " << var_to_asm(instr.dest.name) << "\n";
        } else {
            o << "    movq " << src << ", " << var_to_asm(instr.dest.name) << "\n";
        }
    } else {
        o << "    movl " << reg_to_asm(instr.src)
          << ", "       << var_to_asm(instr.dest.name) << "\n";
    }
}

void AsmGeneratorX86_64::visit(std::ostream& o, LoadStackInstr& instr) {
    if (instr.type == IRType::FLOAT64) {
        o << "    movsd " << var_to_asm(instr.src.name)
          << ", "        << reg_to_asm(instr.dest) << "\n";
    } else if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        // Always load the full 64-bit value from stack into the register's 64-bit view
        RegParam dest64(instr.dest.reg, IRType::POINTER);
        o << "    movq " << var_to_asm(instr.src.name)
            << ", "       << reg_to_asm(dest64) << "\n";
    } else if (instr.type == IRType::INT8) {
        o << "    movsbl " << var_to_asm(instr.src.name)
          << ", "       << reg_to_asm(instr.dest) << "\n";
    } else {
        o << "    movl " << var_to_asm(instr.src.name)
          << ", "       << reg_to_asm(instr.dest) << "\n";
    }
}

void AsmGeneratorX86_64::visit(std::ostream& o, AddressOfSymbolInstr& instr) {
    o << "    leaq " << var_to_asm(instr.src.name)
      << ", " << reg_to_asm(instr.dest) << "\n";
}

void AsmGeneratorX86_64::visit(std::ostream& o, LoadPointerInstr& instr) {
    if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        o << "    movq (" << reg_to_asm(instr.ptr) << "), " << reg_to_asm(instr.dest) << "\n";
    } else if (instr.type == IRType::FLOAT64) {
        o << "    movsd (" << reg_to_asm(instr.ptr) << "), " << reg_to_asm(instr.dest) << "\n";
    } else {
        o << "    movl (" << reg_to_asm(instr.ptr) << "), " << reg_to_asm(instr.dest) << "\n";
    }
}

void AsmGeneratorX86_64::visit(std::ostream& o, StorePointerInstr& instr) {
    if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        o << "    movq " << reg_to_asm(instr.src) << ", (" << reg_to_asm(instr.ptr) << ")\n";
    } else if (instr.type == IRType::FLOAT64) {
        o << "    movsd " << reg_to_asm(instr.src) << ", (" << reg_to_asm(instr.ptr) << ")\n";
    } else {
        o << "    movl " << reg_to_asm(instr.src) << ", (" << reg_to_asm(instr.ptr) << ")\n";
    }
}

void AsmGeneratorX86_64::visit(std::ostream& o, AddInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::INT64) {
        if (dest != lhs) o << "    movq " << lhs << ", " << dest << "\n";
        o << "    addq " << rhs << ", " << dest << "\n";
    } else if (instr.type == IRType::POINTER) {
        if (dest != lhs) o << "    movq " << lhs << ", " << dest << "\n";
        o << "    addq " << rhs << ", " << dest << "\n";
    } else if (instr.type == IRType::INT32) {
        if (dest != lhs) o << "    movl " << lhs << ", " << dest << "\n";
        o << "    addl " << rhs << ", " << dest << "\n";
    } else if (instr.type == IRType::FLOAT64) {
        o << "    movsd " << lhs << ", %xmm0\n";
        o << "    addsd " << rhs << ", %xmm0\n";
        o << "    movsd %xmm0, " << dest << "\n";
    }
}

void AsmGeneratorX86_64::visit(std::ostream& o, SubInstr& instr) {
    string lhs  = reg_to_asm(instr.lhs);
    string rhs  = reg_to_asm(instr.rhs);
    string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        if (dest != lhs) o << "    movq " << lhs << ", " << dest << "\n";
        o << "    subq " << rhs << ", " << dest << "\n";
    } else if (instr.type == IRType::INT32) {
        if (dest != lhs) o << "    movl " << lhs << ", " << dest << "\n";
        o << "    subl " << rhs << ", " << dest << "\n";
    } else if (instr.type == IRType::FLOAT64) {
        o << "    movsd " << lhs << ", %xmm0\n";
        o << "    subsd " << rhs << ", %xmm0\n";
        o << "    movsd %xmm0, " << dest << "\n";
    }
}

void AsmGeneratorX86_64::visit(std::ostream& o, MulInstr& instr) {
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

void AsmGeneratorX86_64::visit(std::ostream& o, DivInstr& instr) {
    std::string lhs = reg_to_asm(instr.lhs);
    std::string rhs = reg_to_asm(instr.rhs);
    std::string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::FLOAT64)
        DivFLOAT64(o, lhs, rhs, dest);
    else
        DivINT32(o, lhs, rhs, dest);
}

void AsmGeneratorX86_64::visit(std::ostream& o, ModInstr& instr) {
    std::string lhs = reg_to_asm(instr.lhs);
    std::string rhs = reg_to_asm(instr.rhs);
    std::string dest = reg_to_asm(instr.dest);
    ModINT32(o, lhs, rhs, dest);
}

void AsmGeneratorX86_64::visit(std::ostream& o, BitNotInstr& instr) {
    BitNot(o, reg_to_asm(instr.src), reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, BitAndInstr& instr) {
    BitAnd(o, reg_to_asm(instr.lhs), reg_to_asm(instr.rhs), reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, BitOrInstr& instr) {
    BitOr(o, reg_to_asm(instr.lhs), reg_to_asm(instr.rhs), reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, BitXorInstr& instr) {
    BitXor(o, reg_to_asm(instr.lhs), reg_to_asm(instr.rhs), reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, ShlInstr& instr) {
    std::string lhs = reg_to_asm(instr.lhs);
    std::string rhs = reg_to_asm(instr.rhs);
    std::string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        if (dest != lhs) o << "    movq " << lhs << ", " << dest << "\n";
        if (rhs != "%rcx" && rhs != "%ecx") {
            if (instr.rhs.type == IRType::INT32)
                o << "    movl " << rhs << ", %ecx\n";
            else
                o << "    movq " << rhs << ", %rcx\n";
        }
        o << "    salq %cl, " << dest << "\n";
    } else {
        if (dest != lhs) o << "    movl " << lhs << ", " << dest << "\n";
        if (rhs != "%ecx") {
            o << "    movl " << rhs << ", %ecx\n";
        }
        o << "    sall %cl, " << dest << "\n";
    }
}

void AsmGeneratorX86_64::visit(std::ostream& o, ShrInstr& instr) {
    std::string lhs = reg_to_asm(instr.lhs);
    std::string rhs = reg_to_asm(instr.rhs);
    std::string dest = reg_to_asm(instr.dest);
    if (instr.type == IRType::INT64 || instr.type == IRType::POINTER) {
        if (dest != lhs) o << "    movq " << lhs << ", " << dest << "\n";
        if (rhs != "%rcx" && rhs != "%ecx") {
            if (instr.rhs.type == IRType::INT32)
                o << "    movl " << rhs << ", %ecx\n";
            else
                o << "    movq " << rhs << ", %rcx\n";
        }
        o << "    sarq %cl, " << dest << "\n";
    } else {
        if (dest != lhs) o << "    movl " << lhs << ", " << dest << "\n";
        if (rhs != "%ecx") {
            o << "    movl " << rhs << ", %ecx\n";
        }
        o << "    sarl %cl, " << dest << "\n";
    }
}

void AsmGeneratorX86_64::visit(std::ostream& o, CmpEqInstr& instr) {
    if (instr.type == IRType::FLOAT64)
        CmpEqFLOAT64(o, reg_to_asm(instr.lhs), reg_to_asm(instr.rhs), reg_to_asm(instr.dest));
    else
        CmpEqINT32(o, reg_to_asm(instr.lhs), reg_to_asm(instr.rhs), reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, CmpLtInstr& instr) {
    if (instr.type == IRType::FLOAT64)
        CmpLtFLOAT64(o, reg_to_asm(instr.lhs), reg_to_asm(instr.rhs), reg_to_asm(instr.dest));
    else
        CmpLtINT32(o, reg_to_asm(instr.lhs), reg_to_asm(instr.rhs), reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, CmpLeInstr& instr) {
    if (instr.type == IRType::FLOAT64)
        CmpLeFLOAT64(o, reg_to_asm(instr.lhs), reg_to_asm(instr.rhs), reg_to_asm(instr.dest));
    else
        CmpLeINT32(o, reg_to_asm(instr.lhs), reg_to_asm(instr.rhs), reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, CmpGtInstr& instr) {
    if (instr.type == IRType::FLOAT64)
        CmpGtFLOAT64(o, reg_to_asm(instr.lhs), reg_to_asm(instr.rhs), reg_to_asm(instr.dest));
    else
        CmpGtINT32(o, reg_to_asm(instr.lhs), reg_to_asm(instr.rhs), reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, CmpGeInstr& instr) {
    if (instr.type == IRType::FLOAT64)
        CmpGeFLOAT64(o, reg_to_asm(instr.lhs), reg_to_asm(instr.rhs), reg_to_asm(instr.dest));
    else
        CmpGeINT32(o, reg_to_asm(instr.lhs), reg_to_asm(instr.rhs), reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, LogicalAndInstr& instr) {
    LogicalAnd(o, reg_to_asm(instr.lhs), reg_to_asm(instr.rhs), reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, LogicalOrInstr& instr) {
    LogicalOr(o, reg_to_asm(instr.lhs), reg_to_asm(instr.rhs), reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, CallInstr& instr) {
    int numArgs = (int) instr.args.size();
    std::vector<std::string> args;
    for (int i = 0; i < numArgs; ++i) {
        args.push_back(reg_to_asm(instr.args[i]));
    }
    
    if (instr.dest.type == IRType::FLOAT64)
        CallWithFLOAT64Return(o, instr.funcLabel, args, reg_to_asm(instr.dest));
    else
        CallWithINT32Return(o, instr.funcLabel, args, reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, F64ToI32Instr& instr) {
    FToI(o, reg_to_asm(instr.src), reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, I32ToF64Instr& instr) {
    I32ToF64(o, reg_to_asm(instr.src), reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, I8ToI32Instr& instr) {
    I8ToI32(o, reg_to_asm(instr.src), reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, I32ToI8Instr& instr) {
    I32ToI8(o, reg_to_asm(instr.src), reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, FToIInstr& instr) {
    FToI(o, reg_to_asm(instr.src), reg_to_asm(instr.dest));
}

void AsmGeneratorX86_64::visit(std::ostream& o, RetInstr& instr) {
    Ret(o);
}

// ---------------------------------------------------------------------------
// Load constant helpers (typed API)
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::ldConstInstrINT8(std::ostream& o, ConstParam constant, const std::string& dest) {
    o << "    movb $" << constant.raw_int() << ", " << reg32_to_reg8(dest) << "\n";
}
void AsmGeneratorX86_64::ldConstInstrINT32(std::ostream& o, ConstParam constant, const std::string& dest) {
    o << "    movl $" << constant.raw_int() << ", " << dest << "\n";
}
void AsmGeneratorX86_64::ldConstInstrINT64(std::ostream& o, ConstParam constant, const std::string& dest) {
    o << "    movq $" << constant.raw_int() << ", " << dest << "\n";
}
void AsmGeneratorX86_64::ldConstInstrFLOAT64(std::ostream& o, double constant, const std::string& dest) {
    uint64_t bits = std::bit_cast<uint64_t>(constant);
    o << "    movabsq $" << bits << ", %rax\n";
    o << "    movq %rax, " << dest << "\n";
}

// ---------------------------------------------------------------------------
// Typed register-copy / stack / arithmetic APIs used elsewhere
// ---------------------------------------------------------------------------

void AsmGeneratorX86_64::CopyRegINT8(std::ostream& o, const std::string& src, const std::string& dest) {
    if (src != dest) o << "    movb " << reg32_to_reg8(src) << ", " << reg32_to_reg8(dest) << "\n";
}
void AsmGeneratorX86_64::CopyRegINT32(std::ostream& o, const std::string& src, const std::string& dest) {
    if (src != dest) o << "    movl " << src << ", " << dest << "\n";
}
void AsmGeneratorX86_64::CopyRegFLOAT64(std::ostream& o, const std::string& src, const std::string& dest) {
    if (src != dest) o << "    movsd " << src << ", " << dest << "\n";
}

void AsmGeneratorX86_64::StoreStackInstrINT8(std::ostream& o, const std::string& src, const std::string& dest) {
    o << "    movb " << reg32_to_reg8(src) << ", " << dest << "\n";
}
void AsmGeneratorX86_64::StoreStackInstrINT32(std::ostream& o, const std::string& src, const std::string& dest) {
    o << "    movl " << src << ", " << dest << "\n";
}
void AsmGeneratorX86_64::StoreStackInstrFLOAT64(std::ostream& o, const std::string& src, const std::string& dest) {
    o << "    movsd " << src << ", " << dest << "\n";
}

void AsmGeneratorX86_64::LoadStackInstrINT8(std::ostream& o, const std::string& src, const std::string& dest) {
    o << "    movsbl " << var_to_asm(src) << ", " << dest << "\n";
}
void AsmGeneratorX86_64::LoadStackInstrINT32(std::ostream& o, const std::string& src, const std::string& dest) {
    o << "    movl " << var_to_asm(src) << ", " << dest << "\n";
}
void AsmGeneratorX86_64::LoadStackInstrFLOAT64(std::ostream& o, const std::string& src, const std::string& dest) {
    o << "    movsd " << var_to_asm(src) << ", " << dest << "\n";
}

void AsmGeneratorX86_64::AddINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    if (dest != lhs) o << "    movl " << lhs << ", " << dest << "\n";
    o << "    addl " << rhs << ", " << dest << "\n";
}
void AsmGeneratorX86_64::AddFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    movsd " << lhs << ", %xmm0\n";
    o << "    addsd " << rhs << ", %xmm0\n";
    o << "    movsd %xmm0, " << dest << "\n";
}

void AsmGeneratorX86_64::SubINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    if (dest != lhs) o << "    movl " << lhs << ", " << dest << "\n";
    o << "    subl " << rhs << ", " << dest << "\n";
}
void AsmGeneratorX86_64::SubFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    movsd " << lhs << ", %xmm0\n";
    o << "    subsd " << rhs << ", %xmm0\n";
    o << "    movsd %xmm0, " << dest << "\n";
}

void AsmGeneratorX86_64::MulINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    if (dest != lhs) o << "    movl " << lhs << ", " << dest << "\n";
    o << "    imull " << rhs << ", " << dest << "\n";
}
void AsmGeneratorX86_64::MulFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    if (dest != lhs) o << "    movsd " << lhs << ", " << dest << "\n";
    o << "    mulsd " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::DivINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cltd\n";
    o << "    idivl " << rhs << "\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}
void AsmGeneratorX86_64::DivFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    movsd " << lhs << ", %xmm0\n";
    o << "    divsd " << rhs << ", %xmm0\n";
    o << "    movsd %xmm0, " << dest << "\n";
}

void AsmGeneratorX86_64::ModINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cltd\n";
    o << "    idivl " << rhs << "\n";
    if (dest != "%edx") o << "    movl %edx, " << dest << "\n";
}

void AsmGeneratorX86_64::BitNot(std::ostream& o, const std::string& src, const std::string& dest) {
    if (dest != src) o << "    movl " << src << ", " << dest << "\n";
    o << "    notl " << dest << "\n";
}
void AsmGeneratorX86_64::BitAnd(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    if (dest != lhs) o << "    movl " << lhs << ", " << dest << "\n";
    o << "    andl " << rhs << ", " << dest << "\n";
}
void AsmGeneratorX86_64::BitOr(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    if (dest != lhs) o << "    movl " << lhs << ", " << dest << "\n";
    o << "    orl " << rhs << ", " << dest << "\n";
}
void AsmGeneratorX86_64::BitXor(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    if (dest != lhs) o << "    movl " << lhs << ", " << dest << "\n";
    o << "    xorl " << rhs << ", " << dest << "\n";
}

void AsmGeneratorX86_64::CmpEqINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl " << rhs << ", %eax\n";
    o << "    sete %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}
void AsmGeneratorX86_64::CmpEqFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    movsd " << lhs << ", %xmm0\n";
    o << "    ucomisd " << rhs << ", %xmm0\n";
    o << "    sete %al\n";
    o << "    setnp %cl\n";
    o << "    andb %cl, %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::CmpLtINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl " << rhs << ", %eax\n";
    o << "    setl %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}
void AsmGeneratorX86_64::CmpLtFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    movsd " << lhs << ", %xmm0\n";
    o << "    comisd " << rhs << ", %xmm0\n";
    o << "    setb %al\n";
    o << "    setnp %cl\n";
    o << "    andb %cl, %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::CmpLeINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl " << rhs << ", %eax\n";
    o << "    setle %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}
void AsmGeneratorX86_64::CmpLeFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    movsd " << lhs << ", %xmm0\n";
    o << "    ucomisd " << rhs << ", %xmm0\n";
    o << "    setle %al\n";
    o << "    setnp %cl\n";
    o << "    andb %cl, %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::CmpGtINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl " << rhs << ", %eax\n";
    o << "    setg %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}
void AsmGeneratorX86_64::CmpGtFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    o << "    movsd " << lhs << ", %xmm0\n";
    o << "    ucomisd " << rhs << ", %xmm0\n";
    o << "    setg %al\n";
    o << "    setnp %cl\n";
    o << "    andb %cl, %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::CmpGeINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    if (lhs != "%eax") o << "    movl " << lhs << ", %eax\n";
    o << "    cmpl " << rhs << ", %eax\n";
    o << "    setge %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}
void AsmGeneratorX86_64::CmpGeFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
    if (lhs != "%xmm0") o << "    movsd " << lhs << ", %xmm0\n";
    o << "    comisd " << rhs << ", %xmm0\n";
    o << "    setae %al\n";
    o << "    movzbl %al, %eax\n";
    if (dest != "%eax") o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::LogicalAnd(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
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

void AsmGeneratorX86_64::LogicalOr(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {
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

void AsmGeneratorX86_64::FToI(std::ostream& o, const std::string& src, const std::string& dest) {
    o << "    cvttsd2sil " << src << ", " << dest << "\n";
}

void AsmGeneratorX86_64::I32ToF64(std::ostream& o, const std::string& src, const std::string& dest) {
    o << "    cvtsi2sdl " << src << ", " << dest << "\n";
}

void AsmGeneratorX86_64::I8ToI32(std::ostream& o, const std::string& src, const std::string& dest) {
    o << "    movsbl " << reg32_to_reg8(src) << ", " << dest << "\n";
}

void AsmGeneratorX86_64::I32ToI8(std::ostream& o, const std::string& src, const std::string& dest) {
    if (src != dest) o << "    movl " << src << ", " << dest << "\n";
    o << "    movsbl " << reg32_to_reg8(dest) << ", " << dest << "\n";
}

void AsmGeneratorX86_64::CallWithINT32Return(std::ostream& o, const std::string& funcLabel, const std::vector<std::string>& args, const std::string& dest) {
    static const std::string argRegs64[] = {"%rdi","%rsi","%rdx","%rcx","%r8","%r9"};
    int intArgCount = 0;
    int floatArgCount = 0;
    for (size_t i = 0; i < args.size(); i++) {
        const std::string& arg = args[i];
        if (arg.size() > 4 && arg.substr(0, 4) == "%xmm") {
            if (floatArgCount < 8) {
                if (arg != "%xmm" + std::to_string(floatArgCount)) {
                    o << "    movsd " << arg << ", %xmm" << floatArgCount << "\n";
                }
                floatArgCount++;
            }
        } else {
            if (intArgCount < 6) {
                // Determine if it's 64-bit or 32-bit register from the name
                if (arg.size() >= 3 && arg[1] == 'r') {
                    o << "    movq " << arg << ", " << argRegs64[intArgCount] << "\n";
                } else if (arg == "%rax" || arg == "%rbx" || arg == "%rcx" || arg == "%rdx" || arg == "%rsp" || arg == "%rbp" || arg == "%rsi" || arg == "%rdi") {
                    o << "    movq " << arg << ", " << argRegs64[intArgCount] << "\n";
                } else {
                    o << "    movslq " << arg << ", " << argRegs64[intArgCount] << "\n";
                }
                intArgCount++;
            }
        }
    }
    // o << "    movl $" << floatArgCount << ", %eax\n"; // Removed to avoid clobbering eax for double main()
    o << "    call " << funcLabel << "\n";
    if (dest != "%eax" && !dest.empty()) o << "    movl %eax, " << dest << "\n";
}

void AsmGeneratorX86_64::CallWithFLOAT64Return(std::ostream& o, const std::string& funcLabel, const std::vector<std::string>& args, const std::string& dest) {
    static const std::string argRegs64[] = {"%rdi","%rsi","%rdx","%rcx","%r8","%r9"};
    int intArgCount = 0;
    int floatArgCount = 0;
    for (size_t i = 0; i < args.size(); i++) {
        const std::string& arg = args[i];
        if (arg.size() > 4 && arg.substr(0, 4) == "%xmm") {
            if (floatArgCount < 8) {
                if (arg != "%xmm" + std::to_string(floatArgCount)) {
                    o << "    movsd " << arg << ", %xmm" << floatArgCount << "\n";
                }
                floatArgCount++;
            }
        } else {
            if (intArgCount < 6) {
                // Determine if it's 64-bit or 32-bit register from the name
                if (arg.size() >= 3 && arg[1] == 'r') {
                    o << "    movq " << arg << ", " << argRegs64[intArgCount] << "\n";
                } else if (arg == "%rax" || arg == "%rbx" || arg == "%rcx" || arg == "%rdx" || arg == "%rsp" || arg == "%rbp" || arg == "%rsi" || arg == "%rdi") {
                    o << "    movq " << arg << ", " << argRegs64[intArgCount] << "\n";
                } else {
                    o << "    movslq " << arg << ", " << argRegs64[intArgCount] << "\n";
                }
                intArgCount++;
            }
        }
    }
    // o << "    movl $" << floatArgCount << ", %eax\n";
    o << "    call " << funcLabel << "\n";
    if (dest != "%xmm0" && !dest.empty()) o << "    movsd %xmm0, " << dest << "\n";
}

void AsmGeneratorX86_64::Ret(std::ostream& o) {
    o << "    leave\n";
    o << "    ret\n";
}

void AsmGeneratorX86_64::gen_prologue(std::ostream& o) {
    int stackSpace = cfg->calculateRequiredStackSpace();
    string funcName = "main";
    if (!cfg->getBBs().empty()) funcName = cfg->getBBs()[0]->label;
    o << funcName << ":\n";
    o << "    pushq %rbp\n";
    o << "    movq %rsp, %rbp\n";
    o << "    subq $" << stackSpace << ", %rsp\n";
    if (funcName == "main") {
        o << "    xorl %eax, %eax\n"; // Ensure 0 exit code if main doesn't return int!
    }
}

void AsmGeneratorX86_64::gen_epilogue(std::ostream& o) {
    o << "    leave\n";
    o << "    ret\n";
}

void AsmGeneratorX86_64::gen_control_flow(std::ostream& o, BasicBlock* bb) {
    if (bb->exit_true == nullptr) {
        gen_epilogue(o);
    } else if (bb->exit_false == nullptr) {
        o << "    jmp " << bb->exit_true->label << "\n";
    } else {
        if (!bb->test_var_name.empty()) {
            string test_asm = var_to_asm(bb->test_var_name);
            o << "    movl " << test_asm << ", %eax\n";
        } else {
            cerr << "Internal Error: Conditional branch in " << bb->label
                 << " has no test_var_name." << endl;
            exit(1);
        }
        o << "    cmpl $0, %eax\n";
        o << "    je "  << bb->exit_false->label << "\n";
        o << "    jmp " << bb->exit_true->label << "\n";
    }
}
