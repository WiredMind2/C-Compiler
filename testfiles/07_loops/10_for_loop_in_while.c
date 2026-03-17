int main() {
    int n;
    int i;
    int total;
    int guard;
    n = 0;
    total = 0;
    guard = 0;

    while (n < 8) {
        for (i = 0; i < 8; i = i + 1) {
            if (i == n) {
                break;
            }
            if (i % 3 == 0) {
                total = total + 2;
            } else {
                total = total + 1;
            }
            guard = guard + 1;
            if (guard > 20) {
                break;
            }
        }

        if (guard > 20) {
            break;
        }

        n = n + 2;
    }

    return total; // 24
}
