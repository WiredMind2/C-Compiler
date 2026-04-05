// Multiple scopes at same level - variable shadowing in each
int main() {
    int x = 1;
    {
        int x = 10;
        x = x + 1;
    }
    {
        int x = 20;
        x = x + 2;
    }
    {
        int x = 30;
        x = x + 3;
    }
    return x;  // Should return 1
}

