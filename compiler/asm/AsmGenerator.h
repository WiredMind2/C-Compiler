#ifndef ASMGENERATOR_H
#define ASMGENERATOR_H

#include <iostream>
#include <string>
#include <vector>

class CFG;
class BasicBlock;
class IRInstr;

//! Abstract base class for assembly generation
/*!
 * This interface defines the contract for generating assembly code
 * for different target architectures (x86_64, ARM64).
 */
class AsmGenerator {
public:
    //! Virtual destructor
    virtual ~AsmGenerator() = default;

    //! Generate assembly for the entire CFG
    virtual void gen_asm(std::ostream& o) = 0;

    //! Generate assembly for a single basic block
    virtual void gen_asm_bb(std::ostream& o, BasicBlock* bb) = 0;

    //! Generate assembly for a single IR instruction
    virtual void gen_asm_instr(std::ostream& o, IRInstr* instr) = 0;

    //---------------- IR Operations ----------------

    //! Generate ldconst: load constant into destination
    virtual void gen_ldconst(std::ostream& o, const std::vector<std::string>& params) = 0;

    //! Generate copy: copy value from source to destination
    virtual void gen_copy(std::ostream& o, const std::vector<std::string>& params) = 0;

    //! Generate add: destination = param1 + param2
    virtual void gen_add(std::ostream& o, const std::vector<std::string>& params) = 0;

    //! Generate sub: destination = param1 - param2
    virtual void gen_sub(std::ostream& o, const std::vector<std::string>& params) = 0;

    //! Generate mul: destination = param1 * param2
    virtual void gen_mul(std::ostream& o, const std::vector<std::string>& params) = 0;

    //! Generate div: destination = param1 / param2
    virtual void gen_div(std::ostream& o, const std::vector<std::string>& params) = 0;

    //! Generate bit_not: destination = ~param1
    virtual void gen_bit_not(std::ostream& o, const std::vector<std::string>& params) = 0;

    //! Generate bit_and: destination = param1 & param2
    virtual void gen_bit_and(std::ostream& o, const std::vector<std::string>& params) = 0;

    //! Generate bit_or: destination = param1 | param2
    virtual void gen_bit_or(std::ostream& o, const std::vector<std::string>& params) = 0;

    //! Generate bit_xor: destination = param1 ^ param2
    virtual void gen_bit_xor(std::ostream& o, const std::vector<std::string>& params) = 0;

    //! Generate cmp_eq: destination = (param1 == param2)
    virtual void gen_cmp_eq(std::ostream& o, const std::vector<std::string>& params) = 0;

    //! Generate cmp_lt: destination = (param1 < param2)
    virtual void gen_cmp_lt(std::ostream& o, const std::vector<std::string>& params) = 0;

    //! Generate cmp_le: destination = (param1 <= param2)
    virtual void gen_cmp_le(std::ostream& o, const std::vector<std::string>& params) = 0;

    //! Generate rmem: read from memory
    virtual void gen_rmem(std::ostream& o, const std::vector<std::string>& params) = 0;

    //! Generate wmem: write to memory
    virtual void gen_wmem(std::ostream& o, const std::vector<std::string>& params) = 0;

    //---------------- Helper Methods ----------------

    //! Convert IR register to assembly representation
    virtual std::string IR_reg_to_asm(std::string reg) = 0;

    //---------------- Prologue/Epilogue ----------------

    //! Generate function prologue
    virtual void gen_prologue(std::ostream& o) = 0;

    //! Generate function epilogue
    virtual void gen_epilogue(std::ostream& o) = 0;

    //! Generate control flow for basic block
    virtual void gen_control_flow(std::ostream& o, BasicBlock* bb) = 0;

protected:
    CFG* cfg;  // Pointer to the CFG for accessing symbol table and helpers

    //! Constructor with CFG pointer
    AsmGenerator(CFG* cfg) : cfg(cfg) {}
};

#endif // ASMGENERATOR_H
