// Simple nested scope - variable redefinition in inner scope allowed
int main() {
    int x = 10;
    {
        int x = 20;  // Different scope, allowed - shadowing
        x = x + 5;
    }
    return x;  // Should return 10
}
