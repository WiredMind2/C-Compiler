/* Full bitwise operations stress test */
int main() {
    int a = 5;
    int b = 3;
    int c = 0xF0; /* 240 */
    int d = 0x0F; /* 15  */

    int r1 = a << 3;     /* 5 << 3 = 40 */
    int r2 = b >> 1;     /* 3 >> 1 = 1  */
    int r3 = c & d;      /* 240 & 15 = 0 */
    int r4 = c | d;      /* 240 | 15 = 255 */
    int r5 = c ^ d;      /* 240 ^ 15 = 255 */
    int r6 = ~a;         /* ~5 = -6 */
    int r7 = (a << 2) + (d >> 1); /* 20 + 7 = 27 */
    int r8 = (c >> 4) & 0xF;      /* (240>>4)=15 & 15 =15 */
    int r9 = (-1) >> 1;  /* arithmetic shift: -1 */

    /* Sum: 40 + 1 + 0 + 255 + 255 -6 +27 +15 -1 = 586 */
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9;
}
