const int two() {
    return 2;
}

int main() {
    int x = 2;

    switch (x) {
        case two():
            return 1;
        default:
            return 0;
    }
}
