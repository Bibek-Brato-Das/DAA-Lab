/*
 * DAA Lab-03, Q4: Matrix Multiplication using D&C (Strassen's Method)
 * -----------------------------------------------------------------------
 * Multiplies two n x n square matrices using Strassen's algorithm,
 * which needs only 7 recursive multiplications of (n/2)x(n/2)
 * submatrices instead of the naive 8, giving
 *      T(n) = 7*T(n/2) + O(n^2)  =>  T(n) = O(n^2.807)
 * instead of the naive O(n^3).
 *
 * If n is not a power of 2, the matrices are zero-padded up to the
 * next power of 2 before multiplying, and the extra rows/columns are
 * discarded from the result at the end.
 *
 * A naive O(n^3) multiplication is also provided so the Strassen
 * result can be verified for correctness, and both are timed to show
 * Strassen's asymptotic advantage for larger matrices.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ---------- basic matrix helpers (int** matrices) ---------- */

int** allocMat(int n) {
    int** m = malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) m[i] = calloc(n, sizeof(int));
    return m;
}

void freeMat(int** m, int n) {
    for (int i = 0; i < n; i++) free(m[i]);
    free(m);
}

/* C = A + sign*B  (sign = 1 for add, -1 for subtract) */
int** addMat(int** A, int** B, int n, int sign) {
    int** C = allocMat(n);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            C[i][j] = A[i][j] + sign * B[i][j];
    return C;
}

int nextPowerOf2(int n) {
    int p = 1;
    while (p < n) p <<= 1;
    return p;
}

/* ---------- naive O(n^3) multiplication (for verification/comparison) ---------- */
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

/* ---------- Strassen's D&C multiplication ---------- */
#define STRASSEN_BASE 16  /* switch to naive multiply below this size: cuts overhead */

int** strassenMultiply(int** A, int** B, int n) {
    if (n <= STRASSEN_BASE) return naiveMultiply(A, B, n);

    int half = n / 2;
    int** A11 = allocMat(half); int** A12 = allocMat(half);
    int** A21 = allocMat(half); int** A22 = allocMat(half);
    int** B11 = allocMat(half); int** B12 = allocMat(half);
    int** B21 = allocMat(half); int** B22 = allocMat(half);

    for (int i = 0; i < half; i++)
        for (int j = 0; j < half; j++) {
            A11[i][j] = A[i][j];             A12[i][j] = A[i][j + half];
            A21[i][j] = A[i + half][j];      A22[i][j] = A[i + half][j + half];
            B11[i][j] = B[i][j];             B12[i][j] = B[i][j + half];
            B21[i][j] = B[i + half][j];      B22[i][j] = B[i + half][j + half];
        }

    /* 7 Strassen products */
    int** t1 = addMat(A11, A22, half, 1);
    int** t2 = addMat(B11, B22, half, 1);
    int** M1 = strassenMultiply(t1, t2, half);      /* (A11+A22)(B11+B22) */
    freeMat(t1, half); freeMat(t2, half);

    int** t3 = addMat(A21, A22, half, 1);
    int** M2 = strassenMultiply(t3, B11, half);      /* (A21+A22)B11 */
    freeMat(t3, half);

    int** t4 = addMat(B12, B22, half, -1);
    int** M3 = strassenMultiply(A11, t4, half);      /* A11(B12-B22) */
    freeMat(t4, half);

    int** t5 = addMat(B21, B11, half, -1);
    int** M4 = strassenMultiply(A22, t5, half);      /* A22(B21-B11) */
    freeMat(t5, half);

    int** t6 = addMat(A11, A12, half, 1);
    int** M5 = strassenMultiply(t6, B22, half);      /* (A11+A12)B22 */
    freeMat(t6, half);

    int** t7a = addMat(A21, A11, half, -1);
    int** t7b = addMat(B11, B12, half, 1);
    int** M6 = strassenMultiply(t7a, t7b, half);     /* (A21-A11)(B11+B12) */
    freeMat(t7a, half); freeMat(t7b, half);

    int** t8a = addMat(A12, A22, half, -1);
    int** t8b = addMat(B21, B22, half, 1);
    int** M7 = strassenMultiply(t8a, t8b, half);     /* (A12-A22)(B21+B22) */
    freeMat(t8a, half); freeMat(t8b, half);

    /* Combine into C11, C12, C21, C22 */
    int** x1 = addMat(M1, M4, half, 1);
    int** x2 = addMat(M7, M5, half, -1);
    int** C11 = addMat(x1, x2, half, 1);              /* M1+M4-M5+M7 */
    freeMat(x1, half); freeMat(x2, half);

    int** C12 = addMat(M3, M5, half, 1);              /* M3+M5 */

    int** C21 = addMat(M2, M4, half, 1);              /* M2+M4 */

    int** x3 = addMat(M1, M2, half, -1);
    int** x4 = addMat(M3, M6, half, 1);
    int** C22 = addMat(x3, x4, half, 1);              /* M1-M2+M3+M6 */
    freeMat(x3, half); freeMat(x4, half);

    freeMat(M1, half); freeMat(M2, half); freeMat(M3, half); freeMat(M4, half);
    freeMat(M5, half); freeMat(M6, half); freeMat(M7, half);
    freeMat(A11, half); freeMat(A12, half); freeMat(A21, half); freeMat(A22, half);
    freeMat(B11, half); freeMat(B12, half); freeMat(B21, half); freeMat(B22, half);

    int** C = allocMat(n);
    for (int i = 0; i < half; i++)
        for (int j = 0; j < half; j++) {
            C[i][j] = C11[i][j];
            C[i][j + half] = C12[i][j];
            C[i + half][j] = C21[i][j];
            C[i + half][j + half] = C22[i][j];
        }
    freeMat(C11, half); freeMat(C12, half); freeMat(C21, half); freeMat(C22, half);
    return C;
}

