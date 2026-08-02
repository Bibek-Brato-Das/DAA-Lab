#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* O(log n): binary search for the first index holding a 1 */
int find_partition_binary(int a[], int n) {
    int lo = 0, hi = n;   /* search half-open interval [lo, hi) */
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (a[mid] == 0) lo = mid + 1;
        else hi = mid;
    }
    return lo;             /* n means "all zeros, no 1 present" */
}

/* O(n): naive linear scan, for comparison */
int find_partition_linear(int a[], int n) {
    for (int i = 0; i < n; i++)
        if (a[i] == 1) return i;
    return n;
}

int *make_test(int n, int p) {
    int *a = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) a[i] = (i < p) ? 0 : 1;
    return a;
}

int main() {
    printf("=== Correctness check on several arrays ===\n");
    int cases[][2] = { {20, 7}, {50, 0}, {50, 50}, {1, 0}, {1, 1}, {15, 8} };
    for (int c = 0; c < 6; c++) {
        int n = cases[c][0], p = cases[c][1];
        int *a = make_test(n, p);
        int b = find_partition_binary(a, n);
        int l = find_partition_linear(a, n);
        printf("n=%-4d true_partition=%-4d  binary=%-4d linear=%-4d  %s\n",
               n, p, b, l, (b == p && l == p) ? "OK" : "MISMATCH");
        free(a);
    }

    printf("\n=== Step-count comparison on n = 1,000,000 ===\n");
    int n = 1000000, p = 730123;
    int *a = make_test(n, p);

    int lo = 0, hi = n, steps_b = 0;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        steps_b++;
        if (a[mid] == 0) lo = mid + 1; else hi = mid;
    }
    int steps_l = 0;
    for (int i = 0; i < n; i++) { steps_l++; if (a[i] == 1) break; }

    printf("True partition point : %d\n", p);
    printf("Binary search found  : %d  in %d steps  (~log2(n) = %.2f)\n", lo, steps_b, log2((double) n));
    printf("Linear scan found    : %d  in %d steps\n", steps_l, steps_l);
    printf("-> Binary search does O(log n) work; linear scan does O(n) work\n");
    printf("   in the worst case (partition point near the end of the array).\n");

    free(a);
    return 0;
}