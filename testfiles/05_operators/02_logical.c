/* Test logical operators */
int main() {
    int a = 1;
    int b = 0;
    int c = 5;
    
    int r1 = a && c;
    int r2 = a && b;
    int r3 = b && b;
    int r4 = a || b;
    int r5 = b || b;
    int r6 = a || c;
    int r7 = (a && c) || (b && b);
    
    return r1 + r2 + r3 + r4 + r5 + r6 + r7;
}
