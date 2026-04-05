// Kadane's algorithm: maximum subarray sum
// arr = [-2, 1, -3, 4, -1, 2, 1, -5, 4] => max = 6
int main() {
    int arr[9];
    arr[0] = -2;
    arr[1] = 1;
    arr[2] = -3;
    arr[3] = 4;
    arr[4] = -1;
    arr[5] = 2;
    arr[6] = 1;
    arr[7] = -5;
    arr[8] = 4;

    int max_so_far = arr[0];
    int max_ending_here = arr[0];
    int i;

    for (i = 1; i < 9; i++) {
        if (max_ending_here + arr[i] > arr[i]) {
            max_ending_here = max_ending_here + arr[i];
        } else {
            max_ending_here = arr[i];
        }
        if (max_ending_here > max_so_far) {
            max_so_far = max_ending_here;
        }
    }

    return max_so_far;
}
