#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "../ir/IRInstr.h"

class CFG;
class BasicBlock;
class IRInstr;

enum class TargetArch { X86_64, ARM64 };

//! Abstract base class for assembly generation.
/*!
 * Implements the Visitor side of the double-dispatch pattern.
 * Each concrete generator (x86_64, ARM64) overrides one visit()
 * method per IRInstr subclass instead of a monolithic switch.
 */
class AsmGenerator {
   public:
    virtual ~AsmGenerator() = default;

    //! Generate assembly for the entire CFG
    virtual void gen_asm(std::ostream& o) = 0;

    //! Generate assembly for a single basic block
    virtual void gen_asm_bb(std::ostream& o, BasicBlock* bb, bool isFirstBB = false) = 0;

    //! Entry point called by IRInstr::gen_asm() — delegates to accept()
    virtual void gen_asm_instr(std::ostream& o, IRInstr* instr) = 0;

    //---------------- Visitor methods (one per IRInstr subclass) ----------------

    virtual void visit(std::ostream& o, LdStringInstr& instr) = 0;
    virtual void visit(std::ostream& o, LdConstInstr& instr) = 0;
    virtual void visit(std::ostream& o, CopyRegInstr& instr) = 0;
    virtual void visit(std::ostream& o, StoreStackInstr& instr) = 0;
    virtual void visit(std::ostream& o, LoadStackInstr& instr) = 0;
    virtual void visit(std::ostream& o, AddressOfSymbolInstr& instr) = 0;
    virtual void visit(std::ostream& o, LoadPointerInstr& instr) = 0;
    virtual void visit(std::ostream& o, StorePointerInstr& instr) = 0;
    virtual void visit(std::ostream& o, AddInstr& instr) = 0;
    virtual void visit(std::ostream& o, SubInstr& instr) = 0;
    virtual void visit(std::ostream& o, MulInstr& instr) = 0;
    virtual void visit(std::ostream& o, DivInstr& instr) = 0;
    virtual void visit(std::ostream& o, ModInstr& instr) = 0;
    virtual void visit(std::ostream& o, BitNotInstr& instr) = 0;
    virtual void visit(std::ostream& o, BitAndInstr& instr) = 0;
    virtual void visit(std::ostream& o, BitOrInstr& instr) = 0;
    virtual void visit(std::ostream& o, BitXorInstr& instr) = 0;
    virtual void visit(std::ostream& o, ShlInstr& instr) = 0;
    virtual void visit(std::ostream& o, ShrInstr& instr) = 0;
    virtual void visit(std::ostream& o, CmpEqInstr& instr) = 0;
    virtual void visit(std::ostream& o, CmpLtInstr& instr) = 0;
    virtual void visit(std::ostream& o, CmpLeInstr& instr) = 0;
    virtual void visit(std::ostream& o, CmpGtInstr& instr) = 0;
    virtual void visit(std::ostream& o, CmpGeInstr& instr) = 0;
    virtual void visit(std::ostream& o, LogicalAndInstr& instr) = 0;
    virtual void visit(std::ostream& o, LogicalOrInstr& instr) = 0;
    virtual void visit(std::ostream& o, CallInstr& instr) = 0;
    virtual void visit(std::ostream& o, F64ToI32Instr& instr) = 0;
    virtual void visit(std::ostream& o, I32ToF64Instr& instr) = 0;
    virtual void visit(std::ostream& o, FToIInstr& instr) = 0;
    virtual void visit(std::ostream& o, I8ToI32Instr& instr) = 0;
    virtual void visit(std::ostream& o, I32ToI8Instr& instr) = 0;
    virtual void visit(std::ostream& o, RetInstr& instr) = 0;

    //---------------- Optional typed helper methods ----------------

    virtual void ldConstInstrINT8(std::ostream& o, ConstParam src, const std::string& dest) {}
    virtual void ldConstInstrINT32(std::ostream& o, ConstParam src, const std::string& dest) {}
    virtual void ldConstInstrINT64(std::ostream& o, ConstParam src, const std::string& dest) {}
    virtual void ldConstInstrFLOAT64(std::ostream& o, double src, const std::string& dest) {}

    virtual void CopyRegINT8(std::ostream& o, const std::string& src, const std::string& dest) {}
    virtual void CopyRegINT32(std::ostream& o, const std::string& src, const std::string& dest) {}
    virtual void CopyRegFLOAT64(std::ostream& o, const std::string& src, const std::string& dest) {}

    virtual void StoreStackInstrINT8(std::ostream& o, const std::string& src, const std::string& dest) {}
    virtual void StoreStackInstrINT32(std::ostream& o, const std::string& src, const std::string& dest) {}
    virtual void StoreStackInstrFLOAT64(std::ostream& o, const std::string& src, const std::string& dest) {}

    virtual void LoadStackInstrINT8(std::ostream& o, const std::string& src, const std::string& dest) {}
    virtual void LoadStackInstrINT32(std::ostream& o, const std::string& src, const std::string& dest) {}
    virtual void LoadStackInstrFLOAT64(std::ostream& o, const std::string& src, const std::string& dest) {}

    virtual void AddINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}
    virtual void AddFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}

    virtual void SubINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}
    virtual void SubFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}

    virtual void MulINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}
    virtual void MulFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}

    virtual void DivINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}
    virtual void DivFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}

    virtual void ModINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}
    virtual void BitNot(std::ostream& o, const std::string& src, const std::string& dest) {}

    virtual void BitAnd(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}
    virtual void BitOr(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}
    virtual void BitXor(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}

    virtual void CmpEqFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}
    virtual void CmpEqINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}

    virtual void CmpLtINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}
    virtual void CmpLtFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}
    virtual void CmpLeINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}
    virtual void CmpLeFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}

    virtual void CmpGtFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}
    virtual void CmpGtINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}

    virtual void CmpGeINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}
    virtual void CmpGeFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}

    virtual void LogicalAnd(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}
    virtual void LogicalOr(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) {}

    virtual void CallWithINT32Return(std::ostream& o, const std::string& funcLabel, const std::vector<std::string>& args, const std::string& dest) {}
    virtual void CallWithFLOAT64Return(std::ostream& o, const std::string& funcLabel, const std::vector<std::string>& args, const std::string& dest) {
    }

    virtual void FToI(std::ostream& o, const std::string& src, const std::string& dest) {}
    virtual void I32ToF64(std::ostream& o, const std::string& src, const std::string& dest) {}
    virtual void I8ToI32(std::ostream& o, const std::string& src, const std::string& dest) {}
    virtual void I32ToI8(std::ostream& o, const std::string& src, const std::string& dest) {}

    virtual void Ret(std::ostream& o) {}

    //---------------- Helper Methods ----------------

    /** Convert a RegParam (reg + IRType) to a machine register name string */
    virtual std::string reg_to_asm(const RegParam& p) = 0;

    /** Convert a stack variable name to its memory operand string */
    virtual std::string var_to_asm(const std::string& varName) = 0;

    //---------------- Prologue / Epilogue ----------------

    virtual void gen_prologue(std::ostream& o) = 0;
    virtual void gen_epilogue(std::ostream& o) = 0;
    virtual void gen_control_flow(std::ostream& o, BasicBlock* bb) = 0;

   protected:
    CFG* cfg;
    int labelCount;
    explicit AsmGenerator(CFG* cfg) : cfg(cfg), labelCount(0) {}

    int getNextLabel() { return labelCount++; }
};
