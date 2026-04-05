int main() {
    int x = (1 << 3); // 8
    int y = (64 >> 2); // 16
    
    if (x != 8) return 1;
    if (y != 16) return 2;
    return 0;
}
