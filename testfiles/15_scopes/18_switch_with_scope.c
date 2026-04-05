// Test simple : case avec scope
int main() {
    int i = 1;
    switch (i) {
        case 0: {
            int x = 10;
            i = x;
            break;
        }
        case 1: {
            int x = 20;
            i = x;
            break;
        }
    }
    return i;  // Should return 20
}

