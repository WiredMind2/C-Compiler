# Build Instructions

This guide covers building the ifcc compiler from source.

## Prerequisites

- **ANTLR4** (installed and configured)
- **C++17** compatible compiler (`g++` or `clang++`)
- **gcc** (for assembling and linking output)
- **Python 3** (for running tests)

See [ANTLR4 Installation](antlr4-installation.md) for ANTLR4 setup.

## Configuration

### Step 1: Copy Configuration Template

Navigate to the compiler directory and copy the appropriate configuration file:

```bash
cd compiler
```

Choose the template that matches your environment:

| Platform | Command |
|----------|---------|
| WSL / Linux | `cp config-wsl-2025.mk config.mk` |
| macOS (Apple Silicon) | `cp config-macos.mk config.mk` |
| INSA Lab Machines | `cp config-IF501.mk config.mk` |

### Step 2: Edit config.mk

Open `config.mk` and verify/adjust the paths:

```makefile
# Path to ANTLR4 JAR
ANTLRJAR=/home/username/antlr4-install/antlr-4.13.2-complete.jar

# Path to ANTLR4 C++ headers
ANTLRINC=/usr/local/include/antlr4-runtime/

# Path to ANTLR4 C++ library
ANTLRLIB=/usr/local/lib/libantlr4-runtime.a
```

Adjust the paths if your installation is in a different location.

## Building

### Build the Compiler

From the project root:

```bash
make
```

Or from the compiler directory:

```bash
cd compiler
make
```

This will:
1. Generate the ANTLR4 parser files (`src/generated/ifcc*.cpp`)
2. Compile all C++ source files
3. Link the final executable: `compiler/ifcc`

### Build Options

| Command | Description |
|---------|-------------|
| `make` | Build the compiler |
| `make clean` | Delete generated files |

### Generated Files

The build process generates these files in `compiler/src/generated/`:

- `ifccLexer.cpp/h` — Tokenizer
- `ifccParser.cpp/h` — Parser
- `ifccVisitor.cpp/h` — Visitor interface
- `ifccBaseVisitor.cpp/h` — Base visitor implementation

## Usage

### Basic Compilation

```bash
# Compile a C file to assembly
./compiler/ifcc input.c > output.s

# Assemble and link with gcc
gcc -o program output.s

# Run and check exit code
./program; echo $?
```

### Example

```bash
# Compile the test file
./compiler/ifcc testfiles/00_base/00_return42.c > output.s

# Assemble and link
gcc -o myprogram output.s

# Run
./myprogram; echo $?
# Output: 42
```

## Visualizing the Parse Tree

The build system includes a GUI for visualizing the ANTLR parse tree:

```bash
make gui FILE=path/to/your/file.c
```

This opens an interactive window showing the parse tree structure.

## Troubleshooting

### "antlr4: command not found"

Ensure ANTLR4 JAR wrapper is installed:

```bash
which antlr4
# Should output: /usr/local/bin/antlr4
```

### "antlr4-runtime.h: No such file"

Check the include path in `config.mk`:

```bash
ls /usr/local/include/antlr4-runtime/
```

### Linking errors with libantlr4-runtime

Check the library path:

```bash
ls /usr/local/lib/libantlr4-runtime*
```

Ensure `config.mk` has the correct `ANTLRLIB` path.

### "nothing to be done for 'all'"

Run `make clean` first, then rebuild:

```bash
make clean
make
```

## Build System Details

### Makefile Structure

The build system is split:

| File | Description |
|------|-------------|
| `Makefile` | Root makefile (runs tests) |
| `compiler/Makefile` | Compiler build |

### Compiler Targets

The compiler Makefile (`compiler/Makefile`) builds these components:

- **Lexer/Parser**: Generated from `ifcc.g4` by ANTLR
- **Visitors**: `CodeGenVisitor` and sub-visitors
- **IR**: Intermediate representation classes
- **ASM Generators**: x86-64 and ARM64 backends

### Object Files

Object files are placed in `compiler/build/`:

```
compiler/build/
├── ifccBaseVisitor.o
├── ifccLexer.o
├── ifccParser.o
├── ifccVisitor.o
├── main.o
├── CodeGenVisitor.o
├── CodeGenArithmetic.o
├── CodeGenBitwise.o
├── CodeGenComparison.o
├── CodeGenMemory.o
├── CodeGenFunction.o
├── IR.o
├── AsmGeneratorX86_64.o
└── AsmGeneratorARM64.o
```

## Testing

After building, run tests:

```bash
make test-all     # Run all tests
make test-01      # Run test suite 01
make test-x86-01  # Run test suite 01 with x86-64
```

See [Testing](testing.md) for more details.
