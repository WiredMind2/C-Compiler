#ifndef ASMGENERATORARM64_H
#define ASMGENERATORARM64_H

#include "../AsmGenerator.h"

//! ARM64 (Apple Silicon) assembly generator
class AsmGeneratorARM64 : public AsmGenerator {
public:
    explicit AsmGeneratorARM64(CFG* cfg);
    ~AsmGeneratorARM64() override = default;

    void gen_asm(std::ostream& o) override;
    void gen_asm_bb(std::ostream& o, BasicBlock* bb, bool isFirstBB = false) override;
    void gen_asm_instr(std::ostream& o, IRInstr* instr) override;

    //---------------- Visitor methods ----------------
    void visit(std::ostream& o, LdConstInstr&    instr) override;
    void ldConstInstrINT32(std::ostream& o, ConstParam src, std::string dest) override;
    void ldConstInstrFLOAT64(std::ostream& o, double src, std::string dest) override;

    void visit(std::ostream& o, CopyRegInstr&    instr) override;
    void CopyRegINT32(ostream& o, string src, string dest) override;
    void CopyRegFLOAT64(ostream& o, string src, string dest) override;

    void visit(std::ostream& o, StoreStackInstr& instr) override;
    void visit(std::ostream& o, LoadStackInstr&  instr) override;
    void LoadStackInstrINT32(ostream& o, string src, string dest) override;
    void LoadStackInstrFLOAT64(ostream& o, string src, string dest) override;

    void visit(std::ostream& o, AddInstr&    instr) override;
    void AddINT32(std::ostream& o, std::string lhs, std::string rhs, std::string dest) override;
    void AddFLOAT64(std::ostream& o, std::string lhs, std::string rhs, std::string dest) override;

    void visit(std::ostream& o, SubInstr&    instr) override;
    void visit(std::ostream& o, MulInstr&    instr) override;
    void MulINT32(ostream& o, string lhs, string rhs, string dest) override;
    void MulFLOAT64(ostream& o, string lhs, string rhs, string dest) override;

    void visit(std::ostream& o, DivInstr&    instr) override;
    void DivINT32(ostream& o, string lhs, string rhs, string dest) override;
    void DivFLOAT6(ostream& o, string lhs, string rhs, string dest) override;

    void visit(std::ostream& o, ModInstr&    instr) override;
    void visit(std::ostream& o, BitNotInstr& instr) override;
    void BitNot(ostream& o, string src, string dest) override;

    void visit(std::ostream& o, BitAndInstr& instr) override;
    void BitAnd(ostream& o, string lhs, string rhs, string dest) override;

    void visit(std::ostream& o, BitOrInstr&  instr) override;
    void visit(std::ostream& o, BitXorInstr& instr) override;
    void BitXor(ostream& o, string lhs, string rhs, string dest) override;
    void visit(std::ostream& o, CmpEqInstr&  instr) override;
    void CmpEqINT32(ostream& o, string lhs, string rhs, string dest) override;
    void CmpEqFLOAT64(ostream& o, string lhs, string rhs, string dest) override;

    void visit(std::ostream& o, CmpLtInstr&  instr) override;
    void visit(std::ostream& o, CmpLeInstr&  instr) override;
    void CmpLeINT32(ostream& o, string lhs, string rhs, string dest) override;
    void CmpLeFLOAT64(ostream& o, string lhs, string rhs, string dest) override;

    void visit(std::ostream& o, CmpGtInstr&  instr) override;
    void CmpGtINT32(ostream& o, string lhs, string rhs, string dest) override;
    void CmpGtFLOAT64(ostream& o, string lhs, string rhs, string dest) override;

    void visit(std::ostream& o, CmpGeInstr&  instr) override;
    void visit(std::ostream& o, LogicalAndInstr&  instr) override;
    void LogicalAnd(ostream& o, string lhs, string rhs, string dest) override;

    void visit(std::ostream& o, LogicalOrInstr&  instr) override;
    void LogicalOr(ostream& o, string lhs, string rhs, string dest) override;

    void visit(std::ostream& o, CallInstr&   instr) override;
    void Call(ostream& o, string funcLabel, vector<string> args, string dest) override;

    void visit(std::ostream& o, FToIInstr&   instr) override;
    void visit(std::ostream& o, RetInstr&    instr) override;
    void Ret(ostream& o, string src) override;

    //---------------- Helpers ----------------
    /** Map Reg enum → 32-bit register name (e.g. Reg::R0 → "w0") */
    std::string reg_to_asm(const RegParam& p) override;
    /** Map stack variable name → memory operand (e.g. "x" → "[fp, #-4]") */
    std::string var_to_asm(const std::string& varName) override;

    //---------------- Prologue / Epilogue ----------------
    void gen_prologue(std::ostream& o) override;
    void gen_epilogue(std::ostream& o) override;
    void gen_control_flow(std::ostream& o, BasicBlock* bb) override;
};

#endif // ASMGENERATORARM64_H
