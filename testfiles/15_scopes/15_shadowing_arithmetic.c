// Shadowing with arithmetic operations
int main() {
    int result = 100;
    {
        int result = 50;
        result = result + 10;
    }
    {
        int result = 25;
        result = result * 2;
    }
    return result;  // Should return 100
}

