# User Guide

This document explains how to build and use the `ifcc` compiler (user-facing).

Prerequisites
- Linux or WSL
- `gcc` (for assembling/linking tests)
- `make` and a C++ toolchain (for building the compiler)
- `python3` (test driver)
- ANTLR4 (see docs/antlr4-installation.md)

Build

```bash
# From project root
make        # builds the compiler (delegates to compiler/Makefile)
```

Basic usage

```bash
# Compile a C file to assembly
./compiler/ifcc input.c > output.s

# Assemble and link
gcc -o program output.s
./program
# exit code is the program return value
```

Testing

```bash
# Run the provided Python test runner on all tests
python3 ifcc-test.py testfiles/

# Or use the Makefile target
make test
```

Notes
- If using a different C toolchain or OS, set relevant environment variables and consult `docs/build.md` for troubleshooting.
