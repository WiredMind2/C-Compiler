#include <stdio.h>
void test() {
    double b = 3.3;
}

int main() {
    // The following should produce an error and not a bad cast
    if (test()) {
        putchar(72);
    }
    return 0;
}