// Locals in called function do not affect caller's locals
int compute(int n) {
    int result = n * n;
    int x = result + 1;
    return x;
}

int main() {
    int x = 5;
    int y = compute(3);
    return x + y; // 5 + 10 = 15
}
