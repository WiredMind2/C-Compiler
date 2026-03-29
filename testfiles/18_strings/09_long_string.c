int main() {
    char* s = "this is a very long string used to test if the compiler can handle long string literals securely without overflowing some internal buffer during assembly generation.";
    return s[10];
}
