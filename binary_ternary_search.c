/*
 * DAA Lab-03, Q1: Binary vs Ternary Search
 * -----------------------------------------
 * Searches an element x in a sorted array of size n using both
 * Binary Search (2-way split) and Ternary Search (3-way split),
 * counting comparisons made by each, then argues (empirically and
 * theoretically) that Binary Search is better.
 *
 * Why binary beats ternary (theory):
 *   Binary search:  ~log2(n) levels, 2 comparisons per level (approx)
 *                    -> total comparisons ~ 2*log2(n)
 *   Ternary search: ~log3(n) levels, up to 4 comparisons per level
 *                    -> total comparisons ~ 4*log3(n) = 4*log2(n)/log2(3)
 *                                          = (4/1.585)*log2(n) ~ 2.52*log2(n)
 * Since 2*log2(n) < 2.52*log2(n) for all n > 1, binary search always
 * does fewer comparisons in the worst case, despite dividing into
 * fewer pieces per step. Fewer, "heavier" comparisons win over more,
 * "lighter" splits.
 */

#include <stdio.h>
#include <stdlib.h>

static long long binary_comparisons = 0;
static long long ternary_comparisons = 0;

/* Standard iterative binary search. Returns index of x, or -1. */
int binarySearch(int arr[], int lo, int hi, int x) {
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        binary_comparisons++;               /* comparison: arr[mid] == x */
        if (arr[mid] == x) return mid;
        binary_comparisons++;               /* comparison: arr[mid] < x  */
        if (arr[mid] < x) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

/* Standard iterative ternary search. Returns index of x, or -1. */
int ternarySearch(int arr[], int lo, int hi, int x) {
    while (lo <= hi) {
        int mid1 = lo + (hi - lo) / 3;
        int mid2 = hi - (hi - lo) / 3;

        ternary_comparisons++;              /* arr[mid1] == x */
        if (arr[mid1] == x) return mid1;

        ternary_comparisons++;              /* arr[mid2] == x */
        if (arr[mid2] == x) return mid2;

        ternary_comparisons++;              /* x < arr[mid1] */
        if (x < arr[mid1]) {
            hi = mid1 - 1;
        } else {
            ternary_comparisons++;          /* x > arr[mid2] */
            if (x > arr[mid2]) {
                lo = mid2 + 1;
            } else {
                lo = mid1 + 1;
                hi = mid2 - 1;
            }
        }
    }
    return -1;
}

/* Runs both searches for a chosen worst-case-ish element (the last
 * element, which forces the search to walk all the way to a
 * boundary) and prints the comparison counts. */
void runTrial(int n) {
    int *arr = malloc(sizeof(int) * n);
    for (int i = 0; i < n; i++) arr[i] = 2 * i;   /* sorted array: 0,2,4,... */
    int x = arr[n - 1];                            /* worst-case target      */

    binary_comparisons = 0;
    ternary_comparisons = 0;

    int bi = binarySearch(arr, 0, n - 1, x);
    int ti = ternarySearch(arr, 0, n - 1, x);

    printf("%-12d%-10d%-10d%-16lld%-16lld%-10s\n",
           n, bi, ti, binary_comparisons, ternary_comparisons,
           (binary_comparisons < ternary_comparisons) ? "Binary" :
           (binary_comparisons > ternary_comparisons) ? "Ternary" : "Tie");

    free(arr);
}

int main(void) {
    printf("=========== Binary Search vs Ternary Search ===========\n\n");

    int choice;
    printf("1. Run automatic validation across several array sizes\n");
    printf("2. Manual single search\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    if (choice == 2) {
        int n;
        printf("Enter number of elements (sorted ascending input): ");
        scanf("%d", &n);
        int *arr = malloc(sizeof(int) * n);
        printf("Enter %d sorted elements: ", n);
        for (int i = 0; i < n; i++) scanf("%d", &arr[i]);
        int x;
        printf("Enter element to search: ");
        scanf("%d", &x);

        binary_comparisons = 0;
        ternary_comparisons = 0;
        int bi = binarySearch(arr, 0, n - 1, x);
        int ti = ternarySearch(arr, 0, n - 1, x);

        printf("\nBinary search  : index = %d, comparisons = %lld\n", bi, binary_comparisons);
        printf("Ternary search : index = %d, comparisons = %lld\n", ti, ternary_comparisons);
        free(arr);
    } else {
        int sizes[] = {100, 1000, 10000, 100000, 1000000, 10000000};
        int ns = 6;

        printf("\n%-12s%-10s%-10s%-16s%-16s%-10s\n",
               "n", "BinIdx", "TerIdx", "BinaryComps", "TernaryComps", "Winner");
        for (int i = 0; i < ns; i++) runTrial(sizes[i]);
    }

    printf("\nConclusion: Binary search consistently uses fewer comparisons than\n");
    printf("ternary search for the same worst-case target, because although it\n");
    printf("has more levels (log2 n vs log3 n), each ternary level costs up to\n");
    printf("twice as many comparisons, and that overhead outweighs the saving\n");
    printf("from having fewer levels. Hence Binary Search is asymptotically and\n");
    printf("empirically better for this comparison-based search model.\n");

    return 0;
}
