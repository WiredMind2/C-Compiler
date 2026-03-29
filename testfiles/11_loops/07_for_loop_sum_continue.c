int main() {
    int i;
    int sum;
    sum = 0;

    for (i = 0; i < 8; i = i + 1) {
        if ((i % 2) == 0) {
            continue;
        }
        sum = sum + i;
    }

    return sum; // 1+3+5+7 = 16
}
