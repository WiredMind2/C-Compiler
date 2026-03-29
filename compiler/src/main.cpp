#include <iostream>
#include <fstream>
#include <sstream>

#include "antlr4-runtime.h"
#include "generated/ifccLexer.h"
#include "generated/ifccParser.h"
#include "optim/OptimizationManager.h"
#include "optim/LoadConstantToRegister.h"
#include "optim/ConstantPropagation.h"
#include "optim/StoreLoadStackFold.h"
#include "optim/StoreLoadToRegister.h"

#include "visitors/CodeGenVisitor.h"
#include "ir/IR.h"

using namespace antlr4;
using namespace std;

#ifdef __APPLE__
TargetArch DEFAULT_ARCH = TargetArch::ARM64;
#else
TargetArch DEFAULT_ARCH = TargetArch::X86_64;
#endif

int main(int argn, const char **argv) {
    stringstream in;
    TargetArch arch = DEFAULT_ARCH;
    const char *filename = nullptr;
    bool no_optim = false;

    for (int i = 1; i < argn; i++) {
        string arg = argv[i];
        if (arg == "--arch" && i + 1 < argn) {
            string archStr = argv[++i];
            if (archStr == "arm") {
                arch = TargetArch::ARM64;
            } else if (archStr == "x86") {
                arch = TargetArch::X86_64;
            }
        } else if (arg == "--nooptim") {
            no_optim = true;
        } else if (filename == nullptr) {
            filename = argv[i];
        }
    }
    if (filename == nullptr) {
        cerr << "usage: ifcc [--arch arm|x86, --nooptim] path/to/file.c" << endl;
        exit(1);
    }

    ifstream lecture(filename);
    if (!lecture.good()) {
        cerr << "error: cannot read file: " << filename << endl;
        exit(1);
    }
    in << lecture.rdbuf();

    ANTLRInputStream input(in.str());

    ifccLexer lexer(&input);
    CommonTokenStream tokens(&lexer);

    tokens.fill();

    ifccParser parser(&tokens);
    tree::ParseTree *tree = parser.axiom();

    if (parser.getNumberOfSyntaxErrors() != 0) {
        cerr << "error: syntax error during parsing" << endl;
        exit(1);
    }

    // Detect simple #include directives in the raw source so we can
    // enable implicit declarations of standard functions only when
    // the corresponding header was included.
    std::string src = in.str();
    bool include_stdio = (src.find("#include <stdio.h>") != std::string::npos) || (src.find("#include<stdio.h>") != std::string::npos);
    bool include_stdlib = (src.find("#include <stdlib.h>") != std::string::npos) || (src.find("#include<stdlib.h>") != std::string::npos);

    CodeGenVisitor v(arch, include_stdio, include_stdlib);
    v.visit(tree);
    CFG *cfg = v.getCFG();

    // Run optimizations
    optim::OptimizationManager optimizer;
        optimizer.addPass(std::make_unique<optim::StoreLoadStackFoldPass>());
    if (!no_optim) {
        optimizer.addPass(std::make_unique<optim::StoreLoadToRegisterPass>());
        // optimizer.addPass(std::make_unique<optim::LoadConstantToRegisterPass>());
        // optimizer.addPass(std::make_unique<optim::ConstantPropagationPass>());
    }
    optimizer.runOptimizations(cfg);

    cfg->gen_asm(cout);

    return 0;
}
