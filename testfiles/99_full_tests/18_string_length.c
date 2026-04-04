// Compute string length manually, return it
int strlen_manual(char *s) {
    int len = 0;
    while (s[len] != 0) {
        len++;
    }
    return len;
}

int main() {
    char *s = "Hello, World!";
    return strlen_manual(s);
}
