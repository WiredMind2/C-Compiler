int main() {
    int a[3], b[2];

    b[0] = 1;
    a[2] = 2;

    int  x = 0;   // SymbolTable[x] = -4    fp[-4] = 0
    int* y = &x;  // SymbolTable[y] = -8    fp[-8] = -4 (adresse de x)

    // lparam évalue à StackParam(x) avec SymbolTable[x] = -4    fp[-4] = 0
    //x = 5;

    // lparam évalue à StackParam(!tmp1) avec SymbolTable[!tmp1] = -12   fp[-12] = fp[fp[-8]] = 0 (adresse stockée à l’adresse stockée dans y)
    *(y + 8) = 4;

    return b[0] + a[2];
}
