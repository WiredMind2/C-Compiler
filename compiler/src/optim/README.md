# Optimization Framework Documentation

This document explains how the optimization framework works and how to create new optimization passes.

---

## Table of Contents

1. [Overview](#overview)
2. [Architecture](#architecture)
3. [How Optimizations Work](#how-optimizations-work)
4. [Creating a New Optimization Pass](#creating-a-new-optimization-pass)
5. [Pass Categories and Timing](#pass-categories-and-timing)
6. [Example: Step-by-Step](#example-step-by-step)
7. [Testing Optimizations](#testing-optimizations)

---

## Overview

The optimization framework is a modular system that sits between IR generation and assembly code generation:

```
Source Code → Parser → AST → CodeGenVisitor → IR → Optimizer → Assembly
                                                       ↑
                                                       Here!
```

### What Problem Does It Solve?

The original issue was that `return 42;` generated suboptimal assembly:

```asm
; Before optimization
movl $42, -4(%rbp)    # Load constant to stack
movl -4(%rbp), %eax   # Copy to return register
```

After the `LoadConstantToRegister` optimization:

```asm
; After optimization  
movl $42, %eax        # Load directly to return register
```

---

## Architecture

### Core Components

| File | Purpose |
|------|---------|
| [`optim/OptimizationPass.h`](optim/OptimizationPass.h) | Base class for all optimizations |
| [`optim/OptimizationManager.h`](optim/OptimizationManager.h) | Orchestrates running passes |
| [`optim/PassRegistry.h`](optim/PassRegistry.h) | Dynamic pass registration |

### Directory Structure

```
compiler/optim/
├── OptimizationPass.h      # Base class
├── OptimizationManager.h   # Manager
├── OptimizationManager.cpp # Implementation
├── PassRegistry.h          # Registry
├── PassRegistry.cpp        # Implementation
└── LoadConstantToRegister.h/cpp  # Example optimization
```

---

## How Optimizations Work

### 1. The IR Level

The compiler uses an Intermediate Representation (IR) with 3-address instructions:

```cpp
// IR instruction types (from IR.h)
enum Operation {
    ldconst,  // Load constant: ldconst dest, value
    copy,     // Copy: copy dest, src
    add,      // Addition: add dest, a, b
    sub,      // Subtraction: sub dest, a, b
    mul,      // Multiplication: mul dest, a, b
    div,      // Division: div dest, a, b
    // ... and more
};
```

### 2. Optimization Pattern Detection

Each optimization pass scans the IR instructions looking for patterns that can be simplified. For example, `LoadConstantToRegister` looks for:

```
ldconst tmp, value   →   ldconst target, value
copy target, tmp
```

And replaces it with:

```
ldconst target, value
```

### 3. Running the Optimizer

In [`main.cpp`](main.cpp), optimizations run after IR generation but before assembly:

```cpp
// Create the CFG from AST
CodeGenVisitor v(symbolTable);
v.visit(tree);
CFG *cfg = v.getCFG();

// Run optimizations
optim::OptimizationManager optimizer;
optimizer.addPass(std::make_unique<optim::LoadConstantToRegisterPass>());
optimizer.runOptimizations(cfg);

// Generate optimized assembly
cfg->gen_asm(cout);
```

---

## Creating a New Optimization Pass

### Step 1: Create Header File

Create `compiler/optim/MyOptimization.h`:

```cpp
#pragma once

#include "OptimizationPass.h"

namespace optim {

class MyOptimizationPass : public OptimizationPass {
public:
    MyOptimizationPass() = default;
    
    // Required: Pass identification
    std::string getName() const override {
        return "my-optimization";
    }
    
    std::string getDescription() const override {
        return "Description of what this optimization does";
    }
    
    // Required: What kind of optimization?
    PassKind getKind() const override {
        return PassKind::IR_OPT;  // or ASM_OPT or ANALYSIS
    }
    
    // Required: When to run?
    PassTiming getTiming() const override {
        return PassTiming::NORMAL;  // EARLY, NORMAL, or LATE
    }
    
    // Required: Main optimization logic
    bool optimize(CFG* cfg) override;
};

} // namespace optim
```

### Step 2: Create Implementation File

Create `compiler/optim/MyOptimization.cpp`:

```cpp
#include "MyOptimization.h"

namespace optim {

bool MyOptimizationPass::optimize(CFG* cfg) {
    bool modified = false;
    
    // Iterate through all basic blocks
    for (auto* bb : cfg->getBBs()) {
        // Your optimization logic here
        // ...
        modified = true;  // Set true if you make changes
    }
    
    return modified;
}

} // namespace optim
```

### Step 3: Add to Build System

Add to [`Makefile`](Makefile):

```makefile
# Add to OBJECTS list
build/MyOptimization.o \
```

### Step 4: Register the Pass

In [`main.cpp`](main.cpp):

```cpp
#include "optim/MyOptimization.h"

int main() {
    // ... existing code ...
    
    optim::OptimizationManager optimizer;
    optimizer.addPass(std::make_unique<optim::MyOptimizationPass>());
    optimizer.runOptimizations(cfg);
    
    // ... existing code ...
}
```

---

## Pass Categories and Timing

### PassKind

| Kind | Description | When It Runs |
|------|-------------|--------------|
| `IR_OPT` | Operates on IR | After IR generation, before assembly |
| `ASM_OPT` | Operates on assembly | After assembly generation |
| `ANALYSIS` | Gathers information | Any time (doesn't modify) |

### PassTiming

| Timing | Description | Use Case |
|--------|-------------|----------|
| `EARLY` | Runs first | Simplifications that enable other optimizations |
| `NORMAL` | Default | Most optimizations |
| `LATE` | Runs last | Cleanups after all other optimizations |

---

## Example: Step-by-Step

Let's create a simple optimization that removes redundant copies (e.g., `copy x, x`):

### 1. Header File

```cpp
// optim/RedundantCopyElimination.h
#pragma once
#include "OptimizationPass.h"

namespace optim {

class RedundantCopyEliminationPass : public OptimizationPass {
public:
    std::string getName() const override { return "redundant-copy-elim"; }
    std::string getDescription() const override { 
        return "Removes copy instructions where src == dest"; 
    }
    PassKind getKind() const override { return PassKind::IR_OPT; }
    PassTiming getTiming() const override { return PassTiming::EARLY; }
    bool optimize(CFG* cfg) override;
};

} // namespace optim
```

### 2. Implementation

```cpp
// optim/RedundantCopyElimination.cpp
#include "RedundantCopyElimination.h"
#include "../IR.h"

namespace optim {

bool RedundantCopyEliminationPass::optimize(CFG* cfg) {
    bool modified = false;
    
    for (auto* bb : cfg->getBBs()) {
        auto& instrs = bb->instrs;
        
        for (size_t i = 0; i < instrs.size(); ) {
            auto* instr = instrs[i];
            
            // Look for copy instructions
            if (instr->op == IRInstr::Operation::copy) {
                // Check if copy source == dest
                if (instr->params.size() >= 2 && 
                    instr->params[0] == instr->params[1]) {
                    
                    // Remove redundant copy
                    delete instr;
                    instrs.erase(instrs.begin() + i);
                    modified = true;
                    continue;  // Don't increment i
                }
            }
            ++i;
        }
    }
    
    return modified;
}

} // namespace optim
```

### 3. Add to main.cpp

```cpp
#include "optim/RedundantCopyElimination.h"

int main() {
    // ...
    optim::OptimizationManager optimizer;
    optimizer.addPass(std::make_unique<optim::LoadConstantToRegisterPass>());
    optimizer.addPass(std::make_unique<optim::RedundantCopyEliminationPass>());
    optimizer.runOptimizations(cfg);
    // ...
}
```

---

## Testing Optimizations

### Manual Testing

```bash
# Test a simple case
$ ./ifcc test.c
.globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    subq $16, %rsp
BB0:
    movl $42, %eax      # Should be optimized!
    leave
    ret
```

### Unit Testing Pattern

For each optimization, test:

1. **When optimization should apply**
2. **When optimization should NOT apply**  
3. **Edge cases** (empty blocks, multiple patterns, etc.)

### Test Example

```cpp
// Test for LoadConstantToRegisterPass
void test_load_const_to_reg() {
    // Input: return 42;
    // Expected IR before: ldconst !tmp1, 42; copy !eax, !tmp1
    // Expected IR after: ldconst !eax, 42
    
    // Verify assembly output
    // Expected: movl $42, %eax (NOT: movl $42, -4(%rbp); movl -4(%rbp), %eax)
}
```

---

## Available Optimizations

Currently implemented:

| Pass | Description |
|------|-------------|
| `load-const-to-reg` | Load constants directly to target register |

### Planned Optimizations

| Pass | Description |
|------|-------------|
| `constant-folding` | Evaluate constant expressions at compile time |
| `dead-code-elim` | Remove unreachable code |
| `algebraic-simplify` | Simplify x+0, x*1, etc. |
| `copy-propagation` | Propagate copies to eliminate loads |

---

## Tips for Writing Optimizations

1. **Start simple**: Handle one pattern at a time
2. **Test incrementally**: Verify each change works
3. **Be careful with iterators**: When removing elements, don't increment
4. **Consider edge cases**: What if the block is empty? What if there's only one instruction?
5. **Document your pattern**: Comment what IR pattern you're looking for
6. **Use the timing system**: Put cleanups at LATE, foundational opts at EARLY

---

## Debugging

### Enable Debug Output

In [`LoadConstantToRegister.cpp`](optim/LoadConstantToRegister.cpp):

```cpp
#define DEBUG_OPTIMIZATION
#include <iostream>

// Then in your code:
std::cout << "[MyPass] Optimized: " << details << std::endl;
```

### View IR Before/After

You can add debug printing to see the IR:

```cpp
void printIR(CFG* cfg) {
    for (auto* bb : cfg->getBBs()) {
        std::cout << bb->label << ":\n";
        for (auto* instr : bb->instrs) {
            std::cout << "  " << opToString(instr->op);
            for (auto& p : instr->params) {
                std::cout << " " << p;
            }
            std::cout << "\n";
        }
    }
}
```

---

## Related Files

- [`IR.h`](IR.h) - IR instruction definitions
- [`IR.cpp`](IR.cpp) - IR implementation
- [`CodeGenVisitor.cpp`](CodeGenVisitor.cpp) - IR generation from AST
- [`asm/x86_64/AsmGeneratorX86_64.cpp`](asm/x86_64/AsmGeneratorX86_64.cpp) - Assembly generation
- [`plans/optimization_plan.md`](../plans/optimization_plan.md) - Original design plan
