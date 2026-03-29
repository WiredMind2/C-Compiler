# Language Reference

This document describes the C language subset supported by the ifcc compiler.

## Supported Types

| Type | Description |
|------|-------------|
| `int` | 32-bit signed integer |
| `double` | 64-bit floating-point (double) |
| `char` | 8-bit integer (character) |

Currently, `char` is not supported as function return (not tested).

## Variables

### Declaration

```c
int x;
int y, z;
```

Multiple variables can be declared in a single statement.

### Declaration with Initialization

```c
int x = 42;
int y = x + 1;
```

### Assignment

```c
x = 10;
y = x * 2;
```

## Arithmetic Operators

| Operator | Description |
|----------|-------------|
| `+` | Addition |
| `-` | Subtraction |
| `*` | Multiplication |
| `/` | Division (integer) |
| `%` | Modulo (remainder) |

### Precedence

Operator precedence follows standard C rules (highest to lowest):

1. Parentheses: `(expr)`
2. Unary: `-`, `+`, `!`
3. Multiplicative: `*`, `/`, `%`
4. Additive: `+`, `-`
5. Bitwise AND: `&`
6. Bitwise XOR: `^`

### Associativity

- Arithmetic operators are left-associative
- Parentheses can override precedence

### Examples

```c
int x = 6;
int y = 7;
int z = x * y;        // 42
int a = x + y * 2;    // 20 (not 26)
int b = (x + y) * 2;  // 26
```

## Unary Operators

| Operator | Description |
|----------|-------------|
| `-` | Negation |
| `+` | Unary plus |
| `!` | Logical NOT |

- `-x` returns the negation of x
- `!x` returns 1 if x is 0, otherwise 0

## Comparison Operators

| Operator | Description |
|----------|-------------|
| `==` | Equal |
| `!=` | Not equal |

Comparisons return 1 (true) or 0 (false).

```c
int a = 5 == 5;  // 1
int b = 5 != 5;  // 0
```

## Bitwise Operators

| Operator | Description |
|----------|-------------|
| `&` | Bitwise AND |
| `^` | Bitwise XOR |

```c
int a = 5 & 3;   // 1  (0101 & 0011 = 0001)
int b = 5 ^ 3;   // 6  (0101 ^ 0011 = 0110)
```

## Control Flow

### If/Else

```c
if (condition) {
    // statements
} else {
    // statements
}
```

- The `else` clause is optional
- Condition is any expression (0 = false, non-zero = true)

### While Loop

```c
while (condition) {
    // statements
}
```

### For Loop

```c
for (init; condition; update) {
    // statements
}
```

All three parts of the for loop are expressions:

```c
int i;
for (i = 0; i < 10; i = i + 1) {
    // statements
}
```

## Functions

### Function Definition

```c
int main() {
    int x = 5;
    return x;
}
```

### Function Declaration (Prototype)

```c
int foo(int a, int b);
```

### Function Call

```c
int result = foo(1, 2);
```

### Return

```c
return expression;
```

The return value can be `int`, `double`.
Empty return in `void` function is supported.

## Comments

Multi-line comments are supported:

```c
/* This is a comment
   spanning multiple lines */
int x = 1; // This is also a comment
```

## Preprocessor Directives

Preprocessor directives are parsed but skipped (not processed):

```c
#include <stdio.h>  /* ignored */
#define VALUE 42    /* ignored */
```

These are stripped from the input before compilation.

## Scope

Variables are scoped to their containing block:

```c
int main() {
    int x = 1;
    {
        int y = 2;  // y is only visible in this block
    }
    // y is not visible here
    return x;
}
```

## Limitations

- Supported types: `int`, `double`, `char`, no `float` or unsigned types.
- No standard library functions provided by the compiler (link externally)
- No bit-shift operators (`<<`, `>>`)