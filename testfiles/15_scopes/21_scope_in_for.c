// Variable declared inside for body shadows outer variable
int main() {
    int x = 100;
    int i;
    for (i = 0; i < 3; i++) {
        int x = i * 2;
        x += 1;
    }
    return x; // outer x unchanged: 100
}
