# ifcc — Simplified C Compiler

## Contributors

- Elise BACHET
- Clément DUPIC
- Clément GRENNERAT
- Clément JACQUIER
- Léo MARNAS
- William MICHAUD

Compiler built with **ANTLR4** and **C++17**.
Supports x86-64 (Linux/WSL) and ARM64 (macOS).

## What it compiles

A subset of C: `int` variables, arithmetic, comparisons, bitwise operations, and `return`.

**Supported language features:**

| Feature | Syntax |
|---------|--------|
| Variable declaration | `int x;` / `int x, y;` |
| Declaration + init | `int x = expr;` |
| Assignment | `x = expr;` |
| Arithmetic | `+`, `-`, `*`, `/`, `%` |
| Unary operators | `-x`, `+x`, `!x` |
| Comparisons | `==`, `!=` |
| Bitwise | `&`, `^` |
| Parentheses | `(expr)` |
| Return | `return expr;` |
| Comments | `/* ... */` |
| Preprocessor directives | `#include`, `#define` (skipped) |

```c
int main() {
    int x = 6;
    int y = 7;
    return x * y; // returns 42
}
```

**Key components:**

- `ifcc.g4` — ANTLR4 grammar defining the language
- `CodeGenVisitor` — Generates IR from the AST
- `IR` (`CFG`, `BasicBlock`, `IRInstr`) — intermediate representation with 3-address instructions
- `AsmGenerator` — architecture-specific assembly backend (x86-64 / ARM64)

## Dependencies

- **ANTLR4** runtime (jar + C++ runtime headers/library)
- **C++17** compatible compiler (`g++` or `clang++`)
- **Python 3** (for the test runner)
- **gcc** (for assembling and linking the output)

## Setup

1. Copy a config template in `compiler/`:

```bash
cd compiler
cp config-wsl-2025.mk config.mk    # WSL / Linux
# cp config-IF501.mk config.mk     # INSA lab machines
# cp config-macos.mk config.mk     # macOS / Apple Silicon
```

2. Edit `config.mk` to set the correct paths for `ANTLRJAR`, `ANTLRINC`, `ANTLRLIB`.

3. Build:

```bash
make
```

## Usage

```bash
./compiler/ifcc ./testfiles/00_base/00_return_42.c > 00_return_42.s   # compile to assembly
gcc -o 00_return_42 00_return_42.s                                    # assemble + link
./00_return_42; echo $?                                               # run and print return code
```

## Tests

```bash
make test-all     # run all tests (compares ifcc vs gcc)
make test-01      # run only the test suite 01
make test-x86-01  # run only the test suite 01 with x86-64 target architecture
```

