/* Test relational operators */
int main() {
    int a = 5;
    int b = 10;
    int c = 5;
    
    int r1 = a < b;
    int r2 = a > b;
    int r3 = a <= c;
    int r4 = a >= c;
    
    return r1 + r2 + r3 + r4;
}
