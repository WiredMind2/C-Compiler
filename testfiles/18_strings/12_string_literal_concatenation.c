int main() {
    char* s = "A" "B" "C";
    // Many compilers support string literal concatenation like this.
    // Let's see if ifcc handles it.
    return s[0] + s[1] + s[2];
}
