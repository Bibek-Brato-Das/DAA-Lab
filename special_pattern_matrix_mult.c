/*
 * DAA Lab-03, Q5: Multiplying Special-Pattern Square Matrices in O(n^2)
 * --------------------------------------------------------------------
 * Both n x n matrices (n = 2^k) have the recursive block form
 *
 *          M = [ M1  M2 ]
 *              [ M2  M1 ]
 *
 * where M1 and M2 are (n/2)x(n/2) blocks that ALSO have this same
 * pattern recursively, all the way down to single integers.
 *
 * KEY IDEA (why O(n^2) is possible, not just O(n^2.8) via Strassen)
 * -------------------------------------------------------------------
 * If A = [[A1,A2],[A2,A1]] and B = [[B1,B2],[B2,B1]], ordinary block
 * multiplication gives:
 *      C11 = A1*B1 + A2*B2
 *      C12 = A1*B2 + A2*B1
 *      C21 = C12          (same expression)
 *      C22 = C11          (same expression)
 * so the product ALSO has this pattern: C = [[C11,C12],[C12,C11]].
 * Naively this needs FOUR half-size multiplications (A1B1, A2B2,
 * A1B2, A2B1), giving T(n) = 4T(n/2) + O(n^2) = O(n^2 log n) -- too
 * slow.
 *
 * Trick: treat "swap the two blocks" as an algebraic operator X with
 * X^2 = I (like the matrices M1*I + M2*X). Using the idempotents
 *      e1 = (I+X)/2 ,  e2 = (I-X)/2         (e1+e2=I, e1*e2=0)
 * one can show:
 *      A*B = P*e1 + Q*e2 ,  where
 *          P = (A1+A2)(B1+B2)
 *          Q = (A1-A2)(B1-B2)
 * and converting back:
 *          C1 = (P+Q)/2      (this is C11 above)
 *          C2 = (P-Q)/2      (this is C12 above)
 * Crucially, (A1+A2), (A1-A2), (B1+B2), (B1-B2) are THEMSELVES
 * matrices with the very same recursive pattern (sum/difference of
 * two "M1,M2-form" matrices is again of that form), so P and Q are
 * each just a special-pattern-matrix multiplication of half the
 * size -- i.e. only TWO recursive multiplications are needed, not
 * four:
 *      T(n) = 2*T(n/2) + O(n^2)   =>  T(n) = O(n^2)      (Master Thm, case 3)
 *
 * The O(n^2) term per level is the cost of forming the four
 * (n/2)x(n/2) sum/difference matrices and combining P,Q back into
 * C1,C2 -- ordinary elementwise matrix operations.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ---------------- basic matrix helpers ---------------- */

int** allocMat(int n) {
    int** m = malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) m[i] = calloc(n, sizeof(int));
    return m;
}
void freeMat(int** m, int n) {
    for (int i = 0; i < n; i++) free(m[i]);
    free(m);
}
int** addMat(int** A, int** B, int n, int sign) {
    int** C = allocMat(n);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            C[i][j] = A[i][j] + sign * B[i][j];
    return C;
}
void copyBlock(int** src, int** dst, int rowOff, int colOff, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            dst[i][j] = src[rowOff + i][colOff + j];
}

/* ---------------- naive O(n^3) multiply, for verification ---------------- */
int** naiveMultiply(int** A, int** B, int n) {
    int** C = allocMat(n);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            long sum = 0;
            for (int k = 0; k < n; k++) sum += (long)A[i][k] * B[k][j];
            C[i][j] = (int)sum;
        }
    return C;
}

/* ---------------- generate a random special-pattern matrix ---------------- */
/* Consumes exactly n values from 'vals' (starting at *idx) and builds the
 * n x n matrix [[M1,M2],[M2,M1]] recursively (M1,M2 each (n/2)x(n/2),
 * themselves built from the same recursive pattern). A size-n special
 * matrix needs exactly n independent scalar values -- that's the whole
 * point of the pattern being "self-similar" at every recursion level. */
int** buildSpecial(int vals[], int* idx, int n) {
    if (n == 1) {
        int** M = allocMat(1);
        M[0][0] = vals[(*idx)++];
        return M;
    }
    int half = n / 2;
    int** M1 = buildSpecial(vals, idx, half);
    int** M2 = buildSpecial(vals, idx, half);
    int** M = allocMat(n);
    for (int i = 0; i < half; i++)
        for (int j = 0; j < half; j++) {
            M[i][j] = M1[i][j];
            M[i][j + half] = M2[i][j];
            M[i + half][j] = M2[i][j];
            M[i + half][j + half] = M1[i][j];
        }
    freeMat(M1, half); freeMat(M2, half);
    return M;
}

int** randomSpecialMatrix(int n, int maxVal) {
    int* vals = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) vals[i] = rand() % maxVal;
    int idx = 0;
    int** M = buildSpecial(vals, &idx, n);
    free(vals);
    return M;
}

/* checks a matrix truly has the [[M1,M2],[M2,M1]] pattern recursively */
int isSpecialPattern(int** M, int n) {
    if (n == 1) return 1;
    int half = n / 2;
    for (int i = 0; i < half; i++)
        for (int j = 0; j < half; j++) {
            if (M[i][j] != M[i + half][j + half]) return 0;      /* diag blocks equal */
            if (M[i][j + half] != M[i + half][j]) return 0;      /* off-diag blocks equal */
        }
    int** M1 = allocMat(half); copyBlock(M, M1, 0, 0, half);
    int ok = isSpecialPattern(M1, half);
    freeMat(M1, half);
    return ok;
}

