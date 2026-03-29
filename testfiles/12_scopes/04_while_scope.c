// While loop scope
int main() {
    int i = 0;
    int sum = 0;
    while (i < 5) {
        int x = i * 2;
        sum = sum + x;
        i = i + 1;
    }
    return sum;  // Should return 20 (0 + 2 + 4 + 6 + 8)
}

