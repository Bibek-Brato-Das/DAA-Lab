#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

double toss_coin(double p_head, long trials, long *head_count) {
    long heads = 0;
    for (long i = 0; i < trials; i++) {
        double r = (double) rand() / ((double) RAND_MAX + 1.0);
        if (r < p_head) heads++;
    }
    *head_count = heads;
    return (double) heads / trials;
}

int main() {
    srand((unsigned int) time(NULL));

    printf("================================================\n");
    printf(" Part A: Fair coin (p = 0.5) - Law of Large Numbers\n");
    printf("================================================\n");
    long trial_counts[] = {10, 100, 1000, 10000, 100000, 1000000, 10000000};
    int n_counts = 7;
    printf("%-12s%-12s%-15s%-15s\n", "Trials", "Heads", "P(Head)", "|Error| vs 0.5");
    for (int i = 0; i < n_counts; i++) {
        long heads;
        double p = toss_coin(0.5, trial_counts[i], &heads);
        printf("%-12ld%-12ld%-15.5f%-15.5f\n", trial_counts[i], heads, p, fabs(p - 0.5));
    }
    printf("-> As the number of tosses grows, P(Head) converges to 0.5\n");
    printf("   and the error shrinks roughly like 1/sqrt(trials).\n");

    printf("\n================================================\n");
    printf(" Part B: Fair vs Biased coins (1,000,000 tosses each)\n");
    printf("================================================\n");
    double biases[]     = {0.5,    0.2,                     0.35,             0.65,             0.8};
    const char *labels[] = {"Fair", "Heavily biased (tails)", "Biased (tails)", "Biased (heads)", "Heavily biased (heads)"};
    printf("%-28s%-10s%-15s%-15s\n", "Coin", "True p", "Simulated P", "|Error|");
    for (int i = 0; i < 5; i++) {
        long heads;
        double p = toss_coin(biases[i], 1000000, &heads);
        printf("%-28s%-10.2f%-15.5f%-15.5f\n", labels[i], biases[i], p, fabs(p - biases[i]));
    }
    printf("-> Every coin's simulated P(Head) converges to its own true bias p,\n");
    printf("   confirming the simulation correctly models a Bernoulli(p) process.\n");

    return 0;
}