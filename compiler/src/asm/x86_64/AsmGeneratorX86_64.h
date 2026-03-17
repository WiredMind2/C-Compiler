#ifndef ASMGENERATORX86_64_H
#define ASMGENERATORX86_64_H

#include "../AsmGenerator.h"

// ---------------------------------------------------------------------------
// x86_64 assembly generator
// ----------------------------------------------------------------------------

class AsmGeneratorX86_64 : public AsmGenerator {
public:
    explicit AsmGeneratorX86_64(CFG* cfg);
    ~AsmGeneratorX86_64() override = default;

    //---------------------------------------------------------------------------
    // Main generation functions
    //---------------------------------------------------------------------------
    void gen_asm(std::ostream& o) override;
    void gen_asm_bb(std::ostream& o, BasicBlock* bb, bool isFirstBB = false) override;
    void gen_asm_instr(std::ostream& o, IRInstr* instr) override;

    //---------------------------------------------------------------------------
    // Load Constants
    //---------------------------------------------------------------------------
    void ldConstInstrINT32(ostream& o, ConstParam reg_src, string reg_dest) override;
    void ldConstInstrINT64(ostream& o, ConstParam reg_src, string reg_dest) override;
    void ldConstInstrFLOAT64(ostream& o, double reg_src, string reg_dest) override;

    //---------------------------------------------------------------------------
    // Register Copy
    //---------------------------------------------------------------------------
    void CopyRegINT32(ostream& o, string src, string dest) override;
    void CopyRegFLOAT64(ostream& o, string src, string dest) override;

    //---------------------------------------------------------------------------
    // Stack Operations (Store/Load)
    //---------------------------------------------------------------------------
    void StoreStackInstrINT32(ostream& o, string src, string dest) override;
    void StoreStackInstrFLOAT64(ostream& o, string src, string dest) override;
    void LoadStackInstrINT32(ostream& o, string src, string dest) override;
    void LoadStackInstrFLOAT64(ostream& o, string src, string dest) override;

    //---------------------------------------------------------------------------
    // Arithmetic Operations
    //---------------------------------------------------------------------------
    void AddINT32(ostream& o, string lhs, string rhs, string dest) override;
    void AddFLOAT64(ostream& o, string lhs, string rhs, string dest) override;

    void SubINT32(ostream& o, string lhs, string rhs, string dest) override;
    void SubFLOAT64(ostream& o, string lhs, string rhs, string dest) override;
    void MulINT32(ostream& o, string lhs, string rhs, string dest) override;
    void MulFLOAT64(ostream& o, string lhs, string rhs, string dest) override;

    void DivINT32(ostream& o, string lhs, string rhs, string dest) override;
    void DivFLOAT64(ostream& o, string lhs, string rhs, string dest) override;

    void ModINT32(ostream& o, string lhs, string rhs, string dest) override;

    //---------------------------------------------------------------------------
    // Bitwise Operations
    //---------------------------------------------------------------------------
    void BitNot(ostream& o, string src, string dest) override;
    void BitAnd(ostream& o, string lhs, string rhs, string dest) override;
    void BitOr(ostream& o, string lhs, string rhs, string dest) override;
    void BitXor(ostream& o, string lhs, string rhs, string dest) override;

    //---------------------------------------------------------------------------
    // Comparison Operations
    //---------------------------------------------------------------------------
    void CmpEqFLOAT64(ostream& o, string lhs, string rhs, string dest) override;
    void CmpEqINT32(ostream& o, string lhs, string rhs, string dest) override;

    void CmpLtINT32(ostream& o, string lhs, string rhs, string dest) override;
    void CmpLtFLOAT64(ostream& o, string lhs, string rhs, string dest) override;
    void CmpLeINT32(ostream& o, string lhs, string rhs, string dest) override;
    void CmpLeFLOAT64(ostream& o, string lhs, string rhs, string dest) override;

    void CmpGtFLOAT64(ostream& o, string lhs, string rhs, string dest) override;
    void CmpGtINT32(ostream& o, string lhs, string rhs, string dest) override;

    void CmpGeINT32(ostream& o, string lhs, string rhs, string dest) override;

    //---------------------------------------------------------------------------
    // Logical Operations
    //---------------------------------------------------------------------------
    void LogicalAnd(ostream& o, string lhs, string rhs, string dest) override;
    void LogicalOr(ostream& o, string lhs, string rhs, string dest) override;

    //---------------------------------------------------------------------------
    // Function Call / Return
    //---------------------------------------------------------------------------
    void Call(ostream& o, string funcLabel, vector<string> args, string dest) override;
    void Ret(ostream& o) override;

    //---------------------------------------------------------------------------
    // Type Conversion
    //---------------------------------------------------------------------------
    void FToI(ostream& o, string src, string dest) override;
    void I32ToF64(ostream& o, string src, string dest) override;
    void I8ToI32(ostream& o, string src, string dest) override;

    //---------------------------------------------------------------------------
    // Helpers
    //---------------------------------------------------------------------------
    /** Map Reg enum → 32-bit register name (e.g. Reg::R0 → "%eax") */
    std::string reg_to_asm(const RegParam& p) override;
    /** Map stack variable name → memory operand (e.g. "x" → "-4(%rbp)") */
    std::string var_to_asm(const std::string& varName) override;

    //---------------------------------------------------------------------------
    // Prologue / Epilogue / Control flow
    //---------------------------------------------------------------------------
    void gen_prologue(std::ostream& o) override;
    void gen_epilogue(std::ostream& o) override;
    void gen_control_flow(std::ostream& o, BasicBlock* bb) override;
};

#endif // ASMGENERATORX86_64_H
