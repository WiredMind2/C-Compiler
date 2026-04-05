// Nested if statements with shadowing
int main() {
    int x = 5;
    if (x > 0) {
        int x = 10;
        if (x > 5) {
            int x = 20;
            x = x + 1;
        }
    }
    return x;  // Should return 5
}

