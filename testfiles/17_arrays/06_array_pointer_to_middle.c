int main() {
    int arr[5];
    int *p;
    
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;
    arr[3] = 40;
    arr[4] = 50;
    
    p = &arr[2];  // point to middle element
    
    return *p;    // should return 30
}