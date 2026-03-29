# ifcc — Simplified C Compiler

## Contributors

- Elise BACHET
- Clément DUPIC
- Clément GRENNERAT
- Clément JACQUIER
- Léo MARNAS
- William MICHAUD

Compiler built with **ANTLR4** and **C++20**.

## Supported Platforms

- **x86-64**: Linux, WSL (Windows Subsystem for Linux)
- **ARM64**: macOS with Apple Silicon

## Installation

For ANTLR installation and setup, see [ANTLR installation](docs/antlr4-installation.md).
Once you have ANTLR installed and `compiler/config.mk` configured, you can build ifcc.

## Build

Quick start:

```bash
cd compiler && make   # generate the binary compiler/ifcc
```

If you want to compile and run all the tests at once, just run:

```bash
make    # from root directory!
```

For more in-depth on the building process/targets/options, see [build documentation](docs/build.md).

## Usage

```bash
# Compile a C file to assembly
./compiler/ifcc input.c > output.s

# Assemble and link
gcc -o program output.s

# Run and check return code
./program; echo $?
```

## Documentation

You will find in the `docs/` folder the following files:

| Document | Description |
|----------|-------------|
| [Installation Guide](antlr4-installation.md) | How to install ANTLR4 on Ubuntu/WSL |
| [Build Instructions](build.md) | Detailed build configuration and troubleshooting |
| [Language Reference](language.md) | Supported C language features and syntax |
| [Architecture](architecture.md) | Compiler design and internal components |
| [Testing](testing.md) | Test framework and running tests |

