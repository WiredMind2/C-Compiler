int main() {
    int i = 0;
    int j = 0;
    int sum = 0;
    while (i < 3) {
        j = 0;
        while (j < 3) {
            sum = sum + i + j;
            j = j + 1;
        }
        i = i + 1;
    }
    return sum;
}