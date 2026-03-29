// Variable initialization uses outer scope variable
int main() {
    int x = 10;
    {
        int y = x + 5;
        int x = 100;
        y = y + x;
    }
    return x;  // Should return 10
}

