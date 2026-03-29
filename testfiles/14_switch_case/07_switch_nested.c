int main() {
    int a = 2;
    int b = 1;
    int result = 0;

    switch (a) {
        case 1:
            result = 10;
            break;
        case 2:
            switch (b) {
                case 0:
                    result = 20;
                    break;
                case 1:
                    result = 21;
                    break;
                default:
                    result = 22;
                    break;
            }
            break;
        default:
            result = 30;
            break;
    }

    return result;
}

