//@unsupported
void send(int *to, int *from, int count) {
    int n = (count + 7) / 8;
    switch (count % 8) {
    case 0:
        do {
            *to = *from++;
    case 7:
            *to = *from++;
    case 6:
            *to = *from++;
    case 5:
            *to = *from++;
    case 4:
            *to = *from++;
    case 3:
            *to = *from++;
    case 2:
            *to = *from++;
    case 1:
            *to = *from++;
        } while (--n > 0);
    }
}

int main() {
    int source[10];
    int dest[10];
    int i = 0;

    // Initialize source array
    while (i < 10) {
        source[i] = i + 100;
        i = i + 1;
    }

    // Copy using Duff's Device
    send(dest, source, 10);

    // Verify the copy worked
    return dest[9];  // Should return 109 (i + 100 where i = 9)
}

