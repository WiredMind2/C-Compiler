// Mixed scopes - if, while, blocks
int main() {
    int sum = 0;
    int i = 0;
    while (i < 3) {
        {
            int temp = i * 2;
            sum = sum + temp;
        }
        if (i > 0) {
            int temp = i * 3;
            sum = sum + temp;
        }
        i = i + 1;
    }
    return sum;  // Should return 20 (0 + 3 + 4 + 9 + 4)
}

