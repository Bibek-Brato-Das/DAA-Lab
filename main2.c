#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

/* ---------------- Standard 2-way merge sort ---------------- */
static void merge2(int *a, int lo, int mid, int hi, int *tmp) {
    int i=lo, j=mid, k=lo;
    while (i<mid && j<hi) tmp[k++] = (a[i]<=a[j]) ? a[i++] : a[j++];
    while (i<mid) tmp[k++]=a[i++];
    while (j<hi)  tmp[k++]=a[j++];
    memcpy(a+lo, tmp+lo, (hi-lo)*sizeof(int));
}
static void mergesort2(int *a, int lo, int hi, int *tmp) {
    if (hi-lo<=1) return;
    int mid=(lo+hi)/2;
    mergesort2(a,lo,mid,tmp);
    mergesort2(a,mid,hi,tmp);
    merge2(a,lo,mid,hi,tmp);
}

/* ---------------- Modified 3-way merge sort ---------------- */
static void merge3(int *a, int lo, int m1, int m2, int hi, int *tmp) {
    int i=lo, j=m1, k=m2, t=lo;
    while (i<m1 && j<m2 && k<hi) {
        int v = (a[i]<=a[j]) ? (a[i]<=a[k]?a[i++]:a[k++]) : (a[j]<=a[k]?a[j++]:a[k++]);
        tmp[t++]=v;
    }
    while (i<m1 && j<m2) tmp[t++] = (a[i]<=a[j]) ? a[i++] : a[j++];
    while (j<m2 && k<hi) tmp[t++] = (a[j]<=a[k]) ? a[j++] : a[k++];
    while (i<m1 && k<hi) tmp[t++] = (a[i]<=a[k]) ? a[i++] : a[k++];
    while (i<m1) tmp[t++]=a[i++];
    while (j<m2) tmp[t++]=a[j++];
    while (k<hi) tmp[t++]=a[k++];
    memcpy(a+lo, tmp+lo, (hi-lo)*sizeof(int));
}
static void mergesort3(int *a, int lo, int hi, int *tmp) {
    if (hi-lo<=1) return;
    int len=hi-lo, m1=lo+len/3, m2=lo+2*len/3;
    mergesort3(a,lo,m1,tmp);
    mergesort3(a,m1,m2,tmp);
    mergesort3(a,m2,hi,tmp);
    merge3(a,lo,m1,m2,hi,tmp);
}

static double now_us(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC,&ts);
    return ts.tv_sec*1e6 + ts.tv_nsec/1e3;
}

static int is_sorted(int *a, int n){ for(int i=1;i<n;i++) if(a[i-1]>a[i]) return 0; return 1; }

#define NUM_SIZES 12
static const int SIZES[NUM_SIZES] = {1000,2000,4000,8000,16000,32000,64000,128000,256000,512000,1000000,2000000};
static double t2[NUM_SIZES], t3[NUM_SIZES];

static void time_all(void) {
    srand(1);
    for (int s=0;s<NUM_SIZES;s++) {
        int n=SIZES[s];
        int *a=malloc(sizeof(int)*n), *tmp=malloc(sizeof(int)*n);
        for (int i=0;i<n;i++) a[i]=rand();

        int *b=malloc(sizeof(int)*n); memcpy(b,a,sizeof(int)*n);
        double t0=now_us();
        mergesort2(b,0,n,tmp);
        t2[s] = now_us()-t0;
        if (!is_sorted(b,n)) fprintf(stderr,"BUG: mergesort2 failed at n=%d\n",n);
        free(b);

        memcpy(a,a,0); /* no-op */
        int *c=malloc(sizeof(int)*n); memcpy(c,a,sizeof(int)*n);
        t0=now_us();
        mergesort3(c,0,n,tmp);
        t3[s] = now_us()-t0;
        if (!is_sorted(c,n)) fprintf(stderr,"BUG: mergesort3 failed at n=%d\n",n);
        free(c);

        printf("n=%8d  2-way=%9.1f us  3-way=%9.1f us  ratio(2/nlogn)=%.4f ratio(3/nlogn)=%.4f\n",
               n, t2[s], t3[s], t2[s]/(n*log2(n)), t3[s]/(n*log2(n)));
        free(a); free(tmp);
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
    for (int i=0;i<NUM_SIZES;i++){ if(t2[i]>maxv) maxv=t2[i]; if(t3[i]>maxv) maxv=t3[i]; }

    for (int which=0; which<2; which++) {
        double *arr = which==0 ? t2 : t3;
        if (which==0) SDL_SetRenderDrawColor(r,60,180,80,255);   /* green: 2-way */
        else          SDL_SetRenderDrawColor(r,230,60,60,255);   /* red: 3-way */
        for (int i=0;i<NUM_SIZES-1;i++) {
            float x1=margin+(W-margin-40)*(float)i/(NUM_SIZES-1);
            float x2=margin+(W-margin-40)*(float)(i+1)/(NUM_SIZES-1);
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
    SDL_CreateWindowAndRenderer("Q2: merge sort (green) vs 3-way merge sort (red)",900,600,0,&win,&ren);
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
    time_all();
    printf("\nBoth curves should grow roughly like n*log(n); the ratio columns\n"
           "above should roughly flatten out as n grows, confirming Theta(n log n)\n"
           "for both the standard and the 3-way merge sort.\n");
#ifdef USE_SDL
    run_sdl();
#else
    printf("\n(Compile with -DUSE_SDL and link SDL3 for the live graph.)\n");
#endif
    return 0;
}
