#include <stdio.h>

void print_int(int x) {
    if (x < 0) {
        putchar('-');
        x = -x;
    }
    if (x / 10 != 0) print_int(x / 10);
    putchar(x % 10 + '0');
}

void print_space() {
    putchar(32);
}

void print_newline() {
    putchar(10);
}

void print_header() {
    putchar('R'); putchar('e'); putchar('s'); putchar('u'); putchar('l'); putchar('t'); putchar(':');
    putchar(10);
}

int main() {
    int n = 4;
    int dist[16]; // 4x4 matrix flattened
    int i, j, k;
    int INF = 999;

    print_header();

    // Initialize distance matrix
    // 0: 0, 1: 3, 2: INF, 3: 7
    // 1: 8, 0, 2, INF
    // 2: 5, INF, 0, 1
    // 3: 2, INF, INF, 0

    dist[0] = 0;   dist[1] = 3;   dist[2] = INF; dist[3] = 7;
    dist[4] = 8;   dist[5] = 0;   dist[6] = 2;   dist[7] = INF;
    dist[8] = 5;   dist[9] = INF; dist[10] = 0;  dist[11] = 1;
    dist[12] = 2;  dist[13] = INF; dist[14] = INF; dist[15] = 0;

    for (k = 0; k < n; k++) {
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                if (dist[i * n + k] + dist[k * n + j] < dist[i * n + j]) {
                    dist[i * n + j] = dist[i * n + k] + dist[k * n + j];
                }
            }
        }
    }

    // Print result
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            print_int(dist[i * n + j]);
            print_space();
        }
        print_newline();
    }

    return 0;
}
