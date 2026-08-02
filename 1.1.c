#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    char name[24];
    double log2val;
} Func;

const char *names[] = {
    "1/n", "log2(n)", "n^0.51", "12*sqrt(n)", "50*n^0.5",
    "n*log2(n)", "n^2 - 324", "100n^2 + 6n", "2n^3",
    "n^log2(n)", "3^n", "2^(32n)"
};

double log2_of(double n, int which) {
    double log2n = log2(n);
    switch (which) {
        case 0:  return -log2n;                          /* 1/n          */
        case 1:  return log2(log2n);                      /* log2(n)      */
        case 2:  return 0.51 * log2n;                      /* n^0.51       */
        case 3:  return log2(12.0) + 0.5 * log2n;          /* 12*sqrt(n)   */
        case 4:  return log2(50.0) + 0.5 * log2n;          /* 50*n^0.5     */
        case 5:  return log2n + log2(log2n);               /* n*log2(n)    */
        case 6:  return log2(n * n - 324.0);                /* n^2 - 324    */
        case 7:  return log2(100.0 * n * n + 6.0 * n);       /* 100n^2 + 6n  */
        case 8:  return 1.0 + 3.0 * log2n;                  /* 2n^3         */
        case 9:  return log2n * log2n;                      /* n^log2(n)    */
        case 10: return n * log2(3.0);                       /* 3^n          */
        case 11: return 32.0 * n;                            /* 2^(32n)      */
    }
    return 0;
}

int cmp_func(const void *a, const void *b) {
    double da = ((Func *)a)->log2val, db = ((Func *)b)->log2val;
    return (da > db) - (da < db);
}

void print_order(double n) {
    Func f[12];
    for (int i = 0; i < 12; i++) {
        strcpy(f[i].name, names[i]);
        f[i].log2val = log2_of(n, i);
    }
    qsort(f, 12, sizeof(Func), cmp_func);

    printf("\nOrder of growth at n = %.0f  (sorted by log2 of function value)\n", n);
    for (int i = 0; i < 12; i++)
        printf("  %2d. %-14s  log2(f(n)) ~ %10.3f\n", i + 1, f[i].name, f[i].log2val);
}

void write_csv_data() {
    FILE *fp = fopen("growth_data.csv", "w");
    fprintf(fp, "n");
    for (int i = 0; i < 12; i++) fprintf(fp, ",%s", names[i]);
    fprintf(fp, "\n");

    /* log-spaced n values so the curves look smooth on a log-x plot
       (start at n=32 so n^2-324 stays positive -- fits "sufficiently large n" anyway) */
    for (double exp = 5; exp <= 20; exp += 0.25) {
        double n = pow(2.0, exp);
        fprintf(fp, "%.4f", n);
        for (int i = 0; i < 12; i++) fprintf(fp, ",%.6f", log2_of(n, i));
        fprintf(fp, "\n");
    }
    fclose(fp);
    printf("\nGrowth data (for plotting) saved to growth_data.csv\n");
}

int main() {
    printf("=== Direct values at a small n = 20 (no overflow risk here) ===\n");
    {
        double n = 20;
        printf("  1/n          = %.6f\n", 1.0 / n);
        printf("  log2(n)      = %.4f\n", log2(n));
        printf("  n^0.51       = %.4f\n", pow(n, 0.51));
        printf("  12*sqrt(n)   = %.4f\n", 12 * sqrt(n));
        printf("  50*n^0.5     = %.4f\n", 50 * pow(n, 0.5));
        printf("  n*log2(n)    = %.4f\n", n * log2(n));
        printf("  n^2 - 324    = %.4f\n", n * n - 324);
        printf("  100n^2 + 6n  = %.4f\n", 100 * n * n + 6 * n);
        printf("  2n^3         = %.4f\n", 2 * n * n * n);
        printf("  n^log2(n)    = %.6e\n", pow(n, log2(n)));
        printf("  3^n          = %.6e\n", pow(3, n));
        printf("  2^(32n)      = %.6e\n", pow(2, 32 * n));
    }

    double test_ns[] = {1e3, 1e6, 1e12};
    for (int i = 0; i < 3; i++) print_order(test_ns[i]);

    printf("\n--- A closer look: n^0.51 vs 12*sqrt(n) / 50*n^0.5 ---\n");
    double crossover_log2n = log2(12.0) / 0.01;
    printf("These look deceptively similar since 0.51 is barely above 0.5.\n");
    printf("Crossover: 0.51*log2(n) = log2(12) + 0.5*log2(n)\n");
    printf("        => 0.01*log2(n) = log2(12) = %.3f\n", log2(12.0));
    printf("        => log2(n) = %.1f   =>   n ~ 2^%.1f\n", crossover_log2n, crossover_log2n);
    printf("That n has over 100 digits -- utterly unreachable in any C program\n");
    printf("(or in the physical universe). So for EVERY n you could ever\n");
    printf("actually compute with, 12*sqrt(n) and 50*n^0.5 will look bigger\n");
    printf("than n^0.51, even though n^0.51 is the asymptotically larger\n");
    printf("function in the strict Big-O sense.\n");

    write_csv_data();

    return 0;
}