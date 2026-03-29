// Test : case avec scope et shadowing
int main() {
    int x = 1;
    int i = 0;
    switch (i) {
        case 0: {
            int x = 10;  // Shadowing x
            x = x + 5;
            break;
        }
        case 1: {
            int x = 20;  // Different shadowing
            x = x * 2;
            break;
        }
    }
    return x;  // Should return 1 (original x)
}

