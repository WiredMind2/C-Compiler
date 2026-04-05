// Sum only odd numbers from 1 to 9 using continue
int main() {
    int sum;
    int i;
    sum = 0;
    for (i = 1; i < 11; i = i + 1) {
        if (i % 2 == 0) {
            continue;
        }
        sum = sum + i;
    }
    return sum; // 1+3+5+7+9 = 25
}
