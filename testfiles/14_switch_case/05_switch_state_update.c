int main() {
    int x = 4;
    int acc = 1;

    switch (x) {
        case 4:
            acc = acc + 8;
            break;
        default:
            acc = 0;
            break;
    }

    return acc;
}

