//
// Created by dupic on 28/03/2026.
//
#include <stdio.h>
int add(int a, int b) {
    return a + b;
}

int main() {
    int x = 10;
    int y = 20;
    int result = 0;
    {
        int result = add(x, y); // Should call add(10, 20) and return 30
        putchar(result); // Print the result (30)
    }
    return result; // Should return 30
}