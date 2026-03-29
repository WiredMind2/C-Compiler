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

    //---------------- Visitor methods ----------------
    void visit(std::ostream& o, LdConstInstr&    instr) override;
    void visit(std::ostream& o, CopyRegInstr&    instr) override;
    void visit(std::ostream& o, StoreStackInstr& instr) override;
    void visit(std::ostream& o, LoadStackInstr&  instr) override;
    void visit(std::ostream& o, AddressOfSymbolInstr& instr) override;
    void visit(std::ostream& o, LoadPointerInstr& instr) override;
    void visit(std::ostream& o, StorePointerInstr& instr) override;
    void visit(std::ostream& o, AddInstr&    instr) override;
    void visit(std::ostream& o, SubInstr&    instr) override;
    void visit(std::ostream& o, MulInstr&    instr) override;
    void visit(std::ostream& o, DivInstr&    instr) override;
    void visit(std::ostream& o, ModInstr&    instr) override;
    void visit(std::ostream& o, BitNotInstr& instr) override;
    void visit(std::ostream& o, BitAndInstr& instr) override;
    void visit(std::ostream& o, BitOrInstr&  instr) override;
    void visit(std::ostream& o, BitXorInstr& instr) override;
    void visit(std::ostream& o, ShlInstr& instr) override;
    void visit(std::ostream& o, ShrInstr& instr) override;
    void visit(std::ostream& o, CmpEqInstr&  instr) override;
    void visit(std::ostream& o, CmpLtInstr&  instr) override;
    void visit(std::ostream& o, CmpLeInstr&  instr) override;
    void visit(std::ostream& o, CmpGtInstr&  instr) override;
    void visit(std::ostream& o, CmpGeInstr&  instr) override;
    void visit(std::ostream& o, LogicalAndInstr&  instr) override;
    void visit(std::ostream& o, LogicalOrInstr&  instr) override;
    void visit(std::ostream& o, CallInstr&   instr) override;
    void visit(std::ostream& o, F64ToI32Instr& instr) override;
    void visit(std::ostream& o, I32ToF64Instr& instr) override;
    void visit(std::ostream& o, FToIInstr&   instr) override;
    void visit(std::ostream& o, I8ToI32Instr& instr) override;
    void visit(std::ostream& o, I32ToI8Instr& instr) override;
    void visit(std::ostream& o, RetInstr&    instr) override;

    //---------------------------------------------------------------------------
    // Load Constants (typed helpers)
    //---------------------------------------------------------------------------
    void ldConstInstrINT8(std::ostream& o, ConstParam reg_src, const std::string& reg_dest) override;
    void ldConstInstrINT32(std::ostream& o, ConstParam reg_src, const std::string& reg_dest) override;
    void ldConstInstrINT64(std::ostream& o, ConstParam reg_src, const std::string& reg_dest) override;
    void ldConstInstrFLOAT64(std::ostream& o, double reg_src, const std::string& reg_dest) override;

    //---------------------------------------------------------------------------
    // Register Copy
    //---------------------------------------------------------------------------
    void CopyRegINT8(std::ostream& o, const std::string& src, const std::string& dest) override;
    void CopyRegINT32(std::ostream& o, const std::string& src, const std::string& dest) override;
    void CopyRegFLOAT64(std::ostream& o, const std::string& src, const std::string& dest) override;

    //---------------------------------------------------------------------------
    // Stack Operations (Store/Load)
    //---------------------------------------------------------------------------
    void StoreStackInstrINT8(std::ostream& o, const std::string& src, const std::string& dest) override;
    void StoreStackInstrINT32(std::ostream& o, const std::string& src, const std::string& dest) override;
    void StoreStackInstrFLOAT64(std::ostream& o, const std::string& src, const std::string& dest) override;
    void LoadStackInstrINT8(std::ostream& o, const std::string& src, const std::string& dest) override;
    void LoadStackInstrINT32(std::ostream& o, const std::string& src, const std::string& dest) override;
    void LoadStackInstrFLOAT64(std::ostream& o, const std::string& src, const std::string& dest) override;

    //---------------------------------------------------------------------------
    // Arithmetic Operations
    //---------------------------------------------------------------------------
    void AddINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;
    void AddFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;

    void SubINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;
    void SubFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;
    void MulINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;
    void MulFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;

    void DivINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;
    void DivFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;

    void ModINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;

    //---------------------------------------------------------------------------
    // Bitwise Operations
    //---------------------------------------------------------------------------
    void BitNot(std::ostream& o, const std::string& src, const std::string& dest) override;
    void BitAnd(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;
    void BitOr(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;
    void BitXor(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;

    //---------------------------------------------------------------------------
    // Comparison Operations
    //---------------------------------------------------------------------------
    void CmpEqFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;
    void CmpEqINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;

    void CmpLtINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;
    void CmpLtFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;
    void CmpLeINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;
    void CmpLeFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;

    void CmpGtFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;
    void CmpGtINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;

    void CmpGeINT32(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;
    void CmpGeFLOAT64(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;

    //---------------------------------------------------------------------------
    // Logical Operations
    //---------------------------------------------------------------------------
    void LogicalAnd(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;
    void LogicalOr(std::ostream& o, const std::string& lhs, const std::string& rhs, const std::string& dest) override;

    //---------------------------------------------------------------------------
    // Function Call / Return
    //---------------------------------------------------------------------------
    void CallWithINT32Return(std::ostream& o, const std::string& funcLabel, const std::vector<std::string>& args, const std::string& dest) override;
    void CallWithFLOAT64Return(std::ostream& o, const std::string& funcLabel, const std::vector<std::string>& args, const std::string& dest) override;
    void Ret(std::ostream& o) override;

    //---------------------------------------------------------------------------
    // Type Conversion
    //---------------------------------------------------------------------------
    void FToI(std::ostream& o, const std::string& src, const std::string& dest) override;
    void I32ToF64(std::ostream& o, const std::string& src, const std::string& dest) override;
    void I8ToI32(std::ostream& o, const std::string& src, const std::string& dest) override;
    void I32ToI8(std::ostream& o, const std::string& src, const std::string& dest) override;

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
