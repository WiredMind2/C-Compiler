#include <stdio.h>
int read_int() {
    int c;
    int n;

    n = 0;
    c = getchar();

    while (c >= '0') {
        if (c > '9') {
            break;
        }
        n = n * 10 + (c - '0');
        c = getchar();
    }

    while (c != '\n') {
        c = getchar();
    }

    return n;
}

int main() {
    int secret;
    int guess;

    secret = 42;

    putchar('G');
    putchar('u');
    putchar('e');
    putchar('s');
    putchar('s');
    putchar(' ');
    putchar('n');
    putchar('u');
    putchar('m');
    putchar('b');
    putchar('e');
    putchar('r');
    putchar(' ');
    putchar('1');
    putchar('-');
    putchar('1');
    putchar('0');
    putchar('0');
    putchar(':');
    putchar('\n');

    while (1) {
        putchar('>');
        putchar(' ');
        guess = read_int();

        if (guess < secret) {
            putchar('T');
            putchar('o');
            putchar('o');
            putchar(' ');
            putchar('s');
            putchar('m');
            putchar('a');
            putchar('l');
            putchar('l');
            putchar('\n');
        } else {
            if (guess > secret) {
                putchar('T');
                putchar('o');
                putchar('o');
                putchar(' ');
                putchar('h');
                putchar('i');
                putchar('g');
                putchar('h');
                putchar('\n');
            } else {
                putchar('W');
                putchar('i');
                putchar('n');
                putchar('!');
                putchar('\n');
                break;
            }
        }
    }

    return 0;
}