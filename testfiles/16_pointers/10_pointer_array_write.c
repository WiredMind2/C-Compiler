// Write to array through a pointer, read back via array indexing
int main() {
    int arr[5];
    int *p = arr;
    int i;
    for (i = 0; i < 5; i++) {
        *(p + i) = i * i;
    }
    // arr = [0, 1, 4, 9, 16], sum = 30
    return arr[0] + arr[1] + arr[2] + arr[3] + arr[4];
}
