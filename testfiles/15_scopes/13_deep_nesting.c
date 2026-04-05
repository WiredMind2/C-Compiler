// Deep nesting with variable shadowing at each level
int main() {
    int x = 1;
    {
        int x = 10;
        {
            int x = 100;
            {
                int x = 1000;
                {
                    int x = 10000;
                    x = x + 1;
                }
            }
        }
    }
    return x;  // Should return 1
}

