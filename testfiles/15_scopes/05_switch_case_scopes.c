// Switch with case scopes (requires braces)
int main() {
    int i = 1;
    switch (i) {
        case 0: {
            int x = 10;
            i = i + x;
            break;
        }
        case 1: {
            int x = 20;
            i = i + x;
            break;
        }
        default: {
            int x = 30;
            i = i + x;
        }
    }
    return i;  // Should return 21
}

