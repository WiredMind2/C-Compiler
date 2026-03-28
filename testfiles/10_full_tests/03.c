void print_int(int x) {
    if (x < 0) {
        putchar('-');
        x = -x;
    }
    if (x / 10 != 0) print_int(x / 10);
    putchar(x % 10 + '0');
}
int main() {
    print_int(-273);
    putchar(10);  // newline
    return 0;
}