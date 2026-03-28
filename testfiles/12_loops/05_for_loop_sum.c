int main() {
    int i;
    int sum;
    sum = 0;

    for (i = 0; i < 5; i = i + 1) {
        sum = sum + i;
    }

    return sum; // 0+1+2+3+4 = 10
}
