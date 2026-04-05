//@unsupported: requires compile-time case label evaluation (duplicate case values)
int main() {
    int x = 3;

    switch (x) {
        case 1 + 2:
            return 1;
        case 3:
            return 2;
        default:
            return 0;
    }
}
