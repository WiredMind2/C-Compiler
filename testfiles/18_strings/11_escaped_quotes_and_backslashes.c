int main() {
    char* s = "Hello, \"World\"!\n\t\\";
    return s[7]; // Should be '"' (double quote)
}
