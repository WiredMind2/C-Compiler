#include "IRInstr.h"
#include "../asm/AsmGenerator.h"
#include <iostream>
#include "IR.h"

void IRInstr::gen_asm(ostream& o) {
    bb->cfg->asmGenerator->gen_asm_instr(o, this);
}

void LdConstInstr   ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CopyRegInstr   ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void StoreStackInstr::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void LoadStackInstr ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void AddInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void SubInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void MulInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void DivInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void BitNotInstr    ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void BitAndInstr    ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void BitOrInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void BitXorInstr    ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CmpEqInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CmpLtInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CmpLeInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CallInstr      ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void RetInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
