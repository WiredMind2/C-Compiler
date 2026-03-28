//
// Created by dupic on 23/03/2026.
//

int main() {
    int i = 0;
    switch (i) {
        case 0:
            i = i + 1;
            break;
        case 0: // Redefinition of case value 0
            i = i + 2;
            break;
        default:
            i = i + 3;
    }
    return i;
}