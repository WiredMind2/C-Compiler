# Developer Guide

This document describes the project layout and how to modify the compiler.

Repository layout (top-level)
- `compiler/` : compiler sources, ANTLR grammar, and compiler Makefile
- `src/` : (if present) additional project sources or helpers
- `testfiles/` : categorized test suites
- `docs/` : documentation

Key areas inside `compiler/`
- `ifcc.g4` : ANTLR grammar
- `src/visitors/` : AST visitors and codegen
- `src/ir/` : intermediate representation and passes
- `src/optim/` : optimization passes
- `src/asm/` : backend assembly generators (x86_64, arm64)

Building
- Use `make` at project root which calls `make -C compiler ifcc`.
- To rebuild from scratch: `make clean && make` (inside `compiler/` as needed).

Running tests
- Tests are in `testfiles/` grouped by feature.
- Use `python3 ifcc-test.py testfiles/` to run everything.

Extending the compiler
- Add a new optimization pass by creating a pass in `src/optim/` and registering it in the optimizer pipeline.
- To add a new backend, implement an assembler generator under `src/asm/` and wire it to the CLI.

Code style & contributions
- Follow existing C++20 code style in the `compiler/src/` tree.
- Run existing tests after changes to ensure no regressions.

Contact
- See top-level `README.md` for contributor list and repository pointer.
