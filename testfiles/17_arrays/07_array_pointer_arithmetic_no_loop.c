int main() {
    int arr[5];
    int *p;
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    arr[3] = 4;
    arr[4] = 5;
    p = &arr[0];
    return *p + *(p+1) + *(p+2) + *(p+3) + *(p+4);
}