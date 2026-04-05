int main() {
    int x = -42;
    int y = +21;
    int z = ~0; // Two's complement not

    if (x != (0-42)) return 1;
    if (y != 21) return 2;
    if (z != -1) return 3;
    return 0;
}
