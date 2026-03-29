#include "DeadRegDefElimination.h"

#include "../ir/IR.h"

namespace optim {
namespace {

bool instrUsesReg(const IRInstr* instr, Reg reg) {
    if (dynamic_cast<const LdConstInstr*>(instr)) return false;
    if (dynamic_cast<const LoadStackInstr*>(instr)) return false;
    if (auto* cp = dynamic_cast<const CopyRegInstr*>(instr)) return cp->src.reg == reg;
    if (auto* st = dynamic_cast<const StoreStackInstr*>(instr)) return st->src.reg == reg;
    if (auto* add = dynamic_cast<const AddInstr*>(instr)) return add->lhs.reg == reg || add->rhs.reg == reg;
    if (auto* sub = dynamic_cast<const SubInstr*>(instr)) return sub->lhs.reg == reg || sub->rhs.reg == reg;
    if (auto* mul = dynamic_cast<const MulInstr*>(instr)) return mul->lhs.reg == reg || mul->rhs.reg == reg;
    if (auto* div = dynamic_cast<const DivInstr*>(instr)) return div->lhs.reg == reg || div->rhs.reg == reg;
    if (auto* mod = dynamic_cast<const ModInstr*>(instr)) return mod->lhs.reg == reg || mod->rhs.reg == reg;
    if (auto* bn = dynamic_cast<const BitNotInstr*>(instr)) return bn->src.reg == reg;
    if (auto* ba = dynamic_cast<const BitAndInstr*>(instr)) return ba->lhs.reg == reg || ba->rhs.reg == reg;
    if (auto* bo = dynamic_cast<const BitOrInstr*>(instr)) return bo->lhs.reg == reg || bo->rhs.reg == reg;
    if (auto* bx = dynamic_cast<const BitXorInstr*>(instr)) return bx->lhs.reg == reg || bx->rhs.reg == reg;
    if (auto* ce = dynamic_cast<const CmpEqInstr*>(instr)) return ce->lhs.reg == reg || ce->rhs.reg == reg;
    if (auto* clt = dynamic_cast<const CmpLtInstr*>(instr)) return clt->lhs.reg == reg || clt->rhs.reg == reg;
    if (auto* cle = dynamic_cast<const CmpLeInstr*>(instr)) return cle->lhs.reg == reg || cle->rhs.reg == reg;
    if (auto* cgt = dynamic_cast<const CmpGtInstr*>(instr)) return cgt->lhs.reg == reg || cgt->rhs.reg == reg;
    if (auto* cge = dynamic_cast<const CmpGeInstr*>(instr)) return cge->lhs.reg == reg || cge->rhs.reg == reg;
    if (auto* la = dynamic_cast<const LogicalAndInstr*>(instr)) return la->lhs.reg == reg || la->rhs.reg == reg;
    if (auto* lo = dynamic_cast<const LogicalOrInstr*>(instr)) return lo->lhs.reg == reg || lo->rhs.reg == reg;
    if (auto* fti = dynamic_cast<const F64ToI32Instr*>(instr)) return fti->src.reg == reg;
    if (auto* itf = dynamic_cast<const I32ToF64Instr*>(instr)) return itf->src.reg == reg;
    if (auto* fi = dynamic_cast<const FToIInstr*>(instr)) return fi->src.reg == reg;
    if (auto* i8i = dynamic_cast<const I8ToI32Instr*>(instr)) return i8i->src.reg == reg;
    if (auto* i32i8 = dynamic_cast<const I32ToI8Instr*>(instr)) return i32i8->src.reg == reg;
    if (auto* call = dynamic_cast<const CallInstr*>(instr)) {
        for (const auto& arg : call->args) {
            if (arg.reg == reg) return true;
        }
        return false;
    }
    if (dynamic_cast<const RetInstr*>(instr)) {
        return reg == Reg::RET;
    }
    // Unknown shape: conservative, keep candidate alive.
    return true;
}

bool instrDefinesReg(const IRInstr* instr, Reg reg) {
    if (auto* ldc = dynamic_cast<const LdConstInstr*>(instr)) return ldc->dest.reg == reg;
    if (auto* cp = dynamic_cast<const CopyRegInstr*>(instr)) return cp->dest.reg == reg;
    if (auto* ld = dynamic_cast<const LoadStackInstr*>(instr)) return ld->dest.reg == reg;
    if (auto* add = dynamic_cast<const AddInstr*>(instr)) return add->dest.reg == reg;
    if (auto* sub = dynamic_cast<const SubInstr*>(instr)) return sub->dest.reg == reg;
    if (auto* mul = dynamic_cast<const MulInstr*>(instr)) return mul->dest.reg == reg;
    if (auto* div = dynamic_cast<const DivInstr*>(instr)) return div->dest.reg == reg;
    if (auto* mod = dynamic_cast<const ModInstr*>(instr)) return mod->dest.reg == reg;
    if (auto* bn = dynamic_cast<const BitNotInstr*>(instr)) return bn->dest.reg == reg;
    if (auto* ba = dynamic_cast<const BitAndInstr*>(instr)) return ba->dest.reg == reg;
    if (auto* bo = dynamic_cast<const BitOrInstr*>(instr)) return bo->dest.reg == reg;
    if (auto* bx = dynamic_cast<const BitXorInstr*>(instr)) return bx->dest.reg == reg;
    if (auto* ce = dynamic_cast<const CmpEqInstr*>(instr)) return ce->dest.reg == reg;
    if (auto* clt = dynamic_cast<const CmpLtInstr*>(instr)) return clt->dest.reg == reg;
    if (auto* cle = dynamic_cast<const CmpLeInstr*>(instr)) return cle->dest.reg == reg;
    if (auto* cgt = dynamic_cast<const CmpGtInstr*>(instr)) return cgt->dest.reg == reg;
    if (auto* cge = dynamic_cast<const CmpGeInstr*>(instr)) return cge->dest.reg == reg;
    if (auto* la = dynamic_cast<const LogicalAndInstr*>(instr)) return la->dest.reg == reg;
    if (auto* lo = dynamic_cast<const LogicalOrInstr*>(instr)) return lo->dest.reg == reg;
    if (auto* fti = dynamic_cast<const F64ToI32Instr*>(instr)) return fti->dest.reg == reg;
    if (auto* itf = dynamic_cast<const I32ToF64Instr*>(instr)) return itf->dest.reg == reg;
    if (auto* fi = dynamic_cast<const FToIInstr*>(instr)) return fi->dest.reg == reg;
    if (auto* i8i = dynamic_cast<const I8ToI32Instr*>(instr)) return i8i->dest.reg == reg;
    if (auto* i32i8 = dynamic_cast<const I32ToI8Instr*>(instr)) return i32i8->dest.reg == reg;
    if (dynamic_cast<const CallInstr*>(instr)) {
        // Conservative clobber barrier.
        return true;
    }
    return false;
}

} // namespace

bool DeadRegDefEliminationPass::optimize(CFG* cfg) {
    if (!cfg) return false;

    bool modified = false;
    bool progress = true;

    while (progress) {
        progress = false;
        for (auto* bb : cfg->getBBs()) {
            if (optimizeBasicBlock(bb)) {
                progress = true;
                modified = true;
            }
        }
    }

    return modified;
}

bool DeadRegDefEliminationPass::optimizeBasicBlock(BasicBlock* bb) {
    bool modified = false;
    auto& instrs = bb->instrs;

    for (size_t i = 0; i < instrs.size();) {
        Reg defReg = Reg::W0;
        bool candidate = false;

        if (auto* ldc = dynamic_cast<LdConstInstr*>(instrs[i])) {
            defReg = ldc->dest.reg;
            candidate = true;
        } else if (auto* cp = dynamic_cast<CopyRegInstr*>(instrs[i])) {
            defReg = cp->dest.reg;
            candidate = true;
        }

        if (!candidate) {
            ++i;
            continue;
        }

        bool usedBeforeRedef = false;
        for (size_t j = i + 1; j < instrs.size(); ++j) {
            if (instrUsesReg(instrs[j], defReg)) {
                usedBeforeRedef = true;
                break;
            }
            if (instrDefinesReg(instrs[j], defReg)) {
                break;
            }
        }

        if (!usedBeforeRedef) {
            delete instrs[i];
            instrs.erase(instrs.begin() + i, instrs.begin() + i + 1);
            modified = true;
            continue;
        }

        ++i;
    }

    return modified;
}

} // namespace optim

