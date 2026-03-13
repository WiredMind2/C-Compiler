#include "AsmGeneratorX86_64.h"
#include "../../IR.h"

AsmGeneratorX86_64::AsmGeneratorX86_64(CFG* cfg) : AsmGenerator(cfg) {}

void AsmGeneratorX86_64::gen_asm(ostream& o) {
    for (auto bb : cfg->getBBs()) {
        bool isFunction = false;
        for (const auto& func : cfg->get_functions()) {
            if (bb->label == func.name) {
                isFunction = true;
                break;
            }
        }
        
        if (isFunction) {
            o << ".globl " << bb->label << "\n";
            o << bb->label << ":\n";
            o << "    pushq %rbp\n";
            o << "    movq %rsp, %rbp\n";
            int stackSize = cfg->calculateRequiredStackSpace();
            if (stackSize > 0) {
                o << "    subq $" << stackSize << ", %rsp\n";
            }
        } else {
            o << bb->label << ":\n";
        }

        cfg->current_bb = bb;
        bb->gen_asm(o);
    }
}

void AsmGeneratorX86_64::gen_asm_bb(ostream& o, BasicBlock* bb, bool isFirstBB) {
    cfg->current_bb = bb;
    bb->gen_asm(o);
}

void AsmGeneratorX86_64::gen_asm_instr(ostream& o, IRInstr* instr) {
    if (instr->op == IRInstr::ret) {
        o << "    leave\n";
        o << "    ret\n";
        return;
    }
    
    switch (instr->op) {
        case IRInstr::ldconst:
            gen_ldconst(o, instr->params);
            break;
        case IRInstr::copy:
            gen_copy(o, instr->params);
            break;
        case IRInstr::add:
            gen_add(o, instr->params);
            break;
        case IRInstr::sub:
            gen_sub(o, instr->params);
            break;
        case IRInstr::mul:
            gen_mul(o, instr->params);
            break;
        case IRInstr::div:
            gen_div(o, instr->params);
            break;
        case IRInstr::call:
            gen_call(o, instr->params);
            break;
        case IRInstr::cmp_eq:
            gen_cmp_eq(o, instr->params);
            break;
        case IRInstr::cmp_ne:
            gen_cmp_ne(o, instr->params);
            break;
        case IRInstr::cmp_lt:
            gen_cmp_lt(o, instr->params);
            break;
        case IRInstr::cmp_le:
            gen_cmp_le(o, instr->params);
            break;
        case IRInstr::cmp_gt:
            gen_cmp_gt(o, instr->params);
            break;
        case IRInstr::cmp_ge:
            gen_cmp_ge(o, instr->params);
            break;
        case IRInstr::cmp_mod:
            gen_cmp_mod(o, instr->params);
            break;
        case IRInstr::bit_not:
            gen_bit_not(o, instr->params);
            break;
        case IRInstr::bit_and:
            gen_bit_and(o, instr->params);
            break;
        case IRInstr::bit_or:
            gen_bit_or(o, instr->params);
            break;
        case IRInstr::bit_xor:
            gen_bit_xor(o, instr->params);
            break;
        case IRInstr::logical_and:
            gen_logical_and(o, instr->params);
            break;
        case IRInstr::logical_or:
            gen_logical_or(o, instr->params);
            break;
        default:
            o << "    # Unknown instruction " << (int)instr->op << "\n";
            break;
    }
}

