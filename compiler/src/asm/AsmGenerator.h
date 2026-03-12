#pragma once

#include <iostream>
#include <string>
#include "../ir/Reg.h"
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

    virtual void visit(std::ostream& o, LdConstInstr&   instr) = 0;
    virtual void visit(std::ostream& o, CopyRegInstr&   instr) = 0;
    virtual void visit(std::ostream& o, StoreStackInstr& instr) = 0;
    virtual void visit(std::ostream& o, LoadStackInstr&  instr) = 0;
    virtual void visit(std::ostream& o, AddInstr&   instr) = 0;
    virtual void visit(std::ostream& o, SubInstr&        instr) = 0;
    virtual void visit(std::ostream& o, MulInstr&        instr) = 0;
    virtual void visit(std::ostream& o, DivInstr&        instr) = 0;
    virtual void visit(std::ostream& o, BitNotInstr&     instr) = 0;
    virtual void visit(std::ostream& o, BitAndInstr&     instr) = 0;
    virtual void visit(std::ostream& o, BitOrInstr&      instr) = 0;
    virtual void visit(std::ostream& o, BitXorInstr&     instr) = 0;
    virtual void visit(std::ostream& o, CmpEqInstr&      instr) = 0;
    virtual void visit(std::ostream& o, CmpLtInstr&      instr) = 0;
    virtual void visit(std::ostream& o, CmpLeInstr&      instr) = 0;
    virtual void visit(std::ostream& o, CallInstr&       instr) = 0;
    virtual void visit(std::ostream& o, RetInstr&        instr) = 0;

    //---------------- Helper Methods ----------------

    /** Convert architecture-agnostic Reg to a machine register name string */
    virtual std::string reg_to_asm(Reg r) = 0;

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
