int main() {
    int i = 0;
    int acc = 0;

    while (i < 5) {
        switch (i) {
            case 0:
                acc = acc + 10;
                break;
            case 1:
                acc = acc + 20;
                break;
            case 2:
                acc = acc + 5;
                i = i + 1;
                continue;
            default:
                if (i == 4) {
                    acc = acc + 7;
                }
                break;
        }

        if (i == 3) {
            i = i + 1;
            continue;
        }

        i = i + 1;
    }

    return acc;
}

