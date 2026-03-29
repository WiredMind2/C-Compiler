// Nested scope - variable shadowing with multiple levels
int main() {
    int x = 1;
    int y = 2;
    {
        int x = 10;
        y = y + x;
        {
            int x = 100;
            y = y + x;
        }
    }
    return y;  // Should return 112 (2 + 10 + 100)
}
