#include "AsmGeneratorARM64.h"
#include "../../IR.h"
#include <iostream>
#include <sstream>

using namespace std;

AsmGeneratorARM64::AsmGeneratorARM64(CFG* cfg) : AsmGenerator(cfg) {}

void AsmGeneratorARM64::gen_asm(ostream& o) {
    gen_prologue(o);
    // Generate assembly for all basic blocks
    for (auto bb : cfg->getBBs()) {
        gen_asm_bb(o, bb);
    }
}

void AsmGeneratorARM64::gen_asm_bb(ostream& o, BasicBlock* bb) {
    o << bb->label << ":\n";
    for (auto instr : bb->instrs) {
        gen_asm_instr(o, instr);
    }
    gen_control_flow(o, bb);
}

void AsmGeneratorARM64::gen_asm_instr(ostream& o, IRInstr* instr) {
    // This method dispatches to the appropriate gen_* method based on operation
    // Since IRInstr stores the operation, we need to handle it differently
    // For now, we keep the original approach where IRInstr calls gen_asm
    // This method can be used by BasicBlock to generate instructions
}

void AsmGeneratorARM64::gen_ldconst(ostream& o, const vector<string>& params) {
    // ldconst: load constant into destination
    // params[0] = destination, params[1] = constant
    o << "    mov w0, #" << params[1] << "\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_copy(ostream& o, const vector<string>& params) {
    // copy: copy value from source to destination
    // params[0] = destination, params[1] = source
    if (params[0] != params[1]) {
        string src_asm = IR_reg_to_asm(params[1]);
        string dest_asm = IR_reg_to_asm(params[0]);
        o << "    ldr w0, " << src_asm << "\n";
        o << "    str w0, " << dest_asm << "\n";
    }
}

void AsmGeneratorARM64::gen_add(ostream& o, const vector<string>& params) {
    // add: destination = param1 + param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    ldr w8, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    add w0, w0, w8\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_sub(ostream& o, const vector<string>& params) {
    // sub: destination = param1 - param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    ldr w8, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    sub w0, w0, w8\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_mul(ostream& o, const vector<string>& params) {
    // mul: destination = param1 * param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    ldr w8, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    mul w0, w0, w8\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_div(ostream& o, const vector<string>& params) {
    // div: destination = param1 / param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    ldr w8, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    sdiv w0, w0, w8\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_bit_not(ostream& o, const vector<string>& params) {
    // bit_not: destination = ~param1
    // params[0] = destination, params[1] = operand
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    eor w0, w0, #0xFFFFFFFF\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_bit_and(ostream& o, const vector<string>& params) {
    // bit_and: destination = param1 & param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    ldr w8, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    and w0, w0, w8\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_bit_or(ostream& o, const vector<string>& params) {
    // bit_or: destination = param1 | param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    ldr w8, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    orr w0, w0, w8\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_bit_xor(ostream& o, const vector<string>& params) {
    // bit_xor: destination = param1 ^ param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    ldr w8, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    eor w0, w0, w8\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_cmp_eq(ostream& o, const vector<string>& params) {
    // cmp_eq: destination = (param1 == param2)
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    ldr w8, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    cmp w0, w8\n";
    o << "    cset w0, eq\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_cmp_lt(ostream& o, const vector<string>& params) {
    // cmp_lt: destination = (param1 < param2)
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    ldr w8, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    cmp w0, w8\n";
    o << "    cset w0, lt\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_cmp_le(ostream& o, const vector<string>& params) {
    // cmp_le: destination = (param1 <= param2)
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    ldr w8, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    cmp w0, w8\n";
    o << "    cset w0, le\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_rmem(ostream& o, const vector<string>& params) {
    // rmem: read from memory
    // params[0] = destination, params[1] = address
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_wmem(ostream& o, const vector<string>& params) {
    // wmem: write to memory
    // params[0] = address, params[1] = source
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

string AsmGeneratorARM64::IR_reg_to_asm(string reg) {
    if (reg == "!eax") {
        return "w0";  // Return value in w0
    }
    int index = cfg->get_var_index(reg);
    return "[fp, #" + to_string(-index) + "]";  // ARM64 frame pointer offset
}

string AsmGeneratorARM64::getOffset(const string& reg) {
    return IR_reg_to_asm(reg);
}

void AsmGeneratorARM64::gen_prologue(ostream& o) {
    int stackSpace = cfg->calculateRequiredStackSpace();
    o << ".globl _main\n";
    o << "_main:\n";
    o << "    stp fp, lr, [sp, #-16]!\n";  // Save frame pointer and link register
    o << "    mov fp, sp\n";
    o << "    sub sp, sp, #" << stackSpace << "\n";
}

void AsmGeneratorARM64::gen_epilogue(ostream& o) {
    o << "    mov sp, fp\n";
    o << "    ldp fp, lr, [sp], #16\n";
    o << "    ret\n";
}

void AsmGeneratorARM64::gen_control_flow(ostream& o, BasicBlock* bb) {
    if (bb->exit_true == nullptr) {
        gen_epilogue(o);
    } else if (bb->exit_false == nullptr) {
        o << "    b " << bb->exit_true->label << "\n";
    } else {
        o << "    ldr w0, " << IR_reg_to_asm(bb->test_var_name) << "\n";
        o << "    cmp w0, #0\n";
        o << "    b.eq " << bb->exit_false->label << "\n";
        o << "    b " << bb->exit_true->label << "\n";
    }
}
