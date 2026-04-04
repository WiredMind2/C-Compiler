// Break from inner loop, outer loop continues
// Count pairs (i,j) with i+j < 5, i in [0,4), j in [0,4)
int main() {
    int count = 0;
    int i;
    int j;
    for (i = 0; i < 4; i = i + 1) {
        for (j = 0; j < 4; j = j + 1) {
            if (i + j >= 5) {
                break;
            }
            count = count + 1;
        }
    }
    return count; // (0+0,0+1,0+2,0+3),(1+0,1+1,1+2,1+3),(2+0,2+1,2+2),(3+0,3+1) = 4+4+3+2 = 13
}
