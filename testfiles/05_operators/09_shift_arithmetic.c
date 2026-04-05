// Shift combined with arithmetic
int main() {
    int x = 1;
    x = x << 5;      // 32
    x = x + (x >> 2); // 32 + 8 = 40
    x = x - (1 << 3); // 40 - 8 = 32
    return x;
}
