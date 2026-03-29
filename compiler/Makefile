include config.mk

CC=g++
CCFLAGS=-g -c -std=c++20 -I$(ANTLRINC) -Wno-attributes
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
	build/CodeGenFunction.o \
	build/CodeGenRelational.o \
	build/CodeGenLogical.o \
    build/CodeGenControlFlow.o \
	build/IR.o \
	build/IRInstr.o \
	build/AsmGeneratorX86_64.o \
	build/AsmGeneratorARM64.o \
	build/OptimizationManager.o \
	build/PassRegistry.o \
	build/StoreLoadStackFold.o \
	build/StoreLoadToRegister.o \
	build/DeadRegDefElimination.o \
	build/CopyRegChainPropagation.o \
	build/ConstantPropagation.o \
	build/UnusedVariableElimination.o \
	build/StackLayoutPass.o

ifcc: $(OBJECTS)
	@mkdir -p build
	$(CC) $(LDFLAGS) build/*.o $(ANTLRLIB) -o ifcc

##########################################
# compile our hand-writen C++ code: main(), CodeGenVisitor, etc.
build/%.o: src/%.cpp src/generated/ifccParser.cpp
	@mkdir -p build
	$(CC) $(CCFLAGS) -MMD -o $@ $<

build/%.o: src/ir/%.cpp src/generated/ifccParser.cpp
	@mkdir -p build
	$(CC) $(CCFLAGS) -MMD -o $@ $<

build/%.o: src/visitors/%.cpp src/generated/ifccParser.cpp
	@mkdir -p build
	$(CC) $(CCFLAGS) -MMD -o $@ $<

build/%.o: src/asm/%.cpp src/generated/ifccParser.cpp
	@mkdir -p build
	$(CC) $(CCFLAGS) -MMD -o $@ $<

build/%.o: src/asm/arm64/%.cpp src/generated/ifccParser.cpp
	@mkdir -p build
	$(CC) $(CCFLAGS) -MMD -o $@ $<

build/%.o: src/asm/x86_64/%.cpp src/generated/ifccParser.cpp
	@mkdir -p build
	$(CC) $(CCFLAGS) -MMD -o $@ $<

build/%.o: src/optim/%.cpp src/generated/ifccParser.cpp
	@mkdir -p build
	$(CC) $(CCFLAGS) -MMD -o $@ $<

##########################################
# compile all the antlr-generated C++
build/%.o: src/generated/%.cpp
	@mkdir -p build
	$(CC) $(CCFLAGS) -MMD -o $@ $<

-include build/*.d
build/%.d:

##########################################
src/generated/ifccLexer.cpp: src/generated/ifccParser.cpp
src/generated/ifccVisitor.cpp: src/generated/ifccParser.cpp
src/generated/ifccBaseVisitor.cpp: src/generated/ifccParser.cpp
src/generated/ifccParser.cpp: ifcc.g4
	@mkdir -p src/generated
	java -jar $(ANTLRJAR) -visitor -no-listener -Dlanguage=Cpp -o src/generated ifcc.g4

.PRECIOUS: src/generated/ifcc%.cpp

FILE ?= ../tests/testfiles/00_base/00_return42.c

gui:
	@mkdir -p src/generated build
	java -jar $(ANTLRJAR) -Dlanguage=Java -o src/generated ifcc.g4
	javac -cp $(ANTLRJAR) -d build src/generated/*.java
	java -cp $(ANTLRJAR):build org.antlr.v4.gui.TestRig ifcc axiom -gui $(FILE)

clean:
	rm -rf build src/generated
	rm -f ifcc
