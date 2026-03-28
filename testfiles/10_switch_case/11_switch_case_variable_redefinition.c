//
// Created by dupic on 23/03/2026.
//

int main() {
    int i = 0;
    switch (i) {
        case 0:
            i = i + 1;
            int a;
            break;
        case 1:
            i = i + 2;
            int a; // Redefinition of variable 'a' in the same scope
            break;
        default:
            i = i + 3;
    }
    return i;
}