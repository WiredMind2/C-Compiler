int putchar(int c);
int main() {
    char* s = "hello";
    int i = 0;
    while(s[i] != 0) {
        putchar(s[i]);
        i = i + 1;
    }
    return i;
}
