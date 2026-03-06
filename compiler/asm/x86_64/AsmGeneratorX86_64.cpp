#include "AsmGeneratorX86_64.h"
#include "../../IR.h"
#include <iostream>
#include <sstream>

using namespace std;

AsmGeneratorX86_64::AsmGeneratorX86_64(CFG* cfg) : AsmGenerator(cfg) {}

void AsmGeneratorX86_64::gen_asm(ostream& o) {
    gen_prologue(o);
    // Generate assembly for all basic blocks
    for (auto bb : cfg->getBBs()) {
        gen_asm_bb(o, bb);
    }
}

void AsmGeneratorX86_64::gen_asm_bb(ostream& o, BasicBlock* bb) {
    o << bb->label << ":\n";
    for (auto instr : bb->instrs) {
        gen_asm_instr(o, instr);
    }
    gen_control_flow(o, bb);
}

void AsmGeneratorX86_64::gen_asm_instr(ostream& o, IRInstr* instr) {
    // This method dispatches to the appropriate gen_* method based on operation
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
        case IRInstr::cmp_eq:
            gen_cmp_eq(o, instr->params);
            break;
        case IRInstr::cmp_lt:
            gen_cmp_lt(o, instr->params);
            break;
        case IRInstr::cmp_le:
            gen_cmp_le(o, instr->params);
            break;
        case IRInstr::rmem:
            gen_rmem(o, instr->params);
            break;
        case IRInstr::wmem:
            gen_wmem(o, instr->params);
            break;
        default:
            cerr << "Error: Unknown operation in gen_asm_instr" << endl;
            break;
    }
}

void AsmGeneratorX86_64::gen_ldconst(ostream& o, const vector<string>& params) {
    // ldconst: load constant into destination
    // params[0] = destination, params[1] = constant
    o << "    movl $" << params[1] << ", " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_copy(ostream& o, const vector<string>& params) {
    // copy: copy value from source to destination
    // params[0] = destination, params[1] = source
    if (params[0] != params[1]) {
        string src_asm = IR_reg_to_asm(params[1]);
        string dest_asm = IR_reg_to_asm(params[0]);
        
        // Special case: if destination is !eax (return value register),
        // directly move from source to %eax without intermediate copy
        if (params[0] == "!eax") {
            o << "    movl " << src_asm << ", %eax\n";
        } else if (params[1] == "!eax") {
            // Source is !eax, just move from %eax to destination
            o << "    movl %eax, " << dest_asm << "\n";
        } else if (dest_asm != src_asm) {
            o << "    movl " << src_asm << ", %eax\n";
            o << "    movl %eax, " << dest_asm << "\n";
        }
    }
}

void AsmGeneratorX86_64::gen_add(ostream& o, const vector<string>& params) {
    // add: destination = param1 + param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    addl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_sub(ostream& o, const vector<string>& params) {
    // sub: destination = param1 - param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    subl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_mul(ostream& o, const vector<string>& params) {
    // mul: destination = param1 * param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    imull " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_div(ostream& o, const vector<string>& params) {
    // div: destination = param1 / param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    cltd\n";
    o << "    idivl " << IR_reg_to_asm(params[2]) << "\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_bit_not(ostream& o, const vector<string>& params) {
    // bit_not: destination = ~param1
    // params[0] = destination, params[1] = operand
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    not %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_bit_and(ostream& o, const vector<string>& params) {
    // bit_and: destination = param1 & param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    andl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_bit_or(ostream& o, const vector<string>& params) {
    // bit_or: destination = param1 | param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    orl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_bit_xor(ostream& o, const vector<string>& params) {
    // bit_xor: destination = param1 ^ param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    xorl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_cmp_eq(ostream& o, const vector<string>& params) {
    // cmp_eq: destination = (param1 == param2)
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    cmpl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    sete %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_cmp_lt(ostream& o, const vector<string>& params) {
    // cmp_lt: destination = (param1 < param2)
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    cmpl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    setl %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_cmp_le(ostream& o, const vector<string>& params) {
    // cmp_le: destination = (param1 <= param2)
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    cmpl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    setle %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_rmem(ostream& o, const vector<string>& params) {
    // rmem: read from memory
    // params[0] = destination, params[1] = address
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_wmem(ostream& o, const vector<string>& params) {
    // wmem: write to memory
    // params[0] = address, params[1] = source
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}

string AsmGeneratorX86_64::IR_reg_to_asm(string reg) {
    if (reg == "!eax") {
        return "%eax";
    }
    int index = cfg->get_var_index(reg);
    return to_string(index) + "(%rbp)";
}

string AsmGeneratorX86_64::getOffset(const string& reg) {
    return IR_reg_to_asm(reg);
}

void AsmGeneratorX86_64::gen_prologue(ostream& o) {
    int stackSpace = cfg->calculateRequiredStackSpace();
    o << ".globl main\n";
    o << "main:\n";
    o << "    pushq %rbp\n";
    o << "    movq %rsp, %rbp\n";
    o << "    subq $" << stackSpace << ", %rsp\n";
}

void AsmGeneratorX86_64::gen_epilogue(ostream& o) {
    o << "    leave\n";
    o << "    ret\n";
}

void AsmGeneratorX86_64::gen_control_flow(ostream& o, BasicBlock* bb) {
    if (bb->exit_true == nullptr) {
        gen_epilogue(o);
    } else if (bb->exit_false == nullptr) {
        o << "    jmp " << bb->exit_true->label << "\n";
    } else {
        o << "    movl " << IR_reg_to_asm(bb->test_var_name) << ", %eax\n";
        o << "    cmpl $0, %eax\n";
        o << "    je " << bb->exit_false->label << "\n";
        o << "    jmp " << bb->exit_true->label << "\n";
    }
}
