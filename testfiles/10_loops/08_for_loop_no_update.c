int main() {
    int i;
    int sum;
    i = 0;
    sum = 0;

    for (; i < 5;) {
        sum = sum + i;
        i = i + 1;
    }

    return sum; // 10
}
