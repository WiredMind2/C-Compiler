#include "IRInstr.h"
#include "../asm/AsmGenerator.h"
#include <iostream>
#include "IR.h"

void IRInstr::gen_asm(ostream& o) {
    bb->cfg->asmGenerator->gen_asm_instr(o, this);
}

void LdConstInstr::accept(AsmGenerator& g, ostream& o) {
    string dest_register = dest.reg_to_asm();

    switch (type) {
        case IRType::INT32: {
            g.ldConstInstrINT32(o, val, dest_register);
            break;
        }
        case IRType::FLOAT64: {
            g.ldConstInstrFLOAT64(o, val.as_f64(), dest_register);
            break;
        }
        default: {
            throw std::runtime_error("Bad type");
        }
    }
}
void CopyRegInstr   ::accept(AsmGenerator& g, ostream& o) {
    if (src.type != dest.type) {
        throw std::runtime_error("Incoherent types");
    }

    string src_register = src.reg_to_asm();
    string dest_register = dest.reg_to_asm();
    switch (src.type) {
        case IRType::INT32: {
            g.CopyRegINT32(o, src_register, dest_register);
            break;
        }
        case IRType::INT64: {
            g.CopyRegFLOAT64(o, src_register, dest_register);
            break;
        }
        default: {
            throw std::runtime_error("Type not supported for addition");
        }
    }
}
void StoreStackInstr::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void LoadStackInstr ::accept(AsmGenerator& g, ostream& o) {
    string dest_register = dest.reg_to_asm();
    string variable_name = src.name;

    switch (type) {
        case IRType::INT32: {
            g.LoadStackInstrINT32(o, variable_name, dest_register);
            break;
        }
        case IRType::FLOAT64: {
            g.LoadStackInstrFLOAT64(o, variable_name, dest_register);
            break;
        }
        default: {
            throw std::runtime_error("Type not supported for addition");
        }
    }
}
void AddInstr       ::accept(AsmGenerator& g, ostream& o) {
    if (lhs.type != rhs.type) {
        throw std::runtime_error("Incoherent types");
    }

    string lhs_register = lhs.reg_to_asm();
    string rhs_register = rhs.reg_to_asm();
    string dest_register = dest.reg_to_asm();
    switch (lhs.type) {
        case IRType::INT32: {
            g.AddINT32(o, lhs_register, rhs_register, dest_register);
            break;
        }
        case IRType::FLOAT64: {
            g.AddFLOAT64(o, lhs_register, rhs_register, dest_register);
            break;
        }
        default: {
            throw std::runtime_error("Type not supported for addition");
        }
    }
}
void SubInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void MulInstr       ::accept(AsmGenerator& g, ostream& o) {
    if (lhs.type != rhs.type) {
        throw std::runtime_error("Incoherent types");
    }

    string lhs_register = lhs.reg_to_asm();
    string rhs_register = rhs.reg_to_asm();
    string dest_register = dest.reg_to_asm();
    switch (lhs.type) {
        case IRType::INT32: {
            g.MulINT32(o, lhs_register, rhs_register, dest_register);
            break;
        }
        case IRType::FLOAT64: {
            g.MulFLOAT64(o, lhs_register, rhs_register, dest_register);
            break;
        }
        default: {
            throw std::runtime_error("Type not supported for multiplication");
        }
    }
}
void DivInstr       ::accept(AsmGenerator& g, ostream& o) {
    if (lhs.type != rhs.type) {
        throw std::runtime_error("Incoherent types");
    }

    string lhs_register = lhs.reg_to_asm();
    string rhs_register = rhs.reg_to_asm();
    string dest_register = dest.reg_to_asm();
    switch (lhs.type) {
        case IRType::INT32: {
            g.DivINT32(o, lhs_register, rhs_register, dest_register);
            break;
        }
        case IRType::FLOAT64: {
            g.DivFLOAT6(o, lhs_register, rhs_register, dest_register);
            break;
        }
        default: {
            throw std::runtime_error("Type not supported for division");
        }
    }
}
void ModInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void BitNotInstr    ::accept(AsmGenerator& g, ostream& o) {
    string src_register = src.reg_to_asm();
    string dest_register = dest.reg_to_asm();
    switch (type) {
        case IRType::INT32: {
            g.BitNot(o, src_register, dest_register);
            break;
        }
        default: {
            throw std::runtime_error("Type not supported for bit not");
        }
    }
}
void BitAndInstr    ::accept(AsmGenerator& g, ostream& o) {
    if (lhs.type != rhs.type) {
        throw std::runtime_error("Incoherent types");
    }

    string lhs_register = lhs.reg_to_asm();
    string rhs_register = rhs.reg_to_asm();
    string dest_register = dest.reg_to_asm();
    switch (lhs.type) {
        case IRType::INT32: {
            g.BitAnd(o, lhs_register, rhs_register, dest_register);
            break;
        }
        default: {
            throw std::runtime_error("Type not supported for bit and");
        }
    }
}
void BitOrInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void BitXorInstr    ::accept(AsmGenerator& g, ostream& o) {
    if (lhs.type != rhs.type) {
        throw std::runtime_error("Incoherent types");
    }

    string lhs_register = lhs.reg_to_asm();
    string rhs_register = rhs.reg_to_asm();
    string dest_register = dest.reg_to_asm();
    switch (lhs.type) {
        case IRType::INT32: {
            g.BitXor(o, lhs_register, rhs_register, dest_register);
            break;
        }
        default: {
            throw std::runtime_error("Type not supported for bit xor");
        }
    }
}
void CmpEqInstr     ::accept(AsmGenerator& g, ostream& o) {
    if (lhs.type != rhs.type) {
        throw std::runtime_error("Incoherent types");
    }

    string lhs_register = lhs.reg_to_asm();
    string rhs_register = rhs.reg_to_asm();
    string dest_register = dest.reg_to_asm();
    switch (lhs.type) {
        case IRType::INT32: {
            g.CmpEqINT32(o, lhs_register, rhs_register, dest_register);
            break;
        }
        case IRType::FLOAT64: {
            g.CmpEqFLOAT64(o, lhs_register, rhs_register, dest_register);
            break;
        }
        default: {
            throw std::runtime_error("Type not supported for cmp eq");
        }
    }
}
void CmpLtInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void CmpLeInstr     ::accept(AsmGenerator& g, ostream& o) {
    if (lhs.type != rhs.type) {
        throw std::runtime_error("Incoherent types");
    }

    string lhs_register = lhs.reg_to_asm();
    string rhs_register = rhs.reg_to_asm();
    string dest_register = dest.reg_to_asm();
    switch (lhs.type) {
        case IRType::INT32: {
            g.CmpLeINT32(o, lhs_register, rhs_register, dest_register);
            break;
        }
        case IRType::FLOAT64: {
            g.CmpLeFLOAT64(o, lhs_register, rhs_register, dest_register);
            break;
        }
        default: {
            throw std::runtime_error("Type not supported for cmp le");
        }
    }
}
void CmpGtInstr     ::accept(AsmGenerator& g, ostream& o) {
    if (lhs.type != rhs.type) {
        throw std::runtime_error("Incoherent types");
    }

    string lhs_register = lhs.reg_to_asm();
    string rhs_register = rhs.reg_to_asm();
    string dest_register = dest.reg_to_asm();
    switch (lhs.type) {
        case IRType::INT32: {
            g.CmpGtINT32(o, lhs_register, rhs_register, dest_register);
            break;
        }
        case IRType::FLOAT64: {
            g.CmpGtFLOAT64(o, lhs_register, rhs_register, dest_register);
            break;
        }
        default: {
            throw std::runtime_error("Type not supported for cmp gt");
        }
    }
}
void CmpGeInstr     ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void LogicalAndInstr::accept(AsmGenerator& g, ostream& o) {
    if (lhs.type != rhs.type) {
        throw std::runtime_error("Incoherent types");
    }

    string lhs_register = lhs.reg_to_asm();
    string rhs_register = rhs.reg_to_asm();
    string dest_register = dest.reg_to_asm();
    switch (lhs.type) {
        case IRType::INT32: {
            g.LogicalAnd(o, lhs_register, rhs_register, dest_register);
            break;
        }
        default: {
            throw std::runtime_error("Type not supported for logical and");
        }
    }
}
void LogicalOrInstr ::accept(AsmGenerator& g, ostream& o) {
    if (lhs.type != rhs.type) {
        throw std::runtime_error("Incoherent types");
    }

    string lhs_register = lhs.reg_to_asm();
    string rhs_register = rhs.reg_to_asm();
    string dest_register = dest.reg_to_asm();
    switch (lhs.type) {
        case IRType::INT32: {
            g.LogicalOr(o, lhs_register, rhs_register, dest_register);
            break;
        }
        default: {
            throw std::runtime_error("Type not supported for logical or");
        }
    }
}
void CallInstr      ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void FToIInstr      ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
void RetInstr       ::accept(AsmGenerator& g, ostream& o) { g.visit(o, *this); }
