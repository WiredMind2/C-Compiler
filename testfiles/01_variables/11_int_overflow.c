int main() {
    int x = 2147483647; // MAX_INT 
    int y = x + 1; // Signed integer overflow
    
    // In C, signed overflow is undefined but typically wraps around to negative in two's complement.
    // GCC usually does this. Let's see if your compiler does the same.
    if (y != -2147483648) return 1;

    int min = -2147483648;
    int res = min - 1;
    if (res != 2147483647) return 2;

    return 0;
}
