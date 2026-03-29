# Testing Guide

This document explains the test framework and how to run tests for the ifcc compiler.

## Test Structure

Tests are organized in the `testfiles/` directory:

```
testfiles/
├── 00_base/           # Basic tests
│   ├── 00_return42.c
│   ├── 01_return_zero.c
│   └── 02_invalid_program.c
├── 01_variables/       # Variable tests
├── 02_preprocessor/   # Preprocessor tests
├── 03_arithmetic_*/   # Arithmetic tests
└── ...
```

Each test directory contains C source files that test specific language features.

## Running Tests

### Run All Tests

From project root directory:

```bash
make test-all
```

This runs the test runner (`ifcc-test.py`) on all test files in `testfiles/`.

### Run Specific Test Suite

```bash
make test-01          # Run test suite 01 (variables)
make test-02          # Run test suite 02 (preprocessor)
make test-03          # Run test suite 03 (arithmetic)
```

### Run Tests for Specific Architecture

```bash
make test-x86-01     # Run test suite 01 with x86-64 target
make test-arm-01     # Run test suite 01 with ARM64 target
```

### Run one specific test file

```bash
make path/to/test_file.c
```

## How Tests Work

### Test Runner (`ifcc-test.py`)

The test runner is a Python script that:

1. Compiles each test file using `ifcc`
2. Assembles and links the output with gcc
3. Runs the compiled program
4. Compares the exit code with the expected result

### Test File Naming

Test files are named with a prefix indicating expected behavior:

| Prefix | Description |
|--------|-------------|
| `00_*` to `NN_*` | Ordered test number |

### Expected Results

The test runner expects:
- Valid programs should return 0 for success
- Each test file may have an associated `.expected` file with the expected return code

## Adding New Tests

Add a new C file in the appropriate test directory.


## Manual Testing

### Compile and Run Manually

```bash
# Compile
./compiler/ifcc testfiles/00_base/00_return42.c > output.s

# Assemble and link
gcc -o myprogram output.s

# Run
./myprogram; echo $?
```

### Compare with GCC

To verify correctness, compare the output with gcc:

```bash
# Compile with ifcc
./compiler/ifcc test.c > ifcc_output.s
gcc -o ifcc_program ifcc_output.s

# Compile with gcc
gcc -o gcc_program test.c

# Run both
./ifcc_program; echo $?       # ifcc return code
./gcc_program; echo $?        # gcc return code
```

## Renumbering Tests

To automatically renumber test files in all directories:

```bash
make renumber
```

This ensures tests are numbered sequentially within each directory.

## Architecture-Specific Tests

You can force the target platform (x86-64 or ARM64):

```bash
make test-x86-01

make test-arm-01
```
