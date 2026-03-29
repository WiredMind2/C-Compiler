// If-else with variable shadowing
int main() {
    int x = 1;
    if (x > 0) {
        int x = 10;
        x = x + 5;
    } else {
        int x = 20;
        x = x + 3;
    }
    return x;  // Should return 1 (original outer x)
}
