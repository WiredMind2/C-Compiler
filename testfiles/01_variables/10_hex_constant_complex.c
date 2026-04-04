int main() {
    int x = 0x12345678;
    int y = 0xABCDEF01;
    int z = x ^ y; 
    // Manual check: 0x12345678 ^ 0xABCDEF01 = 0xB9F9B979
    if (z != 0xB9F9B979) return 1;
    
    int w = 0xDeadBeef;
    if (w != -559038737) return 2; // Signed 32-bit interpretation
    
    return 0;
}
