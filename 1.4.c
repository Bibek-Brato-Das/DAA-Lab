#include <stdio.h>
#include <stdlib.h>

long move_count = 0;

void hanoi(int n, char from, char to, char via, int verbose) {
    if (n == 0) return;
    hanoi(n - 1, from, via, to, verbose);
    move_count++;
    if (verbose) printf("Move disc %d: %c -> %c\n", n, from, to);
    hanoi(n - 1, via, to, from, verbose);
}

int main() {
    printf("=== Demo: solving Tower of Hanoi for n = 4 discs ===\n");
    move_count = 0;
    hanoi(4, 'A', 'C', 'B', 1);
    printf("Total moves for n = 4: %ld  (formula 2^n - 1 = %ld)\n\n", move_count, (1L << 4) - 1);

    FILE *fp = fopen("hanoi_data.csv", "w");
    fprintf(fp, "n,moves\n");

    printf("%-6s%-16s%-16s\n", "n", "Moves (actual)", "2^n - 1 (formula)");
    for (int n = 1; n <= 25; n++) {
        move_count = 0;
        hanoi(n, 'A', 'C', 'B', 0);
        long formula = (1L << n) - 1;
        printf("%-6d%-16ld%-16ld\n", n, move_count, formula);
        fprintf(fp, "%d,%ld\n", n, move_count);
    }
    fclose(fp);
    printf("\nData saved to hanoi_data.csv\n");
    return 0;
}