#ifndef ASMGENERATORX86_64_H
#define ASMGENERATORX86_64_H

#include "../AsmGenerator.h"

//! x86_64 assembly generator
class AsmGeneratorX86_64 : public AsmGenerator {
public:
    explicit AsmGeneratorX86_64(CFG* cfg);
    ~AsmGeneratorX86_64() override = default;

    void gen_asm(std::ostream& o) override;
    void gen_asm_bb(std::ostream& o, BasicBlock* bb, bool isFirstBB = false) override;
    void gen_asm_instr(std::ostream& o, IRInstr* instr) override;

    //---------------- Visitor methods ----------------
    void visit(std::ostream& o, LdConstInstr&    instr) override;
    void visit(std::ostream& o, CopyRegInstr&    instr) override;
    void visit(std::ostream& o, StoreStackInstr& instr) override;
    void visit(std::ostream& o, LoadStackInstr&  instr) override;
    void visit(std::ostream& o, AddInstr&    instr) override;
    void visit(std::ostream& o, SubInstr&    instr) override;
    void visit(std::ostream& o, MulInstr&    instr) override;
    void visit(std::ostream& o, DivInstr&    instr) override;
    void visit(std::ostream& o, BitNotInstr& instr) override;
    void visit(std::ostream& o, BitAndInstr& instr) override;
    void visit(std::ostream& o, BitOrInstr&  instr) override;
    void visit(std::ostream& o, BitXorInstr& instr) override;
    void visit(std::ostream& o, CmpEqInstr&  instr) override;
    void visit(std::ostream& o, CmpLtInstr&  instr) override;
    void visit(std::ostream& o, CmpLeInstr&  instr) override;
    void visit(std::ostream& o, CallInstr&   instr) override;
    void visit(std::ostream& o, RetInstr&    instr) override;

    //---------------- Helpers ----------------
    /** Map Reg enum → 32-bit register name (e.g. Reg::R0 → "%eax") */
    std::string reg_to_asm(const RegParam& p) override;
    /** Map stack variable name → memory operand (e.g. "x" → "-4(%rbp)") */
    std::string var_to_asm(const std::string& varName) override;

    //---------------- Prologue / Epilogue ----------------
    void gen_prologue(std::ostream& o) override;
    void gen_epilogue(std::ostream& o) override;
    void gen_control_flow(std::ostream& o, BasicBlock* bb) override;
};

#endif // ASMGENERATORX86_64_H