/* ---------------- the O(n^2) special multiplication ---------------- */
int** specialMultiply(int** A, int** B, int n) {
    if (n == 1) {
        int** C = allocMat(1);
        C[0][0] = A[0][0] * B[0][0];
        return C;
    }
    int half = n / 2;
    int** A1 = allocMat(half); int** A2 = allocMat(half);
    int** B1 = allocMat(half); int** B2 = allocMat(half);
    copyBlock(A, A1, 0, 0, half);       copyBlock(A, A2, 0, half, half);
    copyBlock(B, B1, 0, 0, half);       copyBlock(B, B2, 0, half, half);

    int** sumA  = addMat(A1, A2, half, 1);
    int** diffA = addMat(A1, A2, half, -1);
    int** sumB  = addMat(B1, B2, half, 1);
    int** diffB = addMat(B1, B2, half, -1);

    int** P = specialMultiply(sumA, sumB, half);   /* (A1+A2)(B1+B2) */
    int** Q = specialMultiply(diffA, diffB, half);  /* (A1-A2)(B1-B2) */

    int** C1 = allocMat(half);
    int** C2 = allocMat(half);
    for (int i = 0; i < half; i++)
        for (int j = 0; j < half; j++) {
            C1[i][j] = (P[i][j] + Q[i][j]) / 2;
            C2[i][j] = (P[i][j] - Q[i][j]) / 2;
        }

    int** C = allocMat(n);
    for (int i = 0; i < half; i++)
        for (int j = 0; j < half; j++) {
            C[i][j] = C1[i][j];
            C[i][j + half] = C2[i][j];
            C[i + half][j] = C2[i][j];
            C[i + half][j + half] = C1[i][j];
        }

    freeMat(A1, half); freeMat(A2, half); freeMat(B1, half); freeMat(B2, half);
    freeMat(sumA, half); freeMat(diffA, half); freeMat(sumB, half); freeMat(diffB, half);
    freeMat(P, half); freeMat(Q, half); freeMat(C1, half); freeMat(C2, half);
    return C;
}

void printMat(int** A, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) printf("%6d ", A[i][j]);
        printf("\n");
    }
}

void manualRun(void) {
    int k;
    printf("Enter k so that matrix size n = 2^k: ");
    scanf("%d", &k);
    int n = 1 << k;

    srand((unsigned)time(NULL));
    printf("\nGenerating two random %d x %d special-pattern matrices...\n", n, n);
    int** A = randomSpecialMatrix(n, 10);
    int** B = randomSpecialMatrix(n, 10);

    printf("\nMatrix A:\n"); printMat(A, n);
    printf("\nMatrix B:\n"); printMat(B, n);

    int** C = specialMultiply(A, B, n);
    printf("\nProduct A x B (via O(n^2) special algorithm):\n");
    printMat(C, n);

    printf("\nDoes result also have the special pattern? %s\n",
           isSpecialPattern(C, n) ? "YES" : "NO");

    int** Cn = naiveMultiply(A, B, n);
    int ok = 1;
    for (int i = 0; i < n && ok; i++)
        for (int j = 0; j < n && ok; j++)
            if (C[i][j] != Cn[i][j]) ok = 0;
    printf("Verification against naive O(n^3) multiplication: %s\n", ok ? "MATCH" : "MISMATCH");

    freeMat(A, n); freeMat(B, n); freeMat(C, n); freeMat(Cn, n);
}

void autoValidate(void) {
    int sizes[] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
    int ns = 10;
    srand((unsigned)time(NULL));

    printf("\n%-8s%-10s%-14s%-14s%-10s\n", "n", "Correct?", "Naive(s)", "Special(s)", "Pattern?");
    for (int t = 0; t < ns; t++) {
        int n = sizes[t];
        int** A = randomSpecialMatrix(n, 10);
        int** B = randomSpecialMatrix(n, 10);

        clock_t t0 = clock();
        int** Cn = naiveMultiply(A, B, n);
        clock_t t1 = clock();

        clock_t t2 = clock();
        int** Cs = specialMultiply(A, B, n);
        clock_t t3 = clock();

        int ok = 1;
        for (int i = 0; i < n && ok; i++)
            for (int j = 0; j < n && ok; j++)
                if (Cn[i][j] != Cs[i][j]) ok = 0;

        printf("%-8d%-10s%-14.5f%-14.5f%-10s\n", n, ok ? "YES" : "NO",
               (double)(t1 - t0) / CLOCKS_PER_SEC,
               (double)(t3 - t2) / CLOCKS_PER_SEC,
               isSpecialPattern(Cs, n) ? "YES" : "NO");

        freeMat(A, n); freeMat(B, n); freeMat(Cn, n); freeMat(Cs, n);
    }
    printf("\nSince the naive method is O(n^3) and the special method is O(n^2),\n");
    printf("watch the 'Special(s)' column grow roughly x4 per doubling of n\n");
    printf("(quadratic), while 'Naive(s)' grows roughly x8 per doubling (cubic).\n");
}

int main(void) {
    printf("=== O(n^2) Multiplication of Special-Pattern [[M1,M2],[M2,M1]] Matrices ===\n\n");
    int choice;
    printf("1. Automatic validation (correctness + timing growth n^2 vs n^3)\n");
    printf("2. Manual demo with a chosen size (prints matrices)\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    if (choice == 2) manualRun();
    else autoValidate();

    return 0;
}
