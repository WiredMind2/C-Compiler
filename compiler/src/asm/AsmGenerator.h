#pragma once

#include <iostream>
#include <string>
#include "../ir/IRInstr.h"

class CFG;
class BasicBlock;
class IRInstr;


enum class TargetArch {
    X86_64,
    ARM64
};

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

    virtual void visit(std::ostream& o, LdConstInstr&    instr) = 0;
    virtual void ldConstInstrINT8(std::ostream& o, std::string src, std::string dest) = 0;
    virtual void ldConstInstrINT32(std::ostream& o, std::string src, std::string dest) = 0;
    virtual void ldConstInstrFLOAT64(std::ostream& o, std::string src, std::string dest) = 0;

    virtual void visit(std::ostream& o, CopyRegInstr&    instr) = 0;
    virtual void CopyRegINT8(std::ostream& o, std::string src, std::string dest) = 0;
    virtual void CopyRegINT32(std::ostream& o, std::string src, std::string dest) = 0;
    virtual void CopyRegFLOAT64(std::ostream& o, std::string src, std::string dest) = 0;

    virtual void visit(std::ostream& o, StoreStackInstr& instr) = 0;
    virtual void visit(std::ostream& o, LoadStackInstr&  instr) = 0;

    virtual void LoadStackInstrINT8(ostream& o, string src, string dest) = 0;
    virtual void LoadStackInstrINT32(ostream& o, string src, string dest) = 0;
    virtual void LoadStackInstrFLOAT64(ostream& o, string src, string dest) = 0;

    virtual void visit(std::ostream& o, AddInstr&        instr) = 0;
    virtual void AddINT32(std::ostream& o, std::string lhs, std::string rhs, std::string dest) = 0;
    virtual void AddFLOAT64(std::ostream& o, std::string lhs, std::string rhs, std::string dest) = 0;
    virtual void AddINT32_by_FLOAT64(std::ostream& o, std::string lhs, std::string rhs, std::string dest) = 0;
    virtual void AddFLOAT64_by_INT32(std::ostream& o, std::string lhs, std::string rhs, std::string dest) = 0;  

    virtual void visit(std::ostream& o, SubInstr&        instr) = 0;
    virtual void visit(std::ostream& o, MulInstr&        instr) = 0;
    virtual void MulINT32_by_INT32(ostream& o, string lhs, string rhs, string dest) = 0;
    virtual void MulFLOAT64_by_FLOAT64(ostream& o, string lhs, string rhs, string dest) = 0;
    virtual void MulINT32_by_FLOAT64(ostream& o, string lhs, string rhs, string dest) = 0;
    virtual void MulFLOAT64_by_INT32(ostream& o, string lhs, string rhs, string dest) = 0;

    virtual void visit(std::ostream& o, DivInstr&        instr) = 0;
    virtual void DivINT32_by_INT32(ostream& o, string lhs, string rhs, string dest) = 0;
    virtual void DivFLOAT64_by_FLOAT64(ostream& o, string lhs, string rhs, string dest) = 0;
    virtual void DivINT32_by_FLOAT64(ostream& o, string lhs, string rhs, string dest) = 0;
    virtual void DivFLOAT64_by_INT32(ostream& o, string lhs, string rhs, string dest) = 0;  

    virtual void visit(std::ostream& o, ModInstr&        instr) = 0;
    virtual void visit(std::ostream& o, BitNotInstr&     instr) = 0;
    virtual void BitNot(ostream& o, string src, string dest) = 0;

    virtual void visit(std::ostream& o, BitAndInstr&     instr) = 0;
    virtual void BitAnd(ostream& o, string lhs, string rhs, string dest) = 0;

    virtual void visit(std::ostream& o, BitOrInstr&      instr) = 0;
    virtual void visit(std::ostream& o, BitXorInstr&     instr) = 0;
    virtual void BitXor(ostream& o, string lhs, string rhs, string dest) = 0;

    virtual void visit(std::ostream& o, CmpEqInstr&      instr) = 0;
    virtual void CmpEqFLOAT64_with_INT32(ostream& o, string lhs, string rhs, string dest) = 0;
    virtual void CmpEqINT32_with_FLOAT64(ostream& o, string lhs, string rhs, string dest) = 0;
    virtual void CmpEqFLOAT64_with_FLOAT64(ostream& o, string lhs, string rhs, string dest) = 0;
    virtual void CmpEqINT32_with_INT32(ostream& o, string lhs, string rhs, string dest) = 0;

    virtual void visit(std::ostream& o, CmpLtInstr&      instr) = 0;
    virtual void visit(std::ostream& o, CmpLeInstr&      instr) = 0;
    virtual void CmpLeINT32_with_INT32(ostream& o, string lhs, string rhs, string dest) = 0;
    virtual void CmpLeFLOAT64_with_FLOAT64(ostream& o, string lhs, string rhs, string dest) = 0;
    virtual void CmpLeINT32_with_FLOAT64(ostream& o, string lhs, string rhs, string dest) = 0;
    virtual void CmpLeFLOAT64_with_INT32(ostream& o, string lhs, string rhs, string dest) = 0;

    virtual void visit(std::ostream& o, CmpGtInstr&      instr) = 0;
    virtual void CmpGtFLOAT64_with_INT32(ostream& o, string lhs, string rhs, string dest) = 0;
    virtual void CmpGtINT32_with_FLOAT64(ostream& o, string lhs, string rhs, string dest) = 0;
    virtual void CmpGtFLOAT64_with_FLOAT64(ostream& o, string lhs, string rhs, string dest) = 0;
    virtual void CmpGtINT32_with_INT32(ostream& o, string lhs, string rhs, string dest) = 0;

    virtual void visit(std::ostream& o, CmpGeInstr&      instr) = 0;
    virtual void visit(std::ostream& o, LogicalAndInstr& instr) = 0;
    virtual void LogicalAnd(ostream& o, string lhs, string rhs, string dest) = 0;

    virtual void visit(std::ostream& o, LogicalOrInstr&  instr) = 0;
    virtual void LogicalOr(ostream& o, string lhs, string rhs, string dest) = 0;

    virtual void visit(std::ostream& o, CallInstr&       instr) = 0;
    virtual void Call(ostream& o, string funcLabel, vector<string> args, string dest) = 0;

    virtual void visit(std::ostream& o, FToIInstr&       instr) = 0;
    virtual void visit(std::ostream& o, RetInstr&        instr) = 0;
    virtual void Ret(ostream& o, string src) = 0;

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
    explicit AsmGenerator(CFG* cfg) : cfg(cfg) {}
};
