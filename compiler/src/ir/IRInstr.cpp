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
void AddInstr       ::accept(AsmGenerator& g, ostream& o) {
    if (lhs.type == rhs.type) {
        string lhs_register = lhs.reg_to_asm();
        string rhs_register = rhs.reg_to_asm();
        string dest_register = dest.reg_to_asm();
        switch (lhs.type) {
            case IRType::INT32: {
                g.addINT32(o, lhs_register, rhs_register, dest_register);
                break;
            }
            case IRType::INT64: {
                g.addFLOAT64(o, lhs_register, rhs_register, dest_register);
                break;
            }
            default: {
                std::cerr << "Bad addition";
                exit(1);
            }
        }
    } else {
        exit(1);
        std::cerr << "Bad types";
    }
}
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
