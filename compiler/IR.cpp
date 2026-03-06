#include "IR.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

using namespace std;

// IRInstr implementation
IRInstr::IRInstr(BasicBlock* bb_, Operation op, Type t, vector<string> params)
    : bb(bb_), op(op), t(t), params(params) {}

void IRInstr::gen_asm(ostream &o) {
    #ifdef __APPLE__
    // ARM64 code generation for Apple Silicon
    if (op == ldconst) {
        o << "    mov w0, #" << params[1] << "\n";
        o << "    str w0, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == copy) {
        if (params[0] != params[1]) {
            string src_asm = bb->cfg->IR_reg_to_asm(params[1]);
            string dest_asm = bb->cfg->IR_reg_to_asm(params[0]);
            o << "    ldr w0, " << src_asm << "\n";
            o << "    str w0, " << dest_asm << "\n";
        }
    } else if (op == add) {
        o << "    ldr w0, " << bb->cfg->IR_reg_to_asm(params[1]) << "\n";
        o << "    ldr w8, " << bb->cfg->IR_reg_to_asm(params[2]) << "\n";
        o << "    add w0, w0, w8\n";
        o << "    str w0, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == sub) {
        o << "    ldr w0, " << bb->cfg->IR_reg_to_asm(params[1]) << "\n";
        o << "    ldr w8, " << bb->cfg->IR_reg_to_asm(params[2]) << "\n";
        o << "    sub w0, w0, w8\n";
        o << "    str w0, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == mul) {
        o << "    ldr w0, " << bb->cfg->IR_reg_to_asm(params[1]) << "\n";
        o << "    ldr w8, " << bb->cfg->IR_reg_to_asm(params[2]) << "\n";
        o << "    mul w0, w0, w8\n";
        o << "    str w0, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == div) {
        o << "    ldr w0, " << bb->cfg->IR_reg_to_asm(params[1]) << "\n";
        o << "    ldr w8, " << bb->cfg->IR_reg_to_asm(params[2]) << "\n";
        o << "    sdiv w0, w0, w8\n";
        o << "    str w0, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == bit_not) {
        o << "    ldr w0, " << bb->cfg->IR_reg_to_asm(params[1]) << "\n";
        o << "    eor w0, w0, #0xFFFFFFFF\n";
        o << "    str w0, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == bit_and) {
        o << "    ldr w0, " << bb->cfg->IR_reg_to_asm(params[1]) << "\n";
        o << "    ldr w8, " << bb->cfg->IR_reg_to_asm(params[2]) << "\n";
        o << "    and w0, w0, w8\n";
        o << "    str w0, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == bit_or) {
        o << "    ldr w0, " << bb->cfg->IR_reg_to_asm(params[1]) << "\n";
        o << "    ldr w8, " << bb->cfg->IR_reg_to_asm(params[2]) << "\n";
        o << "    orr w0, w0, w8\n";
        o << "    str w0, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == bit_xor) {
        o << "    ldr w0, " << bb->cfg->IR_reg_to_asm(params[1]) << "\n";
        o << "    ldr w8, " << bb->cfg->IR_reg_to_asm(params[2]) << "\n";
        o << "    eor w0, w0, w8\n";
        o << "    str w0, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == cmp_eq) {
        o << "    ldr w0, " << bb->cfg->IR_reg_to_asm(params[1]) << "\n";
        o << "    ldr w8, " << bb->cfg->IR_reg_to_asm(params[2]) << "\n";
        o << "    cmp w0, w8\n";
        o << "    cset w0, eq\n";
        o << "    str w0, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == cmp_lt) {
        o << "    ldr w0, " << bb->cfg->IR_reg_to_asm(params[1]) << "\n";
        o << "    ldr w8, " << bb->cfg->IR_reg_to_asm(params[2]) << "\n";
        o << "    cmp w0, w8\n";
        o << "    cset w0, lt\n";
        o << "    str w0, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == cmp_le) {
        o << "    ldr w0, " << bb->cfg->IR_reg_to_asm(params[1]) << "\n";
        o << "    ldr w8, " << bb->cfg->IR_reg_to_asm(params[2]) << "\n";
        o << "    cmp w0, w8\n";
        o << "    cset w0, le\n";
        o << "    str w0, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == rmem) {
        o << "    ldr w0, " << bb->cfg->IR_reg_to_asm(params[1]) << "\n";
        o << "    str w0, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == wmem) {
        o << "    ldr w0, " << bb->cfg->IR_reg_to_asm(params[1]) << "\n";
        o << "    str w0, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    }
    #else
    // x86_64 code generation (Linux)
    if (op == ldconst) {
        o << "    movl $" << params[1] << ", " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == copy) {
        if (params[0] != params[1]) {
            string src_asm = bb->cfg->IR_reg_to_asm(params[1]);
            string dest_asm = bb->cfg->IR_reg_to_asm(params[0]);
            if (dest_asm != src_asm) {
                o << "    movl " << src_asm << ", %eax\n";
                o << "    movl %eax, " << dest_asm << "\n";
            }
        }
    } else if (op == add) {
        o << "    movl " << bb->cfg->IR_reg_to_asm(params[1]) << ", %eax\n";
        o << "    addl " << bb->cfg->IR_reg_to_asm(params[2]) << ", %eax\n";
        o << "    movl %eax, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == sub) {
        o << "    movl " << bb->cfg->IR_reg_to_asm(params[1]) << ", %eax\n";
        o << "    subl " << bb->cfg->IR_reg_to_asm(params[2]) << ", %eax\n";
        o << "    movl %eax, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == mul) {
        o << "    movl " << bb->cfg->IR_reg_to_asm(params[1]) << ", %eax\n";
        o << "    imull " << bb->cfg->IR_reg_to_asm(params[2]) << ", %eax\n";
        o << "    movl %eax, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == div) {
        o << "    movl " << bb->cfg->IR_reg_to_asm(params[1]) << ", %eax\n";
        o << "    cltd\n";
        o << "    idivl " << bb->cfg->IR_reg_to_asm(params[2]) << "\n";
        o << "    movl %eax, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    } else if (op == bit_not) {
        o << "    movl " << bb->cfg->IR_reg_to_asm(params[1]) << ", %eax\n";
        o << "    not %eax\n";
        o << "    movl %eax, " << bb->cfg->IR_reg_to_asm(params[0]) << "\n";
    }
    #endif
}

// BasicBlock implementation
BasicBlock::BasicBlock(CFG* cfg, string entry_label) : cfg(cfg), label(entry_label) {
    exit_true = nullptr;
    exit_false = nullptr;
}

void BasicBlock::gen_asm(ostream &o) {
    o << label << ":\n";
    for (auto instr : instrs) {
        instr->gen_asm(o);
    }
    #ifdef __APPLE__
    // ARM64 code generation for control flow
    if (exit_true == nullptr) {
        cfg->gen_asm_epilogue(o);
    } else if (exit_false == nullptr) {
        o << "    b " << exit_true->label << "\n";
    } else {
        o << "    ldr w0, " << cfg->IR_reg_to_asm(test_var_name) << "\n";
        o << "    cmp w0, #0\n";
        o << "    b.eq " << exit_false->label << "\n";
        o << "    b " << exit_true->label << "\n";
    }
    #else
    // x86_64 code generation for control flow
    if (exit_true == nullptr) {
        cfg->gen_asm_epilogue(o);
    } else if (exit_false == nullptr) {
        o << "    jmp " << exit_true->label << "\n";
    } else {
        o << "    movl " << cfg->IR_reg_to_asm(test_var_name) << ", %eax\n";
        o << "    cmpl $0, %eax\n";
        o << "    je " << exit_false->label << "\n";
        o << "    jmp " << exit_true->label << "\n";
    }
    #endif
}

void BasicBlock::add_IRInstr(IRInstr::Operation op, Type t, vector<string> params) {
    instrs.push_back(new IRInstr(this, op, t, params));
}

// CFG implementation
CFG::CFG() {
    nextFreeSymbolIndex = -4;
    nextBBnumber = 0;
    current_bb = new BasicBlock(this, new_BB_name());
    add_bb(current_bb);
}

void CFG::add_bb(BasicBlock* bb) {
    bbs.push_back(bb);
}

void CFG::gen_asm(ostream& o) {
    gen_asm_prologue(o);
    for (auto bb : bbs) {
        bb->gen_asm(o);
    }
}

string CFG::IR_reg_to_asm(string reg) {
    if (reg == "!eax") {
        #ifdef __APPLE__
        return "w0";  // Return value in w0
        #else
        return "%eax";
        #endif
    }
    int index = get_var_index(reg);
    #ifdef __APPLE__
    return "[fp, #" + to_string(-index) + "]";  // ARM64 frame pointer offset
    #else
    return to_string(index) + "(%rbp)";
    #endif
}

void CFG::gen_asm_prologue(ostream& o) {
    int stackSpace = calculateRequiredStackSpace();
    #ifdef __APPLE__
    // ARM64 prologue for Apple Silicon
    o << ".globl _main\n";
    o << "_main:\n";
    o << "    stp fp, lr, [sp, #-16]!\n";  // Save frame pointer and link register
    o << "    mov fp, sp\n";
    o << "    sub sp, sp, #" << stackSpace << "\n";
    #else
    // x86_64 prologue for Linux
    o << ".globl main\n";
    o << "main:\n";
    o << "    pushq %rbp\n";
    o << "    movq %rsp, %rbp\n";
    o << "    subq $" << stackSpace << ", %rsp\n"; 
    #endif
}

void CFG::gen_asm_epilogue(ostream& o) {
    #ifdef __APPLE__
    // ARM64 epilogue for Apple Silicon
    o << "    mov sp, fp\n";
    o << "    ldp fp, lr, [sp], #16\n";
    o << "    ret\n";
    #else
    // x86_64 epilogue for Linux
    o << "    leave\n";
    o << "    ret\n";
    #endif
}

void CFG::add_to_symbol_table(string name, Type t) {
    SymbolType[name] = t;
    SymbolIndex[name] = nextFreeSymbolIndex;
    nextFreeSymbolIndex -= 4;
}

string CFG::create_new_tempvar(Type t) {
    string name = "!tmp" + to_string(-nextFreeSymbolIndex);
    add_to_symbol_table(name, t);
    return name;
}

int CFG::get_var_index(string name) {
    if (SymbolIndex.find(name) == SymbolIndex.end()) {
        cerr << "Error: Symbol " << name << " not found in symbol table." << endl;
        exit(1);
    }
    return SymbolIndex[name];
}

Type CFG::get_var_type(string name) {
    return SymbolType[name];
}

string CFG::new_BB_name() {
    return "BB" + to_string(nextBBnumber++);
}

// Helper function: get size in bytes for a given type
static int getTypeSize(Type t) {
    switch (t) {
        case INT:
            return 4;  // 4 bytes for int
        case CHAR:
            return 1;  // 1 byte for char
        case VOID:
            return 0;  // void has no size
        default:
            return 4;  // default to 4 bytes
    }
}

int CFG::calculateRequiredStackSpace() {
    // Calculate the exact stack space needed based on all variables
    // The nextFreeSymbolIndex is negative and represents the next free offset
    // We need to calculate how much space has been used (from -4 to nextFreeSymbolIndex)
    
    // Since nextFreeSymbolIndex starts at -4 and decrements by type size,
    // the total negative offset used is: abs(nextFreeSymbolIndex)
    // But we need to add padding for alignment
    
    int usedSpace = -nextFreeSymbolIndex;  // Convert to positive (e.g., -12 -> 12)
    
    // Add extra space for alignment and safety margin
    // Ensure 16-byte alignment
    int alignedSpace = usedSpace;
    if (alignedSpace % 16 != 0) {
        alignedSpace = ((alignedSpace / 16) + 1) * 16;
    }
    
    // Minimum 16 bytes for safety
    if (alignedSpace < 16) {
        alignedSpace = 16;
    }
    
    return alignedSpace;
}

void CFG::allocateVariable(string name, Type type) {
    // Get the size for this type
    int size = getTypeSize(type);
    
    // Add to symbol table
    SymbolType[name] = type;
    SymbolIndex[name] = nextFreeSymbolIndex;
    
    // Update next free index based on type size
    nextFreeSymbolIndex -= size;
}

void CFG::genOptimizedPrologue(ostream& o) {
    // Generate optimized prologue with exact stack space (16-byte aligned)
    int stackSpace = calculateRequiredStackSpace();
    
    #ifdef __APPLE__
    // ARM64 optimized prologue for Apple Silicon
    o << ".globl _main\n";
    o << "_main:\n";
    o << "    stp fp, lr, [sp, #-16]!\n";
    o << "    mov fp, sp\n";
    o << "    sub sp, sp, #" << stackSpace << "\n";
    #else
    // x86_64 optimized prologue for Linux
    o << ".globl main\n";
    o << "main:\n";
    o << "    pushq %rbp\n";
    o << "    movq %rsp, %rbp\n";
    o << "    subq $" << stackSpace << ", %rsp\n";
    #endif
}
