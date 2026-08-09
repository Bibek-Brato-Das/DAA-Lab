#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* standard 2-way merge of two sorted runs a[l1..r1), a[l2..r2) into out */
static int* merge_two(int *a, int len_a, int *b, int len_b) {
    int *out = malloc(sizeof(int)*(len_a+len_b));
    int i=0,j=0,k=0;
    while (i<len_a && j<len_b) out[k++] = (a[i]<=b[j]) ? a[i++] : b[j++];
    while (i<len_a) out[k++]=a[i++];
    while (j<len_b) out[k++]=b[j++];
    return out;
}

/* ---------------- Method 1: sequential merge ---------------- */
static int* merge_sequential(int **arrays, int k, int n) {
    int *acc = malloc(sizeof(int)*n);
    memcpy(acc, arrays[0], sizeof(int)*n);
    int acc_len = n;
    for (int i=1;i<k;i++) {
        int *merged = merge_two(acc, acc_len, arrays[i], n);
        free(acc);
        acc = merged;
        acc_len += n;
    }
    return acc; /* length k*n */
}

/* ---------------- Method 2: pairwise / tournament merge ---------------- */
static int* merge_pairwise(int **arrays, int k, int n) {
    /* copy inputs into a working list of (ptr,len) pairs, repeatedly merge
       neighbours until one array remains */
    int **cur = malloc(sizeof(int*)*k);
    int *lens = malloc(sizeof(int)*k);
    for (int i=0;i<k;i++) { cur[i]=malloc(sizeof(int)*n); memcpy(cur[i],arrays[i],sizeof(int)*n); lens[i]=n; }
    int cnt=k;
    while (cnt>1) {
        int newcnt = (cnt+1)/2;
        int **next = malloc(sizeof(int*)*newcnt);
        int *nlens = malloc(sizeof(int)*newcnt);
        int idx=0;
        for (int i=0;i<cnt;i+=2) {
            if (i+1<cnt) {
                next[idx] = merge_two(cur[i], lens[i], cur[i+1], lens[i+1]);
                nlens[idx] = lens[i]+lens[i+1];
                free(cur[i]); free(cur[i+1]);
            } else {
                next[idx]=cur[i]; nlens[idx]=lens[i]; /* odd one out, carried forward */
            }
            idx++;
        }
        free(cur); free(lens);
        cur=next; lens=nlens; cnt=newcnt;
    }
    int *result = cur[0];
    free(cur); free(lens);
    return result;
}

static int is_sorted(int *a,int n){ for(int i=1;i<n;i++) if(a[i-1]>a[i]) return 0; return 1; }
static double now_us(void){ struct timespec ts; clock_gettime(CLOCK_MONOTONIC,&ts); return ts.tv_sec*1e6+ts.tv_nsec/1e3; }

#define NUM_KS 10
static const int KS[NUM_KS] = {2,4,8,16,32,64,128,256,512,1024};
static const int N = 500; /* elements per array, held fixed while k grows */
static double t_seq[NUM_KS], t_pair[NUM_KS];

static int cmp_int(const void *a,const void *b){ return (*(int*)a)-(*(int*)b); }

static void time_all(void) {
    srand(7);
    for (int s=0;s<NUM_KS;s++) {
        int k=KS[s];
        int **arrays = malloc(sizeof(int*)*k);
        for (int i=0;i<k;i++) {
            arrays[i]=malloc(sizeof(int)*N);
            for (int j=0;j<N;j++) arrays[i][j]=rand()%1000000;
            qsort(arrays[i],N,sizeof(int),cmp_int);
        }

        double t0=now_us();
        int *r1 = merge_sequential(arrays,k,N);
        t_seq[s]=now_us()-t0;
        if (!is_sorted(r1,k*N)) fprintf(stderr,"BUG: sequential merge failed at k=%d\n",k);
        free(r1);

        t0=now_us();
        int *r2 = merge_pairwise(arrays,k,N);
        t_pair[s]=now_us()-t0;
        if (!is_sorted(r2,k*N)) fprintf(stderr,"BUG: pairwise merge failed at k=%d\n",k);
        free(r2);

        printf("k=%5d  seq=%9.1f us  pairwise=%9.1f us  seq/(n*k^2)=%.5f  pairwise/(n*k*log2(k))=%.5f\n",
               k, t_seq[s], t_pair[s],
               t_seq[s]/((double)N*k*k),
               t_pair[s]/((double)N*k*log2(k)));

        for (int i=0;i<k;i++) free(arrays[i]);
        free(arrays);
    }
}

#ifdef USE_SDL
#include <SDL3/SDL.h>
static void draw_plot(SDL_Renderer *r) {
    SDL_SetRenderDrawColor(r,20,20,25,255); SDL_RenderClear(r);
    int W=900,H=600,margin=60;
    SDL_SetRenderDrawColor(r,200,200,200,255);
    SDL_RenderLine(r,margin,H-margin,W-20,H-margin);
    SDL_RenderLine(r,margin,H-margin,margin,20);

    double maxv=1.0;
    for (int i=0;i<NUM_KS;i++){ if(t_seq[i]>maxv) maxv=t_seq[i]; if(t_pair[i]>maxv) maxv=t_pair[i]; }

    for (int which=0; which<2; which++) {
        double *arr = which==0 ? t_seq : t_pair;
        if (which==0) SDL_SetRenderDrawColor(r,230,60,60,255);   /* red: sequential O(nk^2) */
        else          SDL_SetRenderDrawColor(r,60,180,80,255);   /* green: pairwise O(nk log k) */
        for (int i=0;i<NUM_KS-1;i++) {
            float x1=margin+(W-margin-40)*(float)i/(NUM_KS-1);
            float x2=margin+(W-margin-40)*(float)(i+1)/(NUM_KS-1);
            float y1=(H-margin)-(H-margin-40)*(float)(arr[i]/maxv);
            float y2=(H-margin)-(H-margin-40)*(float)(arr[i+1]/maxv);
            SDL_RenderLine(r,x1,y1,x2,y2);
        }
    }
    SDL_RenderPresent(r);
}
static void run_sdl(void) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win; SDL_Renderer *ren;
    SDL_CreateWindowAndRenderer("Q3: sequential merge (red, O(nk^2)) vs pairwise (green, O(nk log k))",900,600,0,&win,&ren);
    draw_plot(ren);
    int running=1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type==SDL_EVENT_QUIT) running=0;
            if (e.type==SDL_EVENT_KEY_DOWN && e.key.key==SDLK_ESCAPE) running=0;
        }
        SDL_Delay(16);
    }
    SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); SDL_Quit();
}
#endif

int main(void) {
    printf("n (elements per array) fixed at %d, k varies.\n", N);
    time_all();
    printf("\nseq/(n*k^2) and pairwise/(n*k*log2 k) should both roughly flatten\n"
           "as k grows, confirming Theta(nk^2) for Method 1 and Theta(nk log k)\n"
           "for Method 2. Notice the pairwise method pulls ahead as k grows.\n");
#ifdef USE_SDL
    run_sdl();
#else
    printf("\n(Compile with -DUSE_SDL and link SDL3 for the live graph.)\n");
#endif
    return 0;
}