/* Pads A (n x n) into a (m x m) zero-padded matrix, m >= n. */
int** padMatrix(int** A, int n, int m) {
    int** P = allocMat(m); /* calloc already zero-fills */
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            P[i][j] = A[i][j];
    return P;
}

void printMat(int** A, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) printf("%6d ", A[i][j]);
        printf("\n");
    }
}

int matricesEqual(int** A, int** B, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (A[i][j] != B[i][j]) return 0;
    return 1;
}

void manualRun(void) {
    int n;
    printf("Enter matrix size n (for n x n matrices): ");
    scanf("%d", &n);

    int** A = allocMat(n);
    int** B = allocMat(n);
    printf("Enter matrix A (%d x %d):\n", n, n);
    for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) scanf("%d", &A[i][j]);
    printf("Enter matrix B (%d x %d):\n", n, n);
    for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) scanf("%d", &B[i][j]);

    int m = nextPowerOf2(n);
    int** Ap = padMatrix(A, n, m);
    int** Bp = padMatrix(B, n, m);

    int** Cp = strassenMultiply(Ap, Bp, m);

    printf("\nResult A x B (Strassen):\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) printf("%6d ", Cp[i][j]);
        printf("\n");
    }

    /* verify against naive */
    int** Cnaive = naiveMultiply(A, B, n);
    int ok = 1;
    for (int i = 0; i < n && ok; i++)
        for (int j = 0; j < n && ok; j++)
            if (Cp[i][j] != Cnaive[i][j]) ok = 0;
    printf("\nVerification against naive O(n^3) multiplication: %s\n", ok ? "MATCH" : "MISMATCH");

    freeMat(A, n); freeMat(B, n); freeMat(Cnaive, n);
    freeMat(Ap, m); freeMat(Bp, m); freeMat(Cp, m);
}

void autoValidate(void) {
    int sizes[] = {32, 64, 128, 256, 512};
    int ns = 5;
    srand((unsigned)time(NULL));

    printf("\n%-8s%-12s%-12s%-10s\n", "n", "Naive(s)", "Strassen(s)", "Correct?");
    for (int t = 0; t < ns; t++) {
        int n = sizes[t];
        int** A = allocMat(n);
        int** B = allocMat(n);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++) { A[i][j] = rand() % 10; B[i][j] = rand() % 10; }

        clock_t t0 = clock();
        int** Cn = naiveMultiply(A, B, n);
        clock_t t1 = clock();

        int m = nextPowerOf2(n);
        int** Ap = padMatrix(A, n, m);
        int** Bp = padMatrix(B, n, m);
        clock_t t2 = clock();
        int** Cs = strassenMultiply(Ap, Bp, m);
        clock_t t3 = clock();

        int ok = 1;
        for (int i = 0; i < n && ok; i++)
            for (int j = 0; j < n && ok; j++)
                if (Cn[i][j] != Cs[i][j]) ok = 0;

        printf("%-8d%-12.4f%-12.4f%-10s\n", n,
               (double)(t1 - t0) / CLOCKS_PER_SEC,
               (double)(t3 - t2) / CLOCKS_PER_SEC,
               ok ? "YES" : "NO");

        freeMat(A, n); freeMat(B, n); freeMat(Cn, n);
        freeMat(Ap, m); freeMat(Bp, m); freeMat(Cs, m);
    }
    printf("\n(Naive base-case threshold in Strassen is set to %d; below that\n"
           "size Strassen just calls the naive routine, since Strassen's\n"
           "constant-factor overhead makes it slower for small matrices.)\n", STRASSEN_BASE);
}

int main(void) {
    printf("=========== Strassen's Matrix Multiplication (D&C) ===========\n\n");
    int choice;
    printf("1. Automatic validation (random matrices, compares to naive, times both)\n");
    printf("2. Manual input\n");
    printf("Enter choice: ");
    scanf("%d", &choice);

    if (choice == 2) manualRun();
    else autoValidate();

    return 0;
}
