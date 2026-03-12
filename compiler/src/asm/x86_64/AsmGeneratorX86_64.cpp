#include "AsmGeneratorX86_64.h"
#include "../../IR.h"
#include <iostream>
#include <sstream>

using namespace std;

AsmGeneratorX86_64::AsmGeneratorX86_64(CFG* cfg) : AsmGenerator(cfg) {}

void AsmGeneratorX86_64::gen_asm(ostream& o) {
    // Determine which BBs are function entry points
    std::vector<BasicBlock*> functionEntryBBs;
    for (auto bb : cfg->getBBs()) {
        bool isEntryBlock = false;
        for (const auto& func : cfg->getFunctions()) {
            if (bb->label == func.name) {
                isEntryBlock = true;
                break;
            }
        }
        if (cfg->getFunctions().empty() && bb->label == "main") {
            isEntryBlock = true;
        }
        if (isEntryBlock) {
            functionEntryBBs.push_back(bb);
        }
    }

    // Generate .globl only for function entry points
    for (auto bb : functionEntryBBs) {
        o << ".globl " << bb->label << "\n";
    }
    
    // Generate assembly for all basic blocks
    for (auto bb : cfg->getBBs()) {
        // Set current_bb for this BB so IR_reg_to_asm can find variable indices
        cfg->current_bb = bb;
        // Let the BasicBlock handle its own generation (instructions + terminator)
        bb->gen_asm(o);
    }
}

void AsmGeneratorX86_64::gen_asm_bb(ostream& o, BasicBlock* bb, bool isFirstBB) {
    // Set current_bb for this BB so IR_reg_to_asm can find variable indices
    cfg->current_bb = bb;
    
    // Let the BasicBlock handle its own generation (instructions + terminator)
    bb->gen_asm(o);
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
        case IRInstr::cmp_gt:
            gen_cmp_gt(o, instr->params);
            break;
        case IRInstr::cmp_ge:
            gen_cmp_ge(o, instr->params);
            break;
        case IRInstr::cmp_mod:
            gen_cmp_mod(o, instr->params);
            break;
        case IRInstr::logical_and:
            gen_logical_and(o, instr->params);
            break;
        case IRInstr::logical_or:
            gen_logical_or(o, instr->params);
            break;
        case IRInstr::rmem:
            gen_rmem(o, instr->params);
            break;
        case IRInstr::wmem:
            gen_wmem(o, instr->params);
            break;
        case IRInstr::call:
            gen_call(o, instr->params);
            break;
        case IRInstr::ret:
            gen_ret(o, instr->params);
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

void AsmGeneratorX86_64::gen_cmp_gt(ostream& o, const vector<string>& params) {
    // cmp_gt: destination = (param1 > param2)
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    cmpl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    setg %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_cmp_ge(ostream& o, const vector<string>& params) {
    // cmp_ge: destination = (param1 >= param2)
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    cmpl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    setge %al\n";
    o << "    movzbl %al, %eax\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_cmp_mod(ostream& o, const vector<string>& params) {
    // cmp_mod: destination = param1 % param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    cltd\n";
    o << "    idivl " << IR_reg_to_asm(params[2]) << "\n";
    o << "    movl %edx, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_logical_and(ostream& o, const vector<string>& params) {
    // logical_and: destination = (param1 && param2)
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    // For logical AND, we need to evaluate both operands and convert to 0/1
    static int labelCount = 0;
    int thisLabel = labelCount++;
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    cmpl $0, %eax\n";
    o << "    je .Lend_and_" << thisLabel << "\n";
    o << "    movl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    cmpl $0, %eax\n";
    o << "    je .Lend_and_" << thisLabel << "\n";
    o << "    movl $1, %eax\n";
    o << "    jmp .Ldone_and_" << thisLabel << "\n";
    o << ".Lend_and_" << thisLabel << ":\n";
    o << "    movl $0, %eax\n";
    o << ".Ldone_and_" << thisLabel << ":\n";
    o << "    movl %eax, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorX86_64::gen_logical_or(ostream& o, const vector<string>& params) {
    // logical_or: destination = (param1 || param2)
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    // For logical OR, we need to evaluate both operands and convert to 0/1
    static int labelCount = 0;
    int thisLabel = labelCount++;
    o << "    movl " << IR_reg_to_asm(params[1]) << ", %eax\n";
    o << "    cmpl $0, %eax\n";
    o << "    jne .Lend_or_" << thisLabel << "\n";
    o << "    movl " << IR_reg_to_asm(params[2]) << ", %eax\n";
    o << "    cmpl $0, %eax\n";
    o << "    jne .Lend_or_" << thisLabel << "\n";
    o << "    movl $0, %eax\n";
    o << "    jmp .Ldone_or_" << thisLabel << "\n";
    o << ".Lend_or_" << thisLabel << ":\n";
    o << "    movl $1, %eax\n";
    o << ".Ldone_or_" << thisLabel << ":\n";
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
    BasicBlock* bb = cfg->findBBByVariable(reg);
    if (!bb) {
        cerr << "Error: Variable " << reg << " not found in any basic block." << endl;
        exit(1);
    }
    int index = bb->get_var_index(reg);
    return to_string(index) + "(%rbp)";
}

string AsmGeneratorX86_64::getOffset(const string& reg) {
    return IR_reg_to_asm(reg);
}

void AsmGeneratorX86_64::gen_prologue(ostream& o) {
    int stackSpace = cfg->calculateRequiredStackSpace();
    // Get the function name from the first basic block's label
    string funcName = "main";
    if (!cfg->getBBs().empty()) {
        funcName = cfg->getBBs()[0]->label;
    }
    // Skip .globl here since it's generated in gen_asm for all functions
    o << funcName << ":\n";
    o << "    pushq %rbp\n";
    o << "    movq %rsp, %rbp\n";
    o << "    subq $" << stackSpace << ", %rsp\n";
}

void AsmGeneratorX86_64::gen_epilogue(ostream& o) {
    o << "    leave\n";
    o << "    ret\n";
}

BasicBlock* AsmGeneratorX86_64::findBBByVariable(string var) {
    return cfg->findBBByVariable(var);
}



void AsmGeneratorX86_64::gen_call(ostream& o, const vector<string>& params) {
    // call instruction: params[0] = function label, params[1] = destination, params[2+] = arguments
    string funcLabel = params[0];
    string destReg = params[1];
    
    // Handle arguments - pass them in registers (rdi, rsi, rdx, rcx, r8, r9)
    // For x86_64, we need to use 64-bit registers
    static const string argRegs[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
    int numArgs = params.size() - 2;
    
    for (int i = 0; i < numArgs && i < 6; i++) {
        string arg = params[2 + i];
        // Move argument to appropriate register (sign-extend 32-bit to 64-bit)
        o << "    movl " << IR_reg_to_asm(arg) << ", %eax\n";
        o << "    movslq %eax, %rax\n";
        o << "    movq %rax, " << argRegs[i] << "\n";
    }
    
    // Generate call instruction
    o << "    call " << funcLabel << "\n";
    
    // Move return value to destination
    if (destReg != "!eax") {
        o << "    movl %eax, " << IR_reg_to_asm(destReg) << "\n";
    }
}

void AsmGeneratorX86_64::gen_ret(ostream& o, const vector<string>& params) {
    // ret instruction: params[0] = return value register (or empty)
    if (!params.empty() && !params[0].empty() && params[0] != "!eax") {
        // Move return value to %eax
        o << "    movl " << IR_reg_to_asm(params[0]) << ", %eax\n";
    }
    // Standard function epilogue
    o << "    leave\n";
    o << "    ret\n";
}
