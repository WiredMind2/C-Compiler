// Shadowing with different variable names coexisting
int main() {
    int a = 1;
    int b = 2;
    {
        int a = 10;
        int c = 3;
        {
            int b = 20;
            int c = 30;
            a = a + b + c;
        }
    }
    return a + b;  // Should return 3 (1 + 2)
}

