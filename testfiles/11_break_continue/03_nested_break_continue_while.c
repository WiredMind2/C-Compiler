//
// Created by dupic on 16/03/2026.
//

// this file include breaks and continues nested withing while and ifs

int main() {
    int i = 0;
    int u = 0;
    while (i < 10) {
        while (u < 10) {
            if (u == 5) {
                if (i == 3) {
                    break;
                }
                u = u + 1;
                continue;
            }
            if (u == 9) {
                break;
            }
            u = u +1;
        }
        if (i == 7) {
            break;
        }
        i = i +1;
    }
    return i*u;
}