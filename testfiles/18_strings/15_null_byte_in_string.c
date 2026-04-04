int main() {
    char* s = "foo\0bar"; // Contains null byte
    return s[4]; // Should be 'b'
}
