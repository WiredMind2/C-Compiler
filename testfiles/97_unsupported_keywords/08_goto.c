//@unsupported
int main() {
    int x = 10;
    if (x == 10) goto my_label;
    return 1;
my_label:
    return 0;
}
