int main() {
    char* s1 = "abc";
    char* s2 = "abc";
    char* s3 = "def";
    // Checks if the compiler correctly deduplicates strings (or at least handles identical ones)
    // and correctly differentiates between different ones.
    if (s1[0] == s2[0] && s1[2] == s2[2] && s1[0] != s3[0]) {
        return 1;
    }
    return 0;
}
