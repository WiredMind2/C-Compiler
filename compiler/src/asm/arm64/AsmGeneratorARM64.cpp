#include "AsmGeneratorARM64.h"
#include "../../IR.h"
#include <sstream>

using namespace std;

AsmGeneratorARM64::AsmGeneratorARM64(CFG* cfg) : AsmGenerator(cfg) {}

void AsmGeneratorARM64::gen_asm(ostream& o) {
    // Generate .globl for all functions
    for (auto bb : cfg->getBBs()) {
        o << ".globl _" << bb->label << "\n";
    }
    
    gen_prologue(o);
    // Generate assembly for all basic blocks
    bool isFirstBB = true;
    for (auto bb : cfg->getBBs()) {
        gen_asm_bb(o, bb, isFirstBB);
        isFirstBB = false;
    }
}

void AsmGeneratorARM64::gen_asm_bb(ostream& o, BasicBlock* bb, bool isFirstBB) {
    // Set current_bb for this BB so IR_reg_to_asm can find variable indices
    cfg->current_bb = bb;
    
    // Skip label for the first BB since prologue already outputs it
    if (!isFirstBB) {
        o << bb->label << ":\n";
    }
    for (auto instr : bb->instrs) {
        gen_asm_instr(o, instr);
    }
    gen_control_flow(o, bb);
}

void AsmGeneratorARM64::gen_asm_instr(ostream& o, IRInstr* instr) {
    switch (instr->op) {
        case IRInstr::ldconst:
            gen_ldconst(o, instr->params, instr->t);
            break;
        case IRInstr::copy:
            gen_copy(o, instr->params, instr->t);
            break;
        case IRInstr::add:
            gen_add(o, instr->params, instr->t);
            break;
        case IRInstr::sub:
            gen_sub(o, instr->params, instr->t);
            break;
        case IRInstr::mul:
            gen_mul(o, instr->params, instr->t);
            break;
        case IRInstr::div:
            gen_div(o, instr->params, instr->t);
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
            gen_cmp_eq(o, instr->params, instr->t);
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
            gen_ret(o, instr->params, instr->t);
            break;
        default:
            break;
    }
}

