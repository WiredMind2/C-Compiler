int main() {
    int x = 2;
    int result = 0;
    switch(x) {
        case 1: result = 1; break;
        case 2: result = 2; // Fall through
        case 3: result += 10; break;
        default: result = 50;
    }
    // Result should be 12 (2+10)
    return result;
}
