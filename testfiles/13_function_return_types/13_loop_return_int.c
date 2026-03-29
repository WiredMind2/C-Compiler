int loop_sum(int n) {
    int sum = 0;
    int i = 0;
    while (i < n) {
        sum = sum + i;
        i = i + 1;
    }
    return sum;
}
int main() {
    return loop_sum(5);
}

