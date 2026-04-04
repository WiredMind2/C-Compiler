int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    int res = factorial(5); // 120
    if (res != 120) return 1;
    return 0;
}
