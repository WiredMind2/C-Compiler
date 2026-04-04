//@unsupported
// Reason: Initializing a char array directly from a string literal.
int main() {
    char s[4] = "abc";
    return s[2];
}
