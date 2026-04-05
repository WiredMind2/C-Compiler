//@unsupported
int main() {
    int i = 100;
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum = sum + i;
    }
    return i; // should return 100
}
