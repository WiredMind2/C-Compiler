int main() {
    int i = 10;
    int sum = 0;
    while (i < 0) {
        sum = sum + i;
        i = i + 1;
    }
    return sum;
}