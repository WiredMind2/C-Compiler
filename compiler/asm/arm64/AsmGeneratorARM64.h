#ifndef ASMGENERATORARM64_H
#define ASMGENERATORARM64_H

#include "../AsmGenerator.h"

//! ARM64 assembly generator implementation (Apple Silicon)
class AsmGeneratorARM64 : public AsmGenerator {
public:
    //! Constructor
    AsmGeneratorARM64(CFG* cfg);

    //! Destructor
    ~AsmGeneratorARM64() override = default;

    //! Generate assembly for the entire CFG
    void gen_asm(std::ostream& o) override;

    //! Generate assembly for a single basic block
    void gen_asm_bb(std::ostream& o, BasicBlock* bb) override;

    //! Generate assembly for a single IR instruction
    void gen_asm_instr(std::ostream& o, IRInstr* instr) override;

    //---------------- IR Operations ----------------

    //! Generate ldconst: load constant into destination
    void gen_ldconst(std::ostream& o, const std::vector<std::string>& params) override;

    //! Generate copy: copy value from source to destination
    void gen_copy(std::ostream& o, const std::vector<std::string>& params) override;

    //! Generate add: destination = param1 + param2
    void gen_add(std::ostream& o, const std::vector<std::string>& params) override;

    //! Generate sub: destination = param1 - param2
    void gen_sub(std::ostream& o, const std::vector<std::string>& params) override;

    //! Generate mul: destination = param1 * param2
    void gen_mul(std::ostream& o, const std::vector<std::string>& params) override;

    //! Generate div: destination = param1 / param2
    void gen_div(std::ostream& o, const std::vector<std::string>& params) override;

    //! Generate bit_not: destination = ~param1
    void gen_bit_not(std::ostream& o, const std::vector<std::string>& params) override;

    //! Generate bit_and: destination = param1 & param2
    void gen_bit_and(std::ostream& o, const std::vector<std::string>& params) override;

    //! Generate bit_or: destination = param1 | param2
    void gen_bit_or(std::ostream& o, const std::vector<std::string>& params) override;

    //! Generate bit_xor: destination = param1 ^ param2
    void gen_bit_xor(std::ostream& o, const std::vector<std::string>& params) override;

    //! Generate cmp_eq: destination = (param1 == param2)
    void gen_cmp_eq(std::ostream& o, const std::vector<std::string>& params) override;

    //! Generate cmp_lt: destination = (param1 < param2)
    void gen_cmp_lt(std::ostream& o, const std::vector<std::string>& params) override;

    //! Generate cmp_le: destination = (param1 <= param2)
    void gen_cmp_le(std::ostream& o, const std::vector<std::string>& params) override;

    //! Generate rmem: read from memory
    void gen_rmem(std::ostream& o, const std::vector<std::string>& params) override;

    //! Generate wmem: write to memory
    void gen_wmem(std::ostream& o, const std::vector<std::string>& params) override;

    //---------------- Helper Methods ----------------

    //! Convert IR register to assembly representation
    std::string IR_reg_to_asm(std::string reg) override;

    //---------------- Prologue/Epilogue ----------------

    //! Generate function prologue
    void gen_prologue(std::ostream& o) override;

    //! Generate function epilogue
    void gen_epilogue(std::ostream& o) override;

    //! Generate control flow for basic block
    void gen_control_flow(std::ostream& o, BasicBlock* bb) override;

private:
    //! Helper to get offset string for a register
    std::string getOffset(const std::string& reg);
};

#endif // ASMGENERATORARM64_H
