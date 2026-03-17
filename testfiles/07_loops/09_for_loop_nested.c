int main() {
    int i;
    int j;
    int sum;
    sum = 0;

    for (i = 0; i < 3; i = i + 1) {
        for (j = 0; j < 5; j = j + 1) {
            if (j == 1) {
                continue;
            }
            if (j == 4) {
                break;
            }
            sum = sum + (10 * i + j);
        }
    }

    return sum; // (0+2+3) + (10+12+13) + (20+22+23) = 105
}
