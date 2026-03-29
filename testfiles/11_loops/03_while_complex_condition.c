int main() {
    int i = 0;
    int j = 5;
    int sum = 0;
    while (i < 10 && j > 0) {
        sum = sum + i + j;
        i = i + 1;
        j = j - 1;
    }
    return sum;
}