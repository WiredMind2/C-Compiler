int main() {
    int x = 10;
    int a = x++; // a=10, x=11
    int b = ++x; // b=12, x=12
    int c = x--; // c=12, x=11
    int d = --x; // d=10, x=10

    if (a != 10) return 1;
    if (b != 12) return 2;
    if (c != 12) return 3;
    if (d != 10) return 4;
    return 0;
}
