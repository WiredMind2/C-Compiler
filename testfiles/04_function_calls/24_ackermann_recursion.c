int ackermann(int m, int n) {
    if (m == 0) return n + 1;
    if (m > 0 && n == 0) return ackermann(m - 1, 1);
    return ackermann(m - 1, ackermann(m, n - 1));
}

int main() {
    // ackermann(3, 2) = 29
    int res = ackermann(3, 2);
    if (res != 29) return 1;
    return 0;
}
