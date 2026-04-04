int main() {
    int x = 0xAA; // 170
    int y = 0x55; // 85
    int z = x ^ y; // should be 0xFF (255)
    if (z != 255) return 1;
    return 0;
}
