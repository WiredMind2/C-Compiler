#include "IRInstr.h"
#include "../asm/AsmGenerator.h"
#include "IR.h"

void IRInstr::gen_asm(ostream& o) {
    bb->cfg->asmGenerator->gen_asm_instr(o, this);
}
void LdConstInstr   ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CopyRegInstr   ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void StoreStackInstr::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void LoadStackInstr ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void AddressOfSymbolInstr::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void LoadPointerInstr::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void StorePointerInstr::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void AddInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void SubInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void MulInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void DivInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void ModInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void BitNotInstr    ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void BitAndInstr    ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void BitOrInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void BitXorInstr    ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CmpEqInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CmpLtInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CmpLeInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CmpGtInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CmpGeInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void LogicalAndInstr::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void LogicalOrInstr ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CallInstr      ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void FToIInstr      ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void RetInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void F64ToI32Instr  ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void I32ToF64Instr  ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void I8ToI32Instr   ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void I32ToI8Instr   ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
