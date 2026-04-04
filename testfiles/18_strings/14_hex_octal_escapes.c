int main() {
    char* s = "A\x41\x42\n\077"; // Hex and Octal
    return s[1] + s[2] + s[4]; // 'A' + 'B' + '?'
}
