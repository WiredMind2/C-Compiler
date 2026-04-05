# ifcc — Simplified C Compiler

A compiler for a subset of C, targeting x86-64 and ARM64 assembly, developed as part of the PLD-Comp project at INSA Lyon (4IF).

## Contributors

- Elise BACHET
- Clément DUPIC
- Clément GRENNERAT
- Clément JACQUIER
- Léo MARNAS
- William MICHAUD

## Repository

This project is tracked on GitHub: https://github.com/WiredMind2/C-Compiler (branch: `main`)

Built with **ANTLR4** and **C++20**.

## Supported Platforms

| Platform | Architecture | Status |
|----------|-------------|--------|
| Linux / WSL | x86-64 | Fully supported |
| macOS (Apple Silicon) | ARM64 | Fully supported |

---

## Quick Start

```bash
# 1. Install ANTLR4 (see docs/antlr4-installation.md)
# 2. Configure compiler/config.mk

# Build the compiler
cd compiler && make

# Compile a C source file to assembly
./compiler/ifcc input.c > output.s

# Assemble, link and run
gcc -o program output.s
./program; echo $?
```

---

## Supported Language Features

### Types

| Type | Size | Notes |
|------|------|-------|
| `int` | 32-bit | Signed integer |
| `char` | 8-bit | Signed character |
| `double` | 64-bit | IEEE 754 double precision |
| `void` | — | For function return types |
| Pointers | 64-bit | e.g. `int*`, `char*` |
| Arrays | fixed size | e.g. `int arr[10]` |

### Operators

| Category | Operators |
|----------|-----------|
| Arithmetic | `+` `-` `*` `/` `%` |
| Unary | `-` `+` `!` `~` `&` `*` `++` `--` (pre/post) |
| Comparison | `==` `!=` `<` `>` `<=` `>=` |
| Logical | `&&` `\|\|` |
| Bitwise | `&` `\|` `^` `~` `<<` `>>` |
| Compound assignment | `+=` `-=` |

### Control Flow

- `if` / `else if` / `else`
- `while`, `do-while`, `for`
- `switch` / `case` / `default`
- `break`, `continue`, `return`

### Functions & Scope

- Function definitions and forward declarations (prototypes)
- Recursive functions
- Multiple parameters with typed arguments
- `putchar` / `getchar` support (with `#include <stdio.h>`)
- Block scoping and variable shadowing
- Global variables
- Void and typed return values

### Semantic Checks

- Undeclared variable usage detection
- Double declaration detection
- Undeclared function call detection

### Other

- Implicit type conversions (`int` / `char` / `double`)
- String literals (stored in data section)
- Multi-line strings (`"hello" " world"`)
- `#include <stdio.h>` / `<stdlib.h>` (enables implicit stdlib declarations)
- Single-line (`//`) and multi-line (`/* */`) comments

---

## Architecture

```
Source (.c)
    │
    ▼
[ANTLR4 Parser]   ← ifcc.g4 grammar
    │  Parse Tree
    ▼
[CodeGenVisitor]  ← AST → 3-address IR
    │  CFG + BasicBlocks
    ▼
[Optimizer]       ← 7 IR-level passes
    │  Optimized IR
    ▼
[AsmGenerator]    ← x86-64 or ARM64
    │
    ▼
Assembly (.s)
```

### Key Components

| Component | Location | Role |
|-----------|----------|------|
| Grammar | `compiler/ifcc.g4` | ANTLR4 grammar for the C subset |
| IR | `compiler/src/ir/` | 3-address intermediate representation (CFG, BasicBlocks, 33+ instructions) |
| Code generation | `compiler/src/visitors/` | AST → IR, split across 9 visitor modules |
| Optimizer | `compiler/src/optim/` | 7 IR optimization passes |
| x86-64 backend | `compiler/src/asm/x86_64/` | AT&T-syntax assembly for Linux/WSL |
| ARM64 backend | `compiler/src/asm/arm64/` | AArch64 assembly for macOS |

### Optimization Passes

The compiler runs 7 IR-level passes before assembly generation:

1. **StackLayoutPass** — allocates stack space and computes variable offsets
2. **StoreLoadStackFoldPass** — eliminates redundant store/load pairs on the stack
3. **StoreLoadToRegisterPass** — promotes frequently used stack slots to registers
4. **ConstantPropagationPass** — constant folding, copy propagation, dead store elimination
5. **DeadRegDefEliminationPass** — removes register definitions that are never read
6. **CopyRegChainPropagationPass** — collapses copy chains (`a = b; c = a` → `c = b`)
7. **UnusedVariableEliminationPass** — removes variables never read from the symbol table

---

## Build

```bash
# Build compiler only
cd compiler && make

# Build and run all tests
make        # from root directory

# Useful make targets (inside compiler/)
make        # build 
make clean  # remove build 
make gui FILE=test.c   # visualize AST
```

For detailed build options and troubleshooting, see [docs/build.md](docs/build.md).

---

## Testing

The test suite uses `ifcc-test.py`, which compiles each test file with both `ifcc` and `gcc`, then compares their exit codes.

```bash
# Run all tests
python3 ifcc-test.py testfiles/

# Run a specific part
python3 ifcc-test.py testfiles/09_conditionals/

# Run a single test
python3 ifcc-test.py testfiles/00_base/00_return42.c

# Options
python3 ifcc-test.py -v testfiles/ # verbose  

python3 ifcc-test.py -a arm testfiles/  # test against ARM64 backend
```

### Test Categories

| Category | Coverage |
|----------|----------|
| `00_base` | Return values, basic constants |
| `01_variables` | Declarations, initialization |
| `02_preprocessor` | Comments, directives |
| `03_arithmetic_mul_div_add_sub` | Arithmetic, precedence, associativity |
| `04_function_calls` | Calls, parameters, nesting |
| `05_operators` | Modulo, relational, logical, bitwise, shifts |
| `06_double` | Double constants, operations, conversions |
| `07_char` | Char constants and arithmetic |
| `08_type_conversions` | Implicit/explicit type conversions |
| `09_conditionals` | if/else, nested conditions |
| `10_loops` | while, do-while, for, nested loops |
| `11_break_continue` | break/continue in loops |
| `12_compound_ops` | `+=`, `-=`, `++`, `--` |
| `13_function_return_types` | Functions with various return types |
| `14_switch_case` | switch/case/default/break |
| `15_scopes` | Nested scopes, variable shadowing |
| `16_pointers` | Pointer ops, dereferencing, arithmetic |
| `17_arrays` | Arrays, indexing, pointer interaction |
| `18_strings` | String literals, indexing, escapes |
| `98_invalid_tests` | Error cases (should be rejected) |
| `99_full_tests` | Integration: fibonacci, sorting, GCD, Floyd-Warshall… |

For more details on the test framework, see [docs/testing.md](docs/testing.md).

---

## Documentation

| Document | Description |
|----------|-------------|
| [docs/antlr4-installation.md](docs/antlr4-installation.md) | How to install ANTLR4 on Ubuntu/WSL |
| [docs/build.md](docs/build.md) | Build configuration, targets, troubleshooting |
| [docs/language.md](docs/language.md) | Supported C language features and syntax |
| [docs/architecture.md](docs/architecture.md) | Compiler design and internal components |
| [docs/testing.md](docs/testing.md) | Test framework usage and conventions |
