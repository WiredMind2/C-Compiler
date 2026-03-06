# config.mk contains the paths to antlr4 etc.
# Each student should have a config.mk corresponding to her system.
# Examples are  ubuntu.mk, DI.mk, fedora.mk
# Then config.mk should be in the .gitignore of your project
include config.mk

CC=g++
# Platform detection: Add -D__APPLE__ when building on macOS (Darwin)
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CCFLAGS=-g -c -std=c++17 -I$(ANTLRINC) -Wno-attributes -D__APPLE__
else
    CCFLAGS=-g -c -std=c++17 -I$(ANTLRINC) -Wno-attributes
endif
LDFLAGS=-g

default: all
all: ifcc

##########################################
# link together all pieces of our compiler
OBJECTS=build/ifccBaseVisitor.o \
	build/ifccLexer.o \
	build/ifccVisitor.o \
	build/ifccParser.o \
	build/main.o \
	build/CodeGenVisitor.o \
	build/CodeGenArithmetic.o \
	build/CodeGenBitwise.o \
	build/CodeGenComparison.o \
	build/CodeGenMemory.o \
	build/DeclarationVisitor.o \
	build/SymbolTable.o \
	build/IR.o \
	build/AsmGeneratorX86_64.o \
	build/AsmGeneratorARM64.o

ifcc: $(OBJECTS)
	@mkdir -p build
	$(CC) $(LDFLAGS) build/*.o $(ANTLRLIB) -o ifcc

##########################################
# compile our hand-writen C++ code: main(), CodeGenVisitor, etc.
build/%.o: %.cpp generated/ifccParser.cpp
	@mkdir -p build
	$(CC) $(CCFLAGS) -MMD -o $@ $<

build/%.o: visitors/%.cpp generated/ifccParser.cpp
	@mkdir -p build
	$(CC) $(CCFLAGS) -MMD -o $@ $<

build/%.o: asm/x86_64/%.cpp generated/ifccParser.cpp
	@mkdir -p build
	$(CC) $(CCFLAGS) -MMD -o $@ $<

build/%.o: asm/arm64/%.cpp generated/ifccParser.cpp
	@mkdir -p build
	$(CC) $(CCFLAGS) -MMD -o $@ $<

##########################################
# compile all the antlr-generated C++
build/%.o: generated/%.cpp
	@mkdir -p build
	$(CC) $(CCFLAGS) -MMD -o $@ $<

# automagic dependency management: `gcc -MMD` generates all the .d files for us
-include build/*.d
build/%.d:

##########################################
# generate the C++ implementation of our Lexer/Parser/Visitor
generated/ifccLexer.cpp: generated/ifccParser.cpp
generated/ifccVisitor.cpp: generated/ifccParser.cpp
generated/ifccBaseVisitor.cpp: generated/ifccParser.cpp
generated/ifccParser.cpp: ifcc.g4
	@mkdir -p generated
	java -jar $(ANTLRJAR) -visitor -no-listener -Dlanguage=Cpp -o generated ifcc.g4

# prevent automatic cleanup of "intermediate" files like ifccLexer.cpp etc
.PRECIOUS: generated/ifcc%.cpp

##########################################
# view the parse tree in a graphical window

# Usage: `make gui FILE=path/to/your/file.c`
FILE ?= ../tests/testfiles/1_return42.c

gui:
	@mkdir -p generated build
	java -jar $(ANTLRJAR) -Dlanguage=Java -o generated ifcc.g4
	javac -cp $(ANTLRJAR) -d build generated/*.java
	java -cp $(ANTLRJAR):build org.antlr.v4.gui.TestRig ifcc axiom -gui $(FILE)

##########################################
# delete all machine-generated files
clean:
	rm -rf build generated
	rm -f ifcc
