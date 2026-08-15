/*
 * DAA Lab-03, Q6: Loop Invariants in Sorting (Selection Sort)
 * ---------------------------------------------------------------
 *
 * PSEUDOCODE
 * ----------
 * SELECTION-SORT(A, n)
 *   for i = 1 to n-1
 *       min_idx = i
 *       for j = i+1 to n
 *           if A[j] < A[min_idx]
 *               min_idx = j
 *       exchange A[i] with A[min_idx]
 *
 * LOOP INVARIANT (for the outer for loop, on index i)
 * -----------------------------------------------------
 * At the start of each iteration of the outer loop, the subarray
 * A[1 .. i-1] consists of the (i-1) smallest elements of the
 * original array, stored in sorted (nondecreasing) order, and every
 * element in A[1..i-1] is <= every element in A[i..n].
 *
 * Initialization: Before the first iteration, i = 1, so A[1..0] is
 *   the empty subarray. It trivially "contains the 0 smallest
 *   elements in sorted order" -- the invariant holds vacuously.
 *
 * Maintenance: Assume the invariant holds at the start of iteration
 *   i. The inner loop scans A[i..n] and finds min_idx, the index of
 *   the smallest element in that range; it is then swapped into
 *   position i. By the invariant, A[1..i-1] already holds the i-1
 *   smallest elements, so the overall i-th smallest element must lie
 *   in A[i..n], and the inner loop correctly locates it. After the
 *   swap, A[1..i] holds the i smallest elements in sorted order
 *   (since the new A[i] is >= every element of A[1..i-1], and A[i-1]
 *   was already the largest of those). This re-establishes the
 *   invariant for i+1.
 *
 * Termination: The outer loop ends when i = n (it runs for
 *   i = 1..n-1, so it stops right after i = n-1 completes). At that
 *   point the invariant says A[1..n-1] holds the n-1 smallest
 *   elements in sorted order. The one remaining element, A[n], is
 *   therefore necessarily the single largest element left, so
 *   A[1..n] is fully sorted -- proving correctness.
 *
 * Why only the first (n-1) elements, not all n?
 *   Once A[1..n-1] is correctly sorted (by the invariant at
 *   termination), A[n] is automatically the maximum of the whole
 *   array by elimination -- there is nothing left to compare it
 *   against. Running the outer loop one more time for i = n would
 *   scan the single-element range A[n..n], find min_idx = n
 *   (trivially, with zero comparisons) and swap A[n] with itself:
 *   a wasted, do-nothing iteration.
 *
 * RUNNING TIME
 * ------------
 * The number of key comparisons is always
 *     (n-1) + (n-2) + ... + 1 = n(n-1)/2 = Theta(n^2)
 * REGARDLESS of the input order -- the inner loop always scans the
 * full remaining range to find the minimum, whether the array is
 * already sorted, reverse sorted, or random. So:
 *     Worst case:  Theta(n^2)
 *     Best case:   Theta(n^2)   -- NOT better than worst case!
 * (Only the number of *swaps* varies with input: 0 in the best
 * case (already sorted) up to n-1 in the worst case -- but swaps
 * are a lower-order O(n) term that doesn't change the dominant
 * Theta(n^2) comparison cost.) This is unlike, e.g., insertion
 * sort, whose best case IS asymptotically better (Theta(n)).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static long long comparisons = 0;
static long long swaps = 0;

void selectionSort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++) {
            comparisons++;
            if (arr[j] < arr[minIdx]) minIdx = j;
        }
        if (minIdx != i) {
            int t = arr[i]; arr[i] = arr[minIdx]; arr[minIdx] = t;
            swaps++;
        }
    }
}

void printArr(int arr[], int n) {
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n");
}

int cmpAsc(const void* a, const void* b)  { return (*(int*)a - *(int*)b); }
int cmpDesc(const void* a, const void* b) { return (*(int*)b - *(int*)a); }

void manualRun(void) {
    int n;
    printf("Enter number of elements: ");
    scanf("%d", &n);
    int* arr = malloc(sizeof(int) * n);
    printf("Enter %d elements: ", n);
    for (int i = 0; i < n; i++) scanf("%d", &arr[i]);

    comparisons = 0; swaps = 0;
    selectionSort(arr, n);

    printf("\nSorted array: ");
    printArr(arr, n);
    printf("Comparisons = %lld (expected n(n-1)/2 = %d)\n", comparisons, n * (n - 1) / 2);
    printf("Swaps       = %lld\n", swaps);

    free(arr);
}

void autoValidate(void) {
    int n = 2000;
    int* base = malloc(sizeof(int) * n);
    srand((unsigned)time(NULL));
    for (int i = 0; i < n; i++) base[i] = rand() % 100000;

    int* alreadySorted = malloc(sizeof(int) * n);
    int* reverseSorted  = malloc(sizeof(int) * n);
    int* randomOrder    = malloc(sizeof(int) * n);
    memcpy(alreadySorted, base, sizeof(int) * n);
    memcpy(reverseSorted, base, sizeof(int) * n);
    memcpy(randomOrder,   base, sizeof(int) * n);

    qsort(alreadySorted, n, sizeof(int), cmpAsc);
    qsort(reverseSorted, n, sizeof(int), cmpDesc);
    /* randomOrder stays as originally generated (random) */

    long long expected = (long long)n * (n - 1) / 2;

    printf("\nn = %d, expected comparisons (n(n-1)/2) = %lld\n\n", n, expected);
    printf("%-20s%-16s%-12s\n", "Input case", "Comparisons", "Swaps");

    comparisons = 0; swaps = 0;
    selectionSort(alreadySorted, n);
    printf("%-20s%-16lld%-12lld\n", "Already sorted (best)", comparisons, swaps);

    comparisons = 0; swaps = 0;
    selectionSort(reverseSorted, n);
    printf("%-20s%-16lld%-12lld\n", "Reverse sorted", comparisons, swaps);

    comparisons = 0; swaps = 0;
    selectionSort(randomOrder, n);
    printf("%-20s%-16lld%-12lld\n", "Random order", comparisons, swaps);

    printf("\nNotice: comparisons are IDENTICAL (%lld) in all three cases,\n", expected);
    printf("confirming Theta(n^2) holds for best, average, AND worst case.\n");
    printf("Only the SWAP count differs with input order (0 for already-\n");
    printf("sorted up to n-1 for reverse-sorted) -- but that's a lower-order\n");
    printf("O(n) term that doesn't change the dominant Theta(n^2) behaviour.\n");

    free(base); free(alreadySorted); free(reverseSorted); free(randomOrder);
}

int main(void) {
    printf("=========== Selection Sort: Loop Invariant & Complexity ===========\n\n");
    int choice;
    printf("1. Automatic validation (best/avg/worst-case comparisons, n=2000)\n");
    printf("2. Manual input\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    if (choice == 2) manualRun();
    else autoValidate();

    return 0;
}
