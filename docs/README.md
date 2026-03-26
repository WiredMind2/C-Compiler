# ifcc — Simplified C Compiler

A C compiler built with **ANTLR4** and **C++17** that compiles a subset of the C language to x86-64 or ARM64 assembly.

## Contributors

- Clément DUPIC
- Clément GRENNERAT
- Clément JACQUIER
- Léo MARNAS
- William MICHAUD
- Elise BACHET

## Quick Start

### Installation

1. **Install ANTLR4** — See [ANTLR4 Installation Guide](antlr4-installation.md) for detailed instructions

2. **Configure the build**:
   ```bash
   cd compiler
   cp config-wsl-2025.mk config.mk    # WSL / Linux
   # OR
   cp config-macos.mk config.mk       # macOS / Apple Silicon
   # OR
   cp config-IF501.mk config.mk       # INSA lab machines
   ```

3. **Build the compiler**:
   ```bash
   make
   ```

### Usage

```bash
# Compile a C file to assembly
./compiler/ifcc input.c > output.s

# Assemble and link
gcc -o program output.s

# Run and check return code
./program; echo $?
```

## Documentation

| Document | Description |
|----------|-------------|
| [Installation Guide](antlr4-installation.md) | How to install ANTLR4 on Ubuntu/WSL |
| [Build Instructions](build.md) | Detailed build configuration and troubleshooting |
| [Language Reference](language.md) | Supported C language features and syntax |
| [Architecture](architecture.md) | Compiler design and internal components |
| [Testing](testing.md) | Test framework and running tests |

## Supported Platforms

- **x86-64**: Linux, WSL (Windows Subsystem for Linux)
- **ARM64**: macOS with Apple Silicon

## Supported Language Features

- Variable declaration and assignment
- Arithmetic operations: `+`, `-`, `*`, `/`, `%`
- Unary operators: `-`, `+`, `!`
- Comparison operators: `==`, `!=`
- Bitwise operators: `&`, `^`
- Control flow: `if/else`, `while`, `for`
- Functions: definition, declaration, and calls
- Comments and preprocessor directives (skipped)

See [Language Reference](language.md) for the complete feature list.