void AsmGeneratorARM64::gen_ldconst(ostream& o, const vector<string>& params, Type type) {
    // ldconst: load constant into destination
    // params[0] = destination, params[1] = constant
    int64_t val = stol(params[1]) & 0xFFFFFFFF;

    if (val < 65536) {
        o << "    mov w0, #" << val << "\n";
    } else {
        o << "    ldr w0, =" << val << "\n";
    }
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_copy(ostream& o, const vector<string>& params, Type type) {
    // copy: copy value from source to destination
    // params[0] = destination, params[1] = source
    if (params[0] != params[1]) {
        string src_asm = IR_reg_to_asm(params[1]);
        string dest_asm = IR_reg_to_asm(params[0]);
        if (src_asm != "w0") {
            o << "    ldr w0, " << src_asm << "\n";
        }
        if (dest_asm != "w0") {
            o << "    str w0, " << dest_asm << "\n";
        }
    }
}

void AsmGeneratorARM64::gen_add(ostream& o, const vector<string>& params, Type type) {
    // add: destination = param1 + param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    ldr w8, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    add w0, w0, w8\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_sub(ostream& o, const vector<string>& params, Type type) {
    // sub: destination = param1 - param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    ldr w8, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    sub w0, w0, w8\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_mul(ostream& o, const vector<string>& params, Type type) {
    // mul: destination = param1 * param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    ldr w8, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    mul w0, w0, w8\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_div(ostream& o, const vector<string>& params, Type type) {
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

void AsmGeneratorARM64::gen_cmp_eq(ostream& o, const vector<string>& params, Type type) {
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

void AsmGeneratorARM64::gen_cmp_gt(ostream& o, const vector<string>& params) {
    // cmp_gt: destination = (param1 > param2)
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    ldr w8, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    cmp w0, w8\n";
    o << "    cset w0, gt\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_cmp_ge(ostream& o, const vector<string>& params) {
    // cmp_ge: destination = (param1 >= param2)
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    ldr w8, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    cmp w0, w8\n";
    o << "    cset w0, ge\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_cmp_mod(ostream& o, const vector<string>& params) {
    // cmp_mod: destination = param1 % param2
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    // In ARM64, we need to use sdiv to get quotient and then multiply/subtract
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    ldr w8, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    sdiv w1, w0, w8\n";  // quotient
    o << "    mul w1, w1, w8\n";    // quotient * divisor
    o << "    sub w0, w0, w1\n";    // remainder
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_logical_and(ostream& o, const vector<string>& params) {
    // logical_and: destination = (param1 && param2)
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    // For logical AND, we need to evaluate both operands and convert to 0/1
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    cmp w0, #0\n";
    o << "    b.eq .Lend_and\n";
    o << "    ldr w0, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    cmp w0, #0\n";
    o << "    b.eq .Lend_and\n";
    o << "    mov w0, #1\n";
    o << "    b .Ldone_and\n";
    o << ".Lend_and:\n";
    o << "    mov w0, #0\n";
    o << ".Ldone_and:\n";
    o << "    str w0, " << IR_reg_to_asm(params[0]) << "\n";
}

void AsmGeneratorARM64::gen_logical_or(ostream& o, const vector<string>& params) {
    // logical_or: destination = (param1 || param2)
    // params[0] = destination, params[1] = operand1, params[2] = operand2
    // For logical OR, we need to evaluate both operands and convert to 0/1
    o << "    ldr w0, " << IR_reg_to_asm(params[1]) << "\n";
    o << "    cmp w0, #0\n";
    o << "    b.ne .Lend_or\n";
    o << "    ldr w0, " << IR_reg_to_asm(params[2]) << "\n";
    o << "    cmp w0, #0\n";
    o << "    b.ne .Lend_or\n";
    o << "    mov w0, #0\n";
    o << "    b .Ldone_or\n";
    o << ".Lend_or:\n";
    o << "    mov w0, #1\n";
    o << ".Ldone_or:\n";
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
        return "w0";
    }
    int index = cfg->getCurrentBB()->get_var_index(reg);
    return "[fp, #" + to_string(index) + "]";  // ARM64 frame pointer offset
}

string AsmGeneratorARM64::getOffset(const string& reg) {
    return IR_reg_to_asm(reg);
}

void AsmGeneratorARM64::gen_prologue(ostream& o) {
    int stackSpace = cfg->calculateRequiredStackSpace();
    // Get the function name from the first basic block's label
    string funcName = "_main";
    if (!cfg->getBBs().empty()) {
        funcName = "_" + cfg->getBBs()[0]->label;
    }
    // Skip .globl here since it's generated in gen_asm for all functions
    o << funcName << ":\n";
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

void AsmGeneratorARM64::gen_call(ostream& o, const vector<string>& params) {
    // call instruction: params[0] = function label, params[1] = destination, params[2+] = arguments
    string funcLabel = params[0];
    string destReg = params[1];
    
    // Handle arguments - pass them in registers (x0, x1, x2, x3, x4, x5)
    static const string argRegs[] = {"x0", "x1", "x2", "x3", "x4", "x5"};
    int numArgs = params.size() - 2;
    
    for (int i = 0; i < numArgs && i < 6; i++) {
        string arg = params[2 + i];
        // Move argument to appropriate register
        o << "    ldr w0, " << IR_reg_to_asm(arg) << "\n";
        o << "    mov " << argRegs[i] << ", w0\n";
    }
    
    // Generate call instruction
    o << "    bl " << funcLabel << "\n";
    
    // Move return value to destination
    if (destReg != "!eax") {
        o << "    str w0, " << IR_reg_to_asm(destReg) << "\n";
    }
}

void AsmGeneratorARM64::gen_ret(ostream& o, const vector<string>& params, Type type) {
    // ret instruction: params[0] = return value register (or empty)
    // ARM64 return value is already in w0, so we just need to return
    // Generate return
    o << "    ret\n";
}
