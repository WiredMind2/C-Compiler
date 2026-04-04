int main() {
    int x = -1; // All ones in two's complement: 0xFFFFFFFF
    
    // Test logical (unsigned) vs arithmetic (signed) shift right.
    // In C, sign extension for signed right shift is implementation-defined but
    // IFCC should ideally match GCC.
    int y = x >> 1; 

    // GCC (signed) shift right for -1 should still be -1.
    if (y != -1) return 1;

    int z = 1 << 31; // Becomes INT_MIN 
    if (z != -2147483648) return 2;

    return 0;
}
