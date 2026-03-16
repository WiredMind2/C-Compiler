//
// Created by dupic on 16/03/2026.
//

int main() {
    int i = 0;
    while (i < 10) {
        if (i == 5) {
            i = i + 1;
            continue;
        }
        if (i == 9) {
            break;
        }
        i = i +1;
    }
    return i;
}