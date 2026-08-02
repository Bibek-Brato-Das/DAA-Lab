#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int has_dup_bruteforce(int a[], int n) {
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (a[i] == a[j]) return 1;
    return 0;
}

int cmp_int(const void *x, const void *y) { return (*(int *) x - *(int *) y); }

int has_dup_sorting(int a[], int n) {
    int *tmp = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) tmp[i] = a[i];
    qsort(tmp, n, sizeof(int), cmp_int);
    int found = 0;
    for (int i = 0; i < n - 1; i++)
        if (tmp[i] == tmp[i + 1]) { found = 1; break; }
    free(tmp);
    return found;
}

typedef struct Node { int val; struct Node *next; } Node;

int has_dup_hashing(int a[], int n) {
    int table_size = 2 * n + 1;
    Node **table = calloc(table_size, sizeof(Node *));
    int found = 0;
    for (int i = 0; i < n; i++) {
        unsigned int h = ((unsigned int) a[i]) % table_size;
        for (Node *cur = table[h]; cur; cur = cur->next)
            if (cur->val == a[i]) { found = 1; break; }
        if (found) break;
        Node *node = malloc(sizeof(Node));
        node->val = a[i]; node->next = table[h]; table[h] = node;
    }
    for (int i = 0; i < table_size; i++) {
        Node *cur = table[i];
        while (cur) { Node *nx = cur->next; free(cur); cur = nx; }
    }
    free(table);
    return found;
}

int *rand_array(int n, int range) {
    int *a = malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) a[i] = rand() % range;
    return a;
}

int main() {
    srand((unsigned int) time(NULL));

    int demo[] = {4, 8, 15, 16, 23, 15};
    printf("Sanity check on {4,8,15,16,23,15}: brute=%d sort=%d hash=%d (expect 1,1,1)\n\n",
           has_dup_bruteforce(demo, 6), has_dup_sorting(demo, 6), has_dup_hashing(demo, 6));

    int sizes[] = {500, 1000, 2000, 4000, 8000, 16000};
    int n_sizes = 6;
    int range = 1000000000; /* huge range: duplicates essentially rare by chance */

    FILE *fp = fopen("uniqueness_data.csv", "w");
    fprintf(fp, "n,brute_ms,sort_ms,hash_ms\n");

    printf("=== Timing comparison (range = %d) ===\n", range);
    printf("%-8s%-14s%-14s%-14s\n", "n", "Brute (ms)", "Sort (ms)", "Hash (ms)");
    for (int i = 0; i < n_sizes; i++) {
        int n = sizes[i];
        int *a = rand_array(n, range);

        volatile int sink; /* prevents the compiler from optimising away unused results */
        clock_t t0 = clock(); sink = has_dup_bruteforce(a, n);
        clock_t t1 = clock(); sink = has_dup_sorting(a, n);
        clock_t t2 = clock(); sink = has_dup_hashing(a, n);
        clock_t t3 = clock();
        (void) sink;

        double bms = 1000.0 * (t1 - t0) / CLOCKS_PER_SEC;
        double sms = 1000.0 * (t2 - t1) / CLOCKS_PER_SEC;
        double hms = 1000.0 * (t3 - t2) / CLOCKS_PER_SEC;

        printf("%-8d%-14.3f%-14.3f%-14.3f\n", n, bms, sms, hms);
        fprintf(fp, "%d,%.3f,%.3f,%.3f\n", n, bms, sms, hms);
        free(a);
    }
    fclose(fp);

    printf("\n=== Birthday-paradox effect: smaller value range = %d ===\n", 10000);
    int small_range = 10000, trials = 500;
    int bday_sizes[] = {10, 30, 60, 100, 140, 200, 300, 500};
    int n_bday = 8;
    printf("%-8s%-10s%-20s\n", "n", "range", "P(duplicate found)");
    for (int i = 0; i < n_bday; i++) {
        int n = bday_sizes[i];
        int dup_hits = 0;
        for (int t = 0; t < trials; t++) {
            int *a = rand_array(n, small_range);
            if (has_dup_hashing(a, n)) dup_hits++;
            free(a);
        }
        printf("%-8d%-10d%-20.3f\n", n, small_range, (double) dup_hits / trials);
    }
    printf("-> sqrt(range) = sqrt(%d) ~ %.0f: notice P crosses ~0.5 near there,\n", small_range, sqrt((double) small_range));
    printf("   the classic birthday-paradox threshold.\n");

    return 0;
}