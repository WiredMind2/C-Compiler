int main() {
    int i;
    int sum;
    sum = 0;

    for (i = 0; i < 10; i = i + 1) {
        if (i == 4) {
            break;
        }
        sum = sum + i;
    }

    return sum; // 0+1+2+3 = 6
}
