int main() {
    int arr[5];
    int i;
    int sum = 0;
    int *p;

    /* Initialize array */
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    arr[3] = 4;
    arr[4] = 5;

    /* Pointer to first element */
    p = &arr[0];

    /* Sum using pointer arithmetic */
    for (i = 0; i < 5; i++) {
        sum += *p;
        p++;
    }

    return sum;
}