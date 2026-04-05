int main() {
    int a, b;
    a = (b = 3, b + 2); // a should be 5
    if (a != 5) return 1;
    if (b != 3) return 2;
    return 0;
}
