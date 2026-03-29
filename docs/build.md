# Build — Makefiles & Useful Targets

This document explains the difference between the two Makefiles in this repository and lists the most useful targets and where to run them.

## High-level difference

- Top-level `Makefile` (repository root)
  - Main purpose: compile and run the tests.
  - Invokes the Python test runner (`ifcc-test.py`) and provides helpers for running suites and renumbering tests.
  - Targets: `test-all`, `test-%`, `test-x86-%`, `test-arm-%`, `renumber`, `clean`.

- `compiler/Makefile` (inside `compiler/`)
  - Purpose: build the compiler executable `ifcc`, generate ANTLR sources, compile and link C++ code.
  - Ttargets: `ifcc` (built by `all`), `gui`, `clean`.


## Target descriptions

### Top-level `Makefile` (project root)

- `make` / `make test-all`
  - Runs the whole test suite: `python3 ifcc-test.py testfiles`.

- `make test-<N>` (e.g. `make test-01`)
  - If `testfiles/<N>` exists, runs `ifcc-test.py testfiles/<N>`; otherwise runs `ifcc-test.py testfiles/<N>_*`.

- `make test-x86-<N>` / `make test-arm-<N>`
  - Run architecture-specific subsets (passes `--arch x86` or `--arch arm` to the test runner).

- `make renumber`
  - Renumbers `.c` test files inside `testfiles/*` directories

- `make clean` (root)
  - Removes `compiler/build`, `generated`, and `compiler/ifcc` (top-level cleanup).


### `compiler/Makefile` (inside `compiler/`)

- `make` / `make all` / `make ifcc`
  - Default build: create the `compiler/ifcc` executable.

- `make gui FILE=path/to/file.c`
  - Display the parse tree

- `make clean` (inside `compiler/`)
  - Removes `build/`, `src/generated`, and `ifcc` (compiler-local cleanup).