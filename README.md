# ifcc — Simplified C Compiler

## Contributors

- Elise BACHET
- Clément DUPIC
- Clément GRENNERAT
- Clément JACQUIER
- Léo MARNAS
- William MICHAUD

Compiler built with **ANTLR4** and **C++17**.  
Supports x86-64 (Linux) and ARM64 (macOS).

## What it compiles

A subset of C: `int` variables, arithmetic (`+`, `-`, `*`, `/`, `%`), comparisons (`==`, `!=`), bitwise (`&`, `^`), unary (`-`, `!`), parentheses, and `return`.

```c
int main() {
    int x = 6;
    return x * 7; // returns 42
}
```

## Setup

1. Copy a config template in `compiler/`:

```bash
cd compiler
cp config-wsl-2025.mk config.mk    # or config-IF501.mk / config-macos.mk
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

## Project layout

- `compiler/ifcc.g4` — ANTLR grammar
- `compiler/*.cpp/.h` — compiler source (visitors, IR, symbol table, asm backends)
- `testfiles/` — test programs
- `ifcc-test.py` — test runner
