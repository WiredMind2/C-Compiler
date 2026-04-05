// Complex nested scopes with multiple variable shadowing
int main() {
    int a = 1;
    int b = 2;
    int c = 3;
    {
        int a = 10;
        int b = 20;
        {
            int a = 100;
            int c = 300;
            a = a + b;
            c = c + 1;
        }
        b = b + a;
    }
    return a + b + c;  // Should return 6 (1 + 2 + 3)
}

