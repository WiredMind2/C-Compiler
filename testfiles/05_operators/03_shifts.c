/* Test shift operators */
int main() {
    int a = 1;
    int b = 2;
    int c = -1;

    int r1 = a << 3;     // 8
    int r2 = b << 1;     // 4
    int r3 = 16 >> 2;    // 4
    int r4 = c >> 1;     // arithmetic shift -> -1 on two's complement

    return r1 + r2 + r3 + (r4 == -1);
}
