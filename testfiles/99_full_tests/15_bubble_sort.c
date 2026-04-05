// Bubble sort on a small array, returns checksum
int main() {
    int arr[8];
    arr[0] = 5;
    arr[1] = 3;
    arr[2] = 8;
    arr[3] = 1;
    arr[4] = 9;
    arr[5] = 2;
    arr[6] = 7;
    arr[7] = 4;

    int n = 8;
    int i;
    int j;
    int tmp;

    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - 1 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
            }
        }
    }

    // arr should be [1,2,3,4,5,7,8,9], sum = 39
    int sum = 0;
    for (i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}
