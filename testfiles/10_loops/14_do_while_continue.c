#include <stdio.h>

int main() {
    int x = 0;
    int y = 0;
    do {
        x = x + 1;
        if (x == 3) {
            continue;
        }
        y = y + 2;
    } while(x < 5);
    return y;
}