void AsmGeneratorX86_64::gen_ldconst(ostream& o, const vector<string>& params) { o << "    movl $" << params[1] << ", " << IR_reg_to_asm(params[0]) << "\n"; }
void AsmGeneratorX86_64::gen_copy(ostream& o, const vector<string>& params) { o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n    movl %eax, " << IR_reg_to_asm(params[0]) << "\n"; }
void AsmGeneratorX86_64::gen_add(ostream& o, const vector<string>& params) { o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n    addl " << IR_reg_to_asm(params[2]) << ", %eax\n    movl %eax, " << IR_reg_to_asm(params[0]) << "\n"; }
void AsmGeneratorX86_64::gen_sub(ostream& o, const vector<string>& params) { o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n    subl " << IR_reg_to_asm(params[2]) << ", %eax\n    movl %eax, " << IR_reg_to_asm(params[0]) << "\n"; }
void AsmGeneratorX86_64::gen_mul(ostream& o, const vector<string>& params) { o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n    imull " << IR_reg_to_asm(params[2]) << ", %eax\n    movl %eax, " << IR_reg_to_asm(params[0]) << "\n"; }
void AsmGeneratorX86_64::gen_div(ostream& o, const vector<string>& params) { o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n    cltd\n    idivl " << IR_reg_to_asm(params[2]) << "\n    movl %eax, " << IR_reg_to_asm(params[0]) << "\n"; }

void AsmGeneratorX86_64::gen_bit_not(ostream& o, const vector<string>& params) {
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    notl %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}
void AsmGeneratorX86_64::gen_bit_and(ostream& o, const vector<string>& params) {
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    andl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}
void AsmGeneratorX86_64::gen_bit_or(ostream& o, const vector<string>& params) {
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    orl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}
void AsmGeneratorX86_64::gen_bit_xor(ostream& o, const vector<string>& params) {
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    xorl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}
void AsmGeneratorX86_64::gen_cmp_eq(ostream& o, const vector<string>& params) {
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    cmpl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    sete %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}
void AsmGeneratorX86_64::gen_cmp_ne(ostream& o, const vector<string>& params) {
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    cmpl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    setne %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}
void AsmGeneratorX86_64::gen_cmp_lt(ostream& o, const vector<string>& params) {
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    cmpl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    setl %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}
void AsmGeneratorX86_64::gen_cmp_le(ostream& o, const vector<string>& params) {
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    cmpl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    setle %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}
void AsmGeneratorX86_64::gen_cmp_gt(ostream& o, const vector<string>& params) {
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    cmpl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    setg %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}
void AsmGeneratorX86_64::gen_cmp_ge(ostream& o, const vector<string>& params) {
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    cmpl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    setge %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}
void AsmGeneratorX86_64::gen_cmp_mod(ostream& o, const vector<string>& params) {
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    cltd\n";
    o << "    idivl " << IR_reg_to_asm(params[2]) << "\n";
    o << "    movl %edx, " << IR_reg_to_asm(params[0]) << "\n";
}
void AsmGeneratorX86_64::gen_logical_and(ostream& o, const vector<string>& params) {
    o << "    cmpl $0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    setne %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    cmpl $0, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    setne %cl\n";
    o << "    movzbl %cl, %ecx\n";
    o << "    andl %ecx, %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}
void AsmGeneratorX86_64::gen_logical_or(ostream& o, const vector<string>& params) {
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    orl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    setne %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}
void AsmGeneratorX86_64::gen_rmem(ostream& o, const vector<string>& params) {}
void AsmGeneratorX86_64::gen_wmem(ostream& o, const vector<string>& params) {}

void AsmGeneratorX86_64::gen_call(ostream& o, const vector<string>& params) {
    // params = {function_label, result_dest, arg1, arg2, ...}
    static const vector<string> argRegs = {"%edi", "%esi", "%edx", "%ecx", "%r8d", "%r9d"};

    // 1. Save caller-saved registers that might be in use
    // In our current simple compiler, we mainly use %eax, %ecx, %edx for intermediate results.
    // %eax is used for return value, so we don't need to save it if we are about to overwrite it.
    // However, the IR might have stored something in a temp that maps to a stack slot.
    // Our IR_reg_to_asm currently maps everything to stack slots except when it's an explicit register name.
    // So we don't strictly need to push/pop %rax, %rcx, %rdx etc. UNLESS they are used for something else.
    
    // 2. Align stack to 16 bytes before call
    // The stack must be 16-byte aligned at the moment of the 'call' instruction.
    // After 'push %rbp', the stack is 16-byte aligned if it was 16-byte aligned at entry.
    // BUT 'call' pushes 8 bytes (return address), and 'push %rbp' pushes another 8 bytes.
    // So inside the function body, RSP is 16-byte aligned.
    // If we subq $N, %rsp where N is a multiple of 16, it remains aligned.
    // Currently CFG::calculateRequiredStackSpace() ensures 16-byte alignment.
    
    // 3. Move arguments into specific registers according to System V AMD64 ABI
    for (size_t i = 2; i < params.size() && i - 2 < argRegs.size(); ++i) {
        o << "    movl " << IR_reg_to_asm(params[i]) << ", " << argRegs[i - 2] << "\n";
    }
    
    // 4. Call function
    o << "    call " << params[0] << "\n";
    
    // 5. Move result back to destination
    o << "    movl %eax, " << IR_reg_to_asm(params[1]) << "\n";
}

void AsmGeneratorX86_64::gen_ret(ostream& o, const vector<string>& params) {}
void AsmGeneratorX86_64::gen_prologue(ostream& o) {}
void AsmGeneratorX86_64::gen_epilogue(ostream& o) {}
BasicBlock* AsmGeneratorX86_64::findBBByVariable(string var) { return cfg->findBBByVariable(var); }

string AsmGeneratorX86_64::IR_reg_to_asm(string reg) {
    if (reg == "%edi" || reg == "!edi" || 
        reg == "%esi" || reg == "!esi" ||
        reg == "%edx" || reg == "!edx" ||
        reg == "%ecx" || reg == "!ecx" ||
        reg == "%r8d" || reg == "!r8d" ||
        reg == "%r9d" || reg == "!r9d" ||
        reg == "%eax" || reg == "!eax" ||
        reg == "%rax" || reg == "!rax") {
        if (reg[0] == '!') return "%" + reg.substr(1);
        return reg;
    }
    
    // Look for variable in the current BB or other BBs if not found
    BasicBlock* varBB = cfg->findBBByVariable(reg);
    if (!varBB && reg[0] == '!') {
        // Fallback to current BB for temps if not found elsewhere
        varBB = cfg->getCurrentBB();
    }
    
    if (varBB && varBB->has_var(reg)) {
        int index = varBB->get_var_index(reg);
        return to_string(index) + "(%rbp)";
    }
    
    return reg;
}
