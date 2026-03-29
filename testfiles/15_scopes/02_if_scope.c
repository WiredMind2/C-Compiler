// If statement creates new scope
int main() {
    int x = 5;
    if (x > 0) {
        int y = 10;
        x = x + y;
    }
    return x;  // Should return 15
}
