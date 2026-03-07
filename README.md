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
- `DeclarationVisitor` — first pass: builds the symbol table
- `CodeGenVisitor` — second pass: generates IR from the AST
- `IR` (`CFG`, `BasicBlock`, `IRInstr`) — intermediate representation with 3-address instructions
- `AsmGenerator` — architecture-specific assembly backend (x86-64 / ARM64)
- `SymbolTable` — maps variable names to stack offsets and types

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
./compiler/ifcc myfile.c > out.s    # compile to assembly
gcc -o out out.s                    # assemble + link
./out; echo $?                      # run and print return code
```

## Tests

```bash
python3 ifcc-test.py testfiles/     # run all tests (compares ifcc vs gcc)
```

The test runner compiles each `.c` file with both `ifcc` and `gcc`, then compares the return codes. A test passes if both return the same value.

**Test categories:**

| Folder | Description |
|--------|-------------|
| `testfiles/00_base/` | Basic return values |
| `testfiles/01_variables/` | Variable declarations and assignments |
| `testfiles/02_excepted_fail/` | Programs that should fail to compile |
| `testfiles/03_arithmetic_expr/` | Arithmetic expressions and operator precedence |