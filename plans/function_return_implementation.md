# Function Return Implementation Guide

## Overview

This document explains how function returns are handled in this compiler and provides guidance on how to properly implement return statements, including changing `current_bb` to use a getter for the bbStack.

## Current Implementation

### Return Statement Handling

The return statement is handled in [`CodeGenVisitor.cpp:12-17`](compiler/src/CodeGenVisitor.cpp:12):

```cpp
antlrcpp::Any CodeGenVisitor::visitReturn_stmt(ifccParser::Return_stmtContext *ctx)
{
    string var = std::any_cast<string>(this->visit(ctx->expr()));
    cfg->current_bb->add_IRInstr(IRInstr::copy, Type::INT, {"!eax", var});
    return "!eax";
}
```

**Flow:**
1. Evaluate the return expression
2. Generate a `copy` IR instruction to move the value to `!eax` (return register)
3. The assembly epilogue is generated automatically when a basic block has `exit_true == nullptr`

### The bbStack

Defined in [`IR.h:195-196`](compiler/src/IR.h:195):

```cpp
vector<BasicBlock*> bbStack; // the stack of basic blocks
```

**Purpose:** Used when generating IR code from the AST - when entering an `if`, `while`, etc., push the current BB onto the stack; when exiting, pop it from the stack.

**Important:** The bbStack is used for **variable lookup** via [`findBBByVariable()`](compiler/src/IR.cpp:185), NOT for control flow. For return statements, you **do not** need to manually pop from the bbStack - control flow is handled through the CFG's automatic epilogue generation.

---

## Changing current_bb to a Getter

### Current State

Currently in [`IR.h:175`](compiler/src/IR.h:175):
```cpp
BasicBlock* current_bb;  // Public member variable
```

And in [`IR.h:172`](compiler/src/IR.h:172):
```cpp
vector<BasicBlock*>& getStackBBs() { return bbStack; }
```

### Proposed Change

Replace the public `current_bb` variable with a getter method that returns the last BB from the stack:

**In IR.h - Add a new getter method:**
```cpp
// Replace: BasicBlock* current_bb;
// With:
BasicBlock* getCurrentBB() {
    if (!bbStack.empty()) {
        return bbStack.back();
    }
    return nullptr;  // Or handle error
}
```

Or with a setter for backward compatibility:
```cpp
BasicBlock* current_bb;  // Keep for backward compatibility, or deprecate

BasicBlock* getCurrentBB() {
    if (!bbStack.empty()) {
        return bbStack.back();
    }
    return current_bb;  // Fallback to legacy variable
}

void setCurrentBB(BasicBlock* bb) {
    if (!bbStack.empty()) {
        bbStack.back() = bb;
    } else {
        bbStack.push_back(bb);
    }
    current_bb = bb;  // Keep legacy in sync
}
```

### Required Changes Summary

| File | Change |
|------|--------|
| `IR.h` | Add `getCurrentBB()` method that returns `bbStack.back()` |
| `IR.h` | Optionally add `push_bb()` and `pop_bb()` helper methods |
| `CodeGenVisitor.cpp` | Change `cfg->current_bb` to `cfg->getCurrentBB()` |
| All visitor files | Change `cfg->current_bb` to `cfg->getCurrentBB()` |
| `IR.cpp` | Update `create_function_entry` to push BB to stack instead of direct assignment |
| `AsmGeneratorX86_64.cpp` | Change `cfg->current_bb` to `cfg->getCurrentBB()` |

### Helper Methods to Add

Add these to the CFG class in IR.h:
```cpp
void push_bb(BasicBlock* bb) {
    bbStack.push_back(bb);
}

BasicBlock* pop_bb() {
    if (bbStack.empty()) return nullptr;
    BasicBlock* bb = bbStack.back();
    bbStack.pop_back();
    return bb;
}

BasicBlock* getCurrentBB() {
    if (!bbStack.empty()) return bbStack.back();
    return nullptr;
}
```

---

## Implementation Options

### Option 1: Current Approach (Copy to !eax)

Generate a `copy` instruction to move the return value to `!eax`, then rely on automatic epilogue generation:

```cpp
// In CodeGenVisitor.cpp - visitReturn_stmt
string var = std::any_cast<string>(this->visit(ctx->expr()));
cfg->getCurrentBB()->add_IRInstr(IRInstr::copy, Type::INT, {"!eax", var});
cfg->getCurrentBB()->exit_true = nullptr;  // Signal end of function
return "!eax";
```

### Option 2: Explicit Ret Instruction

Use the `IRInstr::ret` operation directly:

```cpp
// In CodeGenVisitor.cpp - visitReturn_stmt
string var = std::any_cast<string>(this->visit(ctx->expr()));
cfg->getCurrentBB()->add_IRInstr(IRInstr::ret, Type::INT, {var});
return "!eax";
```

This requires the `ret` operation to be handled in the assembly generator (already implemented at line 82-84 in AsmGeneratorX86_64.cpp).

## Summary

| Aspect | Current Behavior |
|--------|------------------|
| Return value | Copied to `!eax` register |
| Function exit | Automatic epilogue when `exit_true == nullptr` |
| bbStack | Used for variable lookup, NOT popped for returns |
| Assembly | `leave; ret` generated automatically |
| current_bb | Public member variable |
| New design | `getCurrentBB()` returns `bbStack.back()` |
