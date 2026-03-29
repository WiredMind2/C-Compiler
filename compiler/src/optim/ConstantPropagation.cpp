#include "ConstantPropagation.h"
#include "../ir/IR.h"

namespace optim {

namespace {

bool isSupportedConstType(IRType t) {
    return t == IRType::INT8 || t == IRType::INT32 || t == IRType::INT64
        || t == IRType::FLOAT32 || t == IRType::FLOAT64;
}

LdConstInstr* makeLdConstFromValue(BasicBlock* bb, Reg dest, const ConstParam& cst) {
    if (cst.type == IRType::FLOAT32) {
        return new LdConstInstr(bb, dest, cst.type, static_cast<double>(cst.as_f32()));
    }
    if (cst.type == IRType::FLOAT64) {
        return new LdConstInstr(bb, dest, cst.type, cst.as_f64());
    }
    return new LdConstInstr(bb, dest, cst.type, cst.raw_int());
}

void assignConst(std::map<Reg, ConstParam>& m, Reg key, const ConstParam& value) {
    m.erase(key);
    m.emplace(key, value);
}

void assignConst(std::map<std::string, ConstParam>& m, const std::string& key, const ConstParam& value) {
    m.erase(key);
    m.emplace(key, value);
}

}

bool ConstantPropagationPass::optimize(CFG* cfg) {
    if (!cfg) return false;

    bool modified = false;
    bool progress = true;
    while (progress) {
        progress = false;
        for (auto* bb : cfg->getBBs())
            if (optimizeBasicBlock(bb)) { progress = true; }
        if (progress) modified = true;
    }
    return modified;
}

bool ConstantPropagationPass::optimizeBasicBlock(BasicBlock* bb) {
    bool modified = false;
    auto& instrs = bb->instrs;

    // Map: stack variable name / register -> known compile-time constant value.
    std::map<std::string, ConstParam> constants;
    std::map<Reg, ConstParam> regConstants;

    for (size_t i = 0; i < instrs.size(); ++i) {
        auto* instr = instrs[i];

        if (auto* ldc = dynamic_cast<LdConstInstr*>(instr)) {
            if (isSupportedConstType(ldc->type)) {
                assignConst(regConstants, ldc->dest.reg, ldc->val);
            } else {
                regConstants.erase(ldc->dest.reg);
            }
            continue;
        }

        if (auto* cp = dynamic_cast<CopyRegInstr*>(instr)) {
            auto it = regConstants.find(cp->src.reg);
            if (it != regConstants.end() && it->second.type == cp->type) {
                const Reg destReg = cp->dest.reg;
                auto* newInstr = makeLdConstFromValue(bb, destReg, it->second);
                delete cp;
                instrs[i] = newInstr;
                assignConst(regConstants, destReg, it->second);
                modified = true;
                continue;
            }
            regConstants.erase(cp->dest.reg);
            continue;
        }

        // store_stack var, reg
        //   → if the previous instruction was ldconst reg, val, record the constant
        //   → otherwise invalidate the variable
        if (auto* st = dynamic_cast<StoreStackInstr*>(instr)) {
            bool foundConst = false;
            auto rit = regConstants.find(st->src.reg);
            if (rit != regConstants.end() && rit->second.type == st->type) {
                assignConst(constants, st->dest.name, rit->second);
                foundConst = true;
            }
            if (!foundConst) {
                constants.erase(st->dest.name);
            }
            continue;
        }

        // load_stack reg, var
        //   → if var is a known constant, replace the load_stack with ldconst reg, val
        if (auto* ld = dynamic_cast<LoadStackInstr*>(instr)) {
            auto it = constants.find(ld->src.name);
            if (it != constants.end() && it->second.type == ld->type) {
                auto* newInstr = makeLdConstFromValue(bb, ld->dest.reg, it->second);
                delete ld;
                instrs[i] = newInstr;
                assignConst(regConstants, newInstr->dest.reg, it->second);
                modified = true;
            } else {
                regConstants.erase(ld->dest.reg);
            }
            continue;
        }

        // Pointer/address operations are aliasing barriers for stack constants.
        if (auto* addr = dynamic_cast<AddressOfSymbolInstr*>(instr)) {
            regConstants.erase(addr->dest.reg);
            continue;
        }
        if (auto* lp = dynamic_cast<LoadPointerInstr*>(instr)) {
            regConstants.erase(lp->dest.reg);
            continue;
        }
        if (dynamic_cast<StorePointerInstr*>(instr)) {
            constants.clear();
            regConstants.clear();
            continue;
        }

        if (auto* add = dynamic_cast<AddInstr*>(instr)) { regConstants.erase(add->dest.reg); continue; }
        if (auto* sub = dynamic_cast<SubInstr*>(instr)) { regConstants.erase(sub->dest.reg); continue; }
        if (auto* mul = dynamic_cast<MulInstr*>(instr)) { regConstants.erase(mul->dest.reg); continue; }
        if (auto* div = dynamic_cast<DivInstr*>(instr)) { regConstants.erase(div->dest.reg); continue; }
        if (auto* mod = dynamic_cast<ModInstr*>(instr)) { regConstants.erase(mod->dest.reg); continue; }
        if (auto* bn  = dynamic_cast<BitNotInstr*>(instr)) { regConstants.erase(bn->dest.reg); continue; }
        if (auto* ba  = dynamic_cast<BitAndInstr*>(instr)) { regConstants.erase(ba->dest.reg); continue; }
        if (auto* bo  = dynamic_cast<BitOrInstr*>(instr)) { regConstants.erase(bo->dest.reg); continue; }
        if (auto* bx  = dynamic_cast<BitXorInstr*>(instr)) { regConstants.erase(bx->dest.reg); continue; }
        if (auto* ce  = dynamic_cast<CmpEqInstr*>(instr)) { regConstants.erase(ce->dest.reg); continue; }
        if (auto* clt = dynamic_cast<CmpLtInstr*>(instr)) { regConstants.erase(clt->dest.reg); continue; }
        if (auto* cle = dynamic_cast<CmpLeInstr*>(instr)) { regConstants.erase(cle->dest.reg); continue; }
        if (auto* cgt = dynamic_cast<CmpGtInstr*>(instr)) { regConstants.erase(cgt->dest.reg); continue; }
        if (auto* cge = dynamic_cast<CmpGeInstr*>(instr)) { regConstants.erase(cge->dest.reg); continue; }
        if (auto* la  = dynamic_cast<LogicalAndInstr*>(instr)) { regConstants.erase(la->dest.reg); continue; }
        if (auto* lo  = dynamic_cast<LogicalOrInstr*>(instr)) { regConstants.erase(lo->dest.reg); continue; }
        if (auto* fti = dynamic_cast<F64ToI32Instr*>(instr)) { regConstants.erase(fti->dest.reg); continue; }
        if (auto* itf = dynamic_cast<I32ToF64Instr*>(instr)) { regConstants.erase(itf->dest.reg); continue; }
        if (auto* fi  = dynamic_cast<FToIInstr*>(instr)) { regConstants.erase(fi->dest.reg); continue; }
        if (auto* i8i = dynamic_cast<I8ToI32Instr*>(instr)) { regConstants.erase(i8i->dest.reg); continue; }
        if (auto* i32i8 = dynamic_cast<I32ToI8Instr*>(instr)) { regConstants.erase(i32i8->dest.reg); continue; }
        if (auto* call = dynamic_cast<CallInstr*>(instr)) {
            constants.clear();
            regConstants.clear();
            regConstants.erase(call->dest.reg);
            continue;
        }

        // Unknown instruction shape: clear register facts conservatively.
        regConstants.clear();
    }


    return modified;
}

} // namespace optim
