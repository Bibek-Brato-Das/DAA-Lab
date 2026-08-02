#include <stdio.h>
#include <stdlib.h>
#include <time.h>

long bubble_sort_optimized(int a[], int n) {          /* (i) early exit */
    long comparisons = 0;
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            comparisons++;
            if (a[j] > a[j + 1]) {
                int t = a[j]; a[j] = a[j + 1]; a[j + 1] = t;
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
    return comparisons;
}

long bubble_sort_full(int a[], int n) {                /* (ii) always full */
    long comparisons = 0;
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            comparisons++;
            if (a[j] > a[j + 1]) {
                int t = a[j]; a[j] = a[j + 1]; a[j + 1] = t;
            }
        }
    }
    return comparisons;
}

int *make_random(int n) { int *a = malloc(n * sizeof(int)); for (int i = 0; i < n; i++) a[i] = rand() % 1000000; return a; }
int *make_sorted(int n) { int *a = malloc(n * sizeof(int)); for (int i = 0; i < n; i++) a[i] = i; return a; }
int *dup_arr(int *s, int n) { int *a = malloc(n * sizeof(int)); for (int i = 0; i < n; i++) a[i] = s[i]; return a; }

int main() {
    srand((unsigned int) time(NULL));

    int sizes[] = {100, 200, 400, 800, 1600, 3200, 6400, 12800};
    int n_sizes = sizeof(sizes) / sizeof(sizes[0]);

    FILE *fp = fopen("bubble_sort_data.csv", "w");
    fprintf(fp, "n,optimized_random,full_random,optimized_sorted,full_sorted\n");

    printf("%-8s%-16s%-16s%-16s%-16s\n", "n", "Opt (random)", "Full (random)", "Opt (sorted)", "Full (sorted)");
    for (int i = 0; i < n_sizes; i++) {
        int n = sizes[i];

        int *rb = make_random(n);
        int *r1 = dup_arr(rb, n), *r2 = dup_arr(rb, n);
        long c_opt_rand  = bubble_sort_optimized(r1, n);
        long c_full_rand = bubble_sort_full(r2, n);

        int *sb = make_sorted(n);
        int *s1 = dup_arr(sb, n), *s2 = dup_arr(sb, n);
        long c_opt_sorted  = bubble_sort_optimized(s1, n);
        long c_full_sorted = bubble_sort_full(s2, n);

        printf("%-8d%-16ld%-16ld%-16ld%-16ld\n", n, c_opt_rand, c_full_rand, c_opt_sorted, c_full_sorted);
        fprintf(fp, "%d,%ld,%ld,%ld,%ld\n", n, c_opt_rand, c_full_rand, c_opt_sorted, c_full_sorted);

        free(rb); free(r1); free(r2); free(sb); free(s1); free(s2);
    }
    fclose(fp);
    printf("\nData saved to bubble_sort_data.csv\n");
    return 0;
}