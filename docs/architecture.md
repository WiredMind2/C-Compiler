# Compiler Architecture

This document describes briefly the internal design and components of the ifcc C compiler.

## Overview

The ifcc compiler follows a classic compiler pipeline architecture:

```
Source Code (C)
      │
      ▼
┌─────────────────┐
│  ANTLR4 Parser  │  (Lexer + Parser)
│   ifcc.g4       │  Generates AST
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ CodeGenVisitor  │  AST → IR
│ (Visitor Pattern)│ Transforms AST to 3-address IR
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│      IR         │  Intermediate Representation
│  (CFG + BBs)    │  Control Flow Graph with Basic Blocks
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ AsmGenerator    │  IR → Assembly
│ x86_64 / ARM64  │  Architecture-specific code generation
└────────┬────────┘
         │
         ▼
  Assembly Output
```

## Components

### 1. ANTLR4 Grammar ([`compiler/ifcc.g4`](../compiler/ifcc.g4))

The grammar file defines the lexical rules and parsing structure for the C subset.

### 2. CodeGenVisitor ([`compiler/src/CodeGenVisitor.h`](../compiler/src/visitors/CodeGenVisitor.h))

The main visitor class that traverses the AST and generates Intermediate Representation (IR).

**Key responsibilities:**
- Visits parse tree nodes
- Creates IR instructions for each language construct
- Add variables to symbol table (variable names, types, offsets)
- Handles control flow (if/while/for)

**Sub-visitors** (in [`compiler/src/visitors/`](../compiler/src/visitors/)):
- `CodeGenArithmetic` — +, -, *, /, %
- `CodeGenBitwise` — &, ^, ~
- `CodeGenComparison` — ==, !=
- `CodeGenMemory` — Variable access
- `CodeGenFunction` — Function calls and definitions

### 3. Intermediate Representation ([`compiler/src/IR.h`](../compiler/src/ir/IR.h))

The IR uses a **Control Flow Graph (CFG)** with **Basic Blocks** and **3-address instructions**.

#### CFG (Control Flow Graph)
Represents a function with:
- Multiple basic blocks
- Entry and exit blocks
- Function signatures

#### BasicBlock
A sequence of instructions with:
- No jumps (except at end)
- Single entry, single exit
- Contains IR instructions
- Manages its own symbol table

#### IRInstr (3-address instructions)
| Operation | Description | Parameters |
|-----------|-------------|------------|
| `ldconst` | Load constant | `d, c` |
| `copy_reg` | Copy register | `d, s` |
| `store_stack` | Store register to stack | `d, s` |
| `load_stack` | Load stack slot to register | `d, s` |
| `add` | Addition | `d, x, y` |
| `sub` | Subtraction | `d, x, y` |
| `mul` | Multiplication | `d, x, y` |
| `div` | Division | `d, x, y` |
| `mod` | Modulo | `d, x, y` |
| `bit_not` | Bitwise NOT | `d, s` |
| `bit_and` | Bitwise AND | `d, x, y` |
| `bit_or` | Bitwise OR | `d, x, y` |
| `bit_xor` | Bitwise XOR | `d, x, y` |
| `cmp_eq` | Compare equal | `d, x, y` |
| `cmp_lt` | Compare less | `d, x, y` |
| `cmp_le` | Compare less or equal | `d, x, y` |
| `cmp_gt` | Compare greater | `d, x, y` |
| `cmp_ge` | Compare greater or equal | `d, x, y` |
| `logical_and` | Logical AND | `d, x, y` |
| `logical_or` | Logical OR | `d, x, y` |
| `call` | Function call | `label, d, params` |
| `ret` | Return value | `v` |

### 4. Assembly Generators

#### AsmGenerator Base Class ([`compiler/src/asm/AsmGenerator.h`](../compiler/src/asm/AsmGenerator.h))

Abstract interface defining the contract for architecture-specific code generation.

#### x86_64 Generator ([`compiler/src/asm/x86_64/AsmGeneratorX86_64.h`](../compiler/src/asm/x86_64/AsmGeneratorX86_64.h))

Generates x86-64 (64-bit Intel/AMD) assembly using rax, rbx, rcx, rdx, rsi, rdi, rbp, rsp registers.


#### ARM64 Generator ([`compiler/src/asm/arm64/AsmGeneratorARM64.h`](../compiler/src/asm/arm64/AsmGeneratorARM64.h))

Generates ARM64 (Apple Silicon) assembly using x0-x30, sp registers.

## Symbol Table

The symbol table is managed at two levels:

1. **BasicBlock Level**: Maps variable names to stack offsets
2. **CFG Level**: Manages function signatures

Variables are allocated on the stack with negative offsets from `rbp` (x86) or `fp` (ARM).