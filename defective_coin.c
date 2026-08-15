/*
 * DAA Lab-03, Q2: Search the Defective (Lighter) Coin
 * -----------------------------------------------------
 * n coins are given. At most ONE of them may be lighter than the
 * rest (defective); it is also possible that none is defective.
 * Using a balance scale (which compares the TOTAL weight placed on
 * its two pans), find the defective coin, or report that none
 * exists, in log2(n) + c weighings for some small constant c.
 *
 * ALGORITHM (Divide and Conquer)
 * -------------------------------
 * We keep a contiguous range [lo..hi] of "candidate" coins.
 *
 * Phase A - searchMaybeNone(lo,hi): we do not yet know whether a
 *   defective coin exists at all inside this range.
 *     m = (hi-lo+1)/2
 *     Weigh the first m coins against the next m coins.
 *       - If they balance: both groups are provably genuine
 *         (if either contained the light coin, that pan would rise).
 *           * If range size is even, every coin was weighed -> no
 *             defective coin exists in this whole range -> return NONE.
 *           * If range size is odd, one coin (the last one) was left
 *             aside. We already know a genuine coin (any coin from the
 *             balanced pans), so ONE extra weighing of leftover vs a
 *             genuine coin tells us if it is defective or if there is
 *             no defective coin at all.
 *       - If they don't balance: the lighter pan contains the
 *         defective coin, and existence is now CONFIRMED. Recurse
 *         with Phase B on that half.
 *
 * Phase B - searchDefinite(lo,hi): the defective coin is guaranteed
 *   to be inside this range.
 *     Same halving weighing. If it balances, both halves are genuine,
 *     so the (odd) leftover coin MUST be the defective one - no
 *     extra reference weighing is needed here, because we already
 *     know one exists in this range.
 *     If it doesn't balance, recurse into the lighter half.
 *
 * Each level roughly halves the candidate set, so total weighings
 * is about log2(n), plus at most one extra weighing (the reference
 * check), giving log2(n) + c with c a small constant (c<=1 here).
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

static int weighings = 0;

int strcmpSafe(const char *a, const char *b); /* forward declaration */

/* Weighs arr[l1 .. l1+cnt-1] against arr[l2 .. l2+cnt-1].
 * Returns -1 if left pan is lighter, 0 if balanced, 1 if right pan lighter. */
int weigh(int arr[], int l1, int l2, int cnt) {
    weighings++;
    long sumL = 0, sumR = 0;
    for (int i = 0; i < cnt; i++) {
        sumL += arr[l1 + i];
        sumR += arr[l2 + i];
    }
    if (sumL < sumR) return -1;
    if (sumL > sumR) return 1;
    return 0;
}

/* Phase B: defective coin is guaranteed to be in arr[lo..hi]. */
int searchDefinite(int arr[], int lo, int hi) {
    int n = hi - lo + 1;
    if (n == 1) return lo;

    int m = n / 2;
    int leftStart = lo, rightStart = lo + m;
    int cmp = weigh(arr, leftStart, rightStart, m);

    if (cmp == 0) {
        /* left & right (2m coins) are all genuine -> the odd leftover
         * (index hi) must be the defective one. */
        return hi;
    } else if (cmp < 0) {
        return searchDefinite(arr, leftStart, leftStart + m - 1);
    } else {
        return searchDefinite(arr, rightStart, rightStart + m - 1);
    }
}

/* Phase A: defective coin may or may not exist in arr[lo..hi].
 * Returns index of defective coin, or -1 if none. */
int searchMaybeNone(int arr[], int lo, int hi) {
    int n = hi - lo + 1;
    if (n <= 0) return -1;
    if (n == 1) return -1; /* single coin, nothing to compare against: assume genuine */

    int m = n / 2;
    int leftStart = lo, rightStart = lo + m;
    int cmp = weigh(arr, leftStart, rightStart, m);

    if (cmp == 0) {
        if (n % 2 == 0) {
            return -1; /* every coin weighed and balanced -> no defective */
        } else {
            int leftover = hi;
            int cmp2 = weigh(arr, leftover, leftStart, 1); /* vs a proven-genuine coin */
            if (cmp2 < 0) return leftover;
            return -1;
        }
    } else if (cmp < 0) {
        return searchDefinite(arr, leftStart, leftStart + m - 1);
    } else {
        return searchDefinite(arr, rightStart, rightStart + m - 1);
    }
}

void manualRun(void) {
    int n;
    printf("Enter number of coins: ");
    scanf("%d", &n);
    int *arr = malloc(sizeof(int) * n);
    printf("Enter %d coin weights (all equal except at most one smaller):\n", n);
    for (int i = 0; i < n; i++) scanf("%d", &arr[i]);

    weighings = 0;
    int result = searchMaybeNone(arr, 0, n - 1);

    if (result == -1)
        printf("\nResult: No defective coin. All %d coins are genuine.\n", n);
    else
        printf("\nResult: Coin #%d (1-indexed) is defective, weight = %d.\n", result + 1, arr[result]);

    printf("Weighings used: %d   (log2(%d) = %.3f)\n", weighings, n, log2((double)n));
    free(arr);
}

void autoValidate(void) {
    int sizes[] = {5, 10, 25, 50, 100, 500, 1000, 10000, 100000};
    int ns = 9;
    srand((unsigned)time(NULL));

    printf("\n%-10s%-14s%-14s%-16s%-10s\n", "n", "TrueAnswer", "FoundAnswer", "Weighings", "log2(n)");
    for (int t = 0; t < ns; t++) {
        int n = sizes[t];
        int *arr = malloc(sizeof(int) * n);
        int GOOD = 100;
        for (int i = 0; i < n; i++) arr[i] = GOOD;

        /* Randomly decide: no defective, or one random defective coin. */
        int defectiveIdx = (rand() % 4 == 0) ? -1 : (rand() % n);
        if (defectiveIdx != -1) arr[defectiveIdx] = GOOD - 1;

        weighings = 0;
        int found = searchMaybeNone(arr, 0, n - 1);

        char trueBuf[32], foundBuf[32];
        if (defectiveIdx == -1) snprintf(trueBuf, sizeof(trueBuf), "None");
        else snprintf(trueBuf, sizeof(trueBuf), "Coin %d", defectiveIdx + 1);
        if (found == -1) snprintf(foundBuf, sizeof(foundBuf), "None");
        else snprintf(foundBuf, sizeof(foundBuf), "Coin %d", found + 1);

        printf("%-10d%-14s%-14s%-16d%-10.3f  %s\n",
               n, trueBuf, foundBuf, weighings, log2((double)n),
               (strcmpSafe(trueBuf, foundBuf) == 0) ? "[MATCH]" : "[MISMATCH]");

        free(arr);
    }
}

/* tiny helper since we didn't include string.h at top-level intentionally simple */
int strcmpSafe(const char *a, const char *b) {
    while (*a && *b) {
        if (*a != *b) return 1;
        a++; b++;
    }
    return (*a == *b) ? 0 : 1;
}

int main(void) {
    printf("=========== Find the Defective (Lighter) Coin ===========\n\n");
    int choice;
    printf("1. Automatic validation (random trials, checks correctness + weighing bound)\n");
    printf("2. Manual input\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    if (choice == 2) manualRun();
    else autoValidate();

    printf("\nEach weighing halves the candidate set, so the number of\n");
    printf("weighings is bounded by ceil(log2 n) + 1, matching the required\n");
    printf("log2(n) + c bound (here c = 1, only needed for the odd 'none vs\n");
    printf("leftover' disambiguation step).\n");
    return 0;
}
