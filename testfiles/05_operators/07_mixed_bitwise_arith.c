// Mix of bitwise and arithmetic operators with correct precedence
int main() {
    int a = 12;  // 0b1100
    int b = 10;  // 0b1010
    int c = a & b;   // 0b1000 = 8
    int d = a | b;   // 0b1110 = 14
    int e = a ^ b;   // 0b0110 = 6
    int f = c + d - e; // 8 + 14 - 6 = 16
    return f;
}
