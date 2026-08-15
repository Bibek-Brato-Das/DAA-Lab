/*
 * DAA Lab-03, Q3: Maximum and Minimum using Divide and Conquer
 * ---------------------------------------------------------------
 * Classic "tournament" D&C algorithm.
 *
 * findMaxMin(A, lo, hi):
 *   if only 1 element      -> max = min = that element        (0 comparisons)
 *   if exactly 2 elements  -> 1 comparison decides max & min
 *   else split into two halves, solve each recursively, then
 *        combine with exactly 2 more comparisons
 *        (1 to compare the two maxes, 1 to compare the two mins)
 *
 * Recurrence for comparisons: T(n) = 2*T(n/2) + 2, T(2) = 1, T(1) = 0
 * Solving gives T(n) = ceil(3n/2) - 2, i.e. O(3n/2) comparisons -
 * noticeably better than the naive 2(n-1) comparisons of scanning
 * for max and min separately.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static long long comparisons = 0;

typedef struct {
    int mx;
    int mn;
} Pair;

Pair findMaxMin(int arr[], int lo, int hi) {
    Pair result;

    if (lo == hi) {                       /* 1 element */
        result.mx = result.mn = arr[lo];
        return result;
    }

    if (hi - lo == 1) {                   /* 2 elements: 1 comparison */
        comparisons++;
        if (arr[lo] > arr[hi]) { result.mx = arr[lo]; result.mn = arr[hi]; }
        else                   { result.mx = arr[hi]; result.mn = arr[lo]; }
        return result;
    }

    int mid = (lo + hi) / 2;
    Pair left  = findMaxMin(arr, lo, mid);
    Pair right = findMaxMin(arr, mid + 1, hi);

    Pair combined;
    comparisons++;                        /* combine maxes */
    combined.mx = (left.mx > right.mx) ? left.mx : right.mx;
    comparisons++;                        /* combine mins */
    combined.mn = (left.mn < right.mn) ? left.mn : right.mn;

    return combined;
}

void manualRun(void) {
    int n;
    printf("Enter number of elements: ");
    scanf("%d", &n);
    int *arr = malloc(sizeof(int) * n);
    printf("Enter %d elements: ", n);
    for (int i = 0; i < n; i++) scanf("%d", &arr[i]);

    comparisons = 0;
    Pair res = findMaxMin(arr, 0, n - 1);

    printf("\nMaximum = %d\n", res.mx);
    printf("Minimum = %d\n", res.mn);
    printf("Comparisons used = %lld   (bound: ceil(3n/2)-2 = %d)\n",
           comparisons, (3 * n + 1) / 2 - 2);
    if ((n & (n - 1)) != 0)
        printf("(Note: n is not a power of 2, so an odd element may appear at\n"
               " some recursive split; the count can be a few comparisons\n"
               " above the idealized bound while staying Theta(n) overall.)\n");

    free(arr);
}

void autoValidate(void) {
    /* Powers of two give the textbook-exact 3n/2 - 2 comparison count,
     * since every recursive split is then perfectly balanced. */
    int sizes[] = {16, 64, 256, 1024, 4096, 16384, 65536, 262144, 1048576};
    int ns = 9;
    srand((unsigned)time(NULL));

    printf("\n%-12s%-16s%-16s%-10s\n", "n", "Comparisons", "Bound(3n/2-2)", "OK?");
    for (int t = 0; t < ns; t++) {
        int n = sizes[t];
        int *arr = malloc(sizeof(int) * n);
        for (int i = 0; i < n; i++) arr[i] = rand();

        comparisons = 0;
        Pair res = findMaxMin(arr, 0, n - 1);

        /* sanity check against a naive linear scan */
        int naiveMax = arr[0], naiveMin = arr[0];
        for (int i = 1; i < n; i++) {
            if (arr[i] > naiveMax) naiveMax = arr[i];
            if (arr[i] < naiveMin) naiveMin = arr[i];
        }
        int correct = (naiveMax == res.mx && naiveMin == res.mn);
        long long bound = (long long)(3 * n + 1) / 2 - 2;

        printf("%-12d%-16lld%-16lld%-10s\n", n, comparisons, bound,
               (correct && comparisons <= bound) ? "PASS" : "FAIL");

        free(arr);
    }
}

int main(void) {
    printf("=========== Max & Min using Divide and Conquer ===========\n\n");
    int choice;
    printf("1. Automatic validation (random arrays, checks correctness + 3n/2 bound)\n");
    printf("2. Manual input\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    if (choice == 2) manualRun();
    else autoValidate();

    return 0;
}
