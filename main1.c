#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define NUM_STRUCTS 6
#define NUM_SIZES   8
#define TRIALS      200      /* operations averaged per (structure, n) */
static const int SIZES[NUM_SIZES] = {500,1000,2000,4000,6000,8000,12000,16000};
static const char *STRUCT_NAMES[NUM_STRUCTS] = {
    "Unsorted array", "Sorted array",
    "Singly linked unsorted", "Singly linked sorted",
    "Doubly linked unsorted", "Doubly linked sorted"
};

/* ---------------------------------------------------------------------- */
/* Node types                                                              */
typedef struct SNode { int key; struct SNode *next; } SNode;
typedef struct DNode { int key; struct DNode *next, *prev; } DNode;

#define INT_MIN_LOCAL (-2147483647)
#define INT_MAX_LOCAL (2147483647)

/* ======================== Unsorted array ================================ */
static int ua_search(int *a, int n, int key) {
    for (int i = 0; i < n; i++) if (a[i] == key) return i;
    return -1;
}
static int ua_insert(int *a, int n, int key) { a[n] = key; return n + 1; }
static int ua_delete(int *a, int n, int idx) { /* O(1): swap with last */
    a[idx] = a[n - 1]; return n - 1;
}
static int ua_max(int *a, int n) { int m = a[0]; for (int i=1;i<n;i++) if (a[i]>m) m=a[i]; return m; }
static int ua_min(int *a, int n) { int m = a[0]; for (int i=1;i<n;i++) if (a[i]<m) m=a[i]; return m; }
static int ua_predecessor(int *a, int n, int key) {
    int best = INT_MIN_LOCAL; for (int i=0;i<n;i++) if (a[i]<key && a[i]>best) best=a[i]; return best;
}
static int ua_successor(int *a, int n, int key) {
    int best = INT_MAX_LOCAL; for (int i=0;i<n;i++) if (a[i]>key && a[i]<best) best=a[i]; return best;
}

/* ======================== Sorted array =================================== */
static int sa_search(int *a, int n, int key) { /* binary search, O(log n) */
    int lo=0, hi=n-1;
    while (lo<=hi) { int mid=(lo+hi)/2; if (a[mid]==key) return mid; if (a[mid]<key) lo=mid+1; else hi=mid-1; }
    return -1;
}
static int sa_insert(int *a, int n, int key) { /* find spot (binary search) then shift, O(n) */
    int lo=0, hi=n;
    while (lo<hi) { int mid=(lo+hi)/2; if (a[mid]<key) lo=mid+1; else hi=mid; }
    for (int i=n; i>lo; i--) a[i]=a[i-1];
    a[lo]=key;
    return n+1;
}
static int sa_delete(int *a, int n, int idx) { /* shift left, O(n) */
    for (int i=idx; i<n-1; i++) a[i]=a[i+1];
    return n-1;
}
static int sa_max(int *a, int n) { return a[n-1]; }
static int sa_min(int *a, int n) { return a[0]; }
static int sa_predecessor(int *a, int n, int key) {
    int idx = sa_search(a,n,key); if (idx<=0) return INT_MIN_LOCAL; return a[idx-1];
}
static int sa_successor(int *a, int n, int key) {
    int idx = sa_search(a,n,key); if (idx<0 || idx>=n-1) return INT_MAX_LOCAL; return a[idx+1];
}

/* ======================== Singly linked, unsorted ========================= */
static SNode* su_insert(SNode *head, int key) { /* O(1): push front */
    SNode *n = malloc(sizeof(SNode)); n->key=key; n->next=head; return n;
}
static SNode* su_search(SNode *head, int key) {
    for (SNode *p=head; p; p=p->next) if (p->key==key) return p;
    return NULL;
}
static SNode* su_delete(SNode *head, SNode *target) { /* O(n): must find predecessor */
    if (head==target) { SNode *nx=head->next; free(head); return nx; }
    SNode *p=head;
    while (p->next && p->next!=target) p=p->next;
    if (p->next==target) { p->next=target->next; free(target); }
    return head;
}
static int su_max(SNode *head) { int m=head->key; for (SNode*p=head;p;p=p->next) if (p->key>m) m=p->key; return m; }
static int su_min(SNode *head) { int m=head->key; for (SNode*p=head;p;p=p->next) if (p->key<m) m=p->key; return m; }
static int su_predecessor(SNode *head, int key) {
    int best=INT_MIN_LOCAL; for (SNode*p=head;p;p=p->next) if (p->key<key && p->key>best) best=p->key; return best;
}
static int su_successor(SNode *head, int key) {
    int best=INT_MAX_LOCAL; for (SNode*p=head;p;p=p->next) if (p->key>key && p->key<best) best=p->key; return best;
}

/* ======================== Singly linked, sorted ============================ */
static SNode* ss_insert(SNode *head, int key) { /* O(n): walk to correct spot */
    SNode *n = malloc(sizeof(SNode)); n->key=key;
    if (!head || key<head->key) { n->next=head; return n; }
    SNode *p=head;
    while (p->next && p->next->key<key) p=p->next;
    n->next=p->next; p->next=n;
    return head;
}
static SNode* ss_search(SNode *head, int key) {
    for (SNode *p=head; p; p=p->next) { if (p->key==key) return p; if (p->key>key) break; }
    return NULL;
}
static SNode* ss_delete(SNode *head, SNode *target) { return su_delete(head, target); } /* still O(n) */
static int ss_min(SNode *head) { return head->key; }
static int ss_max(SNode *head) { SNode *p=head; while (p->next) p=p->next; return p->key; } /* O(n) */
static int ss_predecessor(SNode *head, int key) {
    int prev = INT_MIN_LOCAL;
    for (SNode *p=head; p && p->key<key; p=p->next) prev=p->key;
    return prev;
}
static int ss_successor(SNode *head, int key) {
    for (SNode *p=head; p; p=p->next) if (p->key>key) return p->key;
    return INT_MAX_LOCAL;
}

/* ======================== Doubly linked, unsorted ============================ */
static DNode* du_insert(DNode *head, int key) { /* O(1) */
    DNode *n=malloc(sizeof(DNode)); n->key=key; n->prev=NULL; n->next=head;
    if (head) head->prev=n;
    return n;
}
static DNode* du_search(DNode *head, int key) {
    for (DNode *p=head; p; p=p->next) if (p->key==key) return p;
    return NULL;
}
static DNode* du_delete(DNode *head, DNode *t) { /* O(1) */
    if (t->prev) t->prev->next=t->next; else head=t->next;
    if (t->next) t->next->prev=t->prev;
    free(t);
    return head;
}
static int du_max(DNode *head){int m=head->key; for(DNode*p=head;p;p=p->next) if(p->key>m) m=p->key; return m;}
static int du_min(DNode *head){int m=head->key; for(DNode*p=head;p;p=p->next) if(p->key<m) m=p->key; return m;}
static int du_predecessor(DNode *head,int key){int best=INT_MIN_LOCAL; for(DNode*p=head;p;p=p->next) if(p->key<key&&p->key>best) best=p->key; return best;}
static int du_successor(DNode *head,int key){int best=INT_MAX_LOCAL; for(DNode*p=head;p;p=p->next) if(p->key>key&&p->key<best) best=p->key; return best;}

/* ======================== Doubly linked, sorted (with tail) ==================== */
typedef struct { DNode *head, *tail; } DList;
static void ds_insert(DList *L, int key) { /* O(n): find spot */
    DNode *n=malloc(sizeof(DNode)); n->key=key;
    if (!L->head || key<L->head->key) {
        n->prev=NULL; n->next=L->head;
        if (L->head) L->head->prev=n; else L->tail=n;
        L->head=n; return;
    }
    DNode *p=L->head;
    while (p->next && p->next->key<key) p=p->next;
    n->next=p->next; n->prev=p;
    if (p->next) p->next->prev=n; else L->tail=n;
    p->next=n;
}
static DNode* ds_search(DList *L, int key) {
    for (DNode *p=L->head; p; p=p->next) { if (p->key==key) return p; if (p->key>key) break; }
    return NULL;
}
static void ds_delete(DList *L, DNode *t) { /* O(1) */
    if (t->prev) t->prev->next=t->next; else L->head=t->next;
    if (t->next) t->next->prev=t->prev; else L->tail=t->prev;
    free(t);
}

/* ---------------------------------------------------------------------- */
/* Timing harness                                                          */
typedef enum { OP_SEARCH=0, OP_INSERT=1, OP_DELETE=2 } OpType;
static double results_us[3][NUM_STRUCTS][NUM_SIZES]; /* op x structure x size */

static double now_us(void) {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec*1e6 + ts.tv_nsec/1e3;
}

static void time_all(void) {
    srand(42);
    for (int s = 0; s < NUM_SIZES; s++) {
        int n = SIZES[s];
        int *keys = malloc(sizeof(int)*n);
        for (int i=0;i<n;i++) keys[i]=i*2; /* distinct even keys, easy to build sorted */

        /* ---- Unsorted array ---- */
        {
            int *a = malloc(sizeof(int)*(n+1));
            for (int i=0;i<n;i++) a[i]=keys[i];
            double t0=now_us();
            for (int k=0;k<TRIALS;k++) ua_search(a,n, keys[rand()%n]);
            results_us[OP_SEARCH][0][s] = (now_us()-t0)/TRIALS;

            t0=now_us();
            int nn=n;
            for (int k=0;k<TRIALS;k++) { nn = ua_insert(a,nn, -1); nn = ua_delete(a,nn,nn-1); }
            results_us[OP_INSERT][0][s] = (now_us()-t0)/(2.0*TRIALS);

            t0=now_us();
            nn=n;
            for (int k=0;k<TRIALS;k++) { nn = ua_delete(a,nn,rand()%nn); nn = ua_insert(a,nn,keys[k%n]); }
            results_us[OP_DELETE][0][s] = (now_us()-t0)/(2.0*TRIALS);
            free(a);
        }

        /* ---- Sorted array ---- */
        {
            int *a = malloc(sizeof(int)*(n+1));
            for (int i=0;i<n;i++) a[i]=keys[i]; /* already sorted */
            double t0=now_us();
            for (int k=0;k<TRIALS;k++) sa_search(a,n, keys[rand()%n]);
            results_us[OP_SEARCH][1][s] = (now_us()-t0)/TRIALS;

            t0=now_us();
            int nn=n;
            for (int k=0;k<TRIALS;k++) { nn = sa_insert(a,nn,-1); nn = sa_delete(a,nn,0); }
            results_us[OP_INSERT][1][s] = (now_us()-t0)/(2.0*TRIALS);

            t0=now_us();
            nn=n;
            for (int k=0;k<TRIALS;k++) { nn = sa_delete(a,nn,nn/2); nn = sa_insert(a,nn,keys[k%n]); }
            results_us[OP_DELETE][1][s] = (now_us()-t0)/(2.0*TRIALS);
            free(a);
        }

        /* ---- Singly linked unsorted ---- */
        {
            SNode *head=NULL;
            for (int i=0;i<n;i++) head = su_insert(head, keys[i]);
            double t0=now_us();
            for (int k=0;k<TRIALS;k++) su_search(head, keys[rand()%n]);
            results_us[OP_SEARCH][2][s] = (now_us()-t0)/TRIALS;

            t0=now_us();
            for (int k=0;k<TRIALS;k++) { head = su_insert(head,-1); head = su_delete(head, head); }
            results_us[OP_INSERT][2][s] = (now_us()-t0)/(2.0*TRIALS);

            t0=now_us();
            for (int k=0;k<TRIALS;k++) {
                SNode *target = su_search(head, keys[k%n]);
                if (target) { int kv=target->key; head = su_delete(head, target); head = su_insert(head, kv); }
            }
            results_us[OP_DELETE][2][s] = (now_us()-t0)/(2.0*TRIALS);

            while (head) { SNode *nx=head->next; free(head); head=nx; }
        }

        /* ---- Singly linked sorted ---- */
        {
            SNode *head=NULL;
            for (int i=n-1;i>=0;i--) head = ss_insert(head, keys[i]);
            double t0=now_us();
            for (int k=0;k<TRIALS;k++) ss_search(head, keys[rand()%n]);
            results_us[OP_SEARCH][3][s] = (now_us()-t0)/TRIALS;

            t0=now_us();
            for (int k=0;k<TRIALS;k++) {
                head = ss_insert(head,-1);
                SNode *target = ss_search(head,-1);
                head = ss_delete(head, target);
            }
            results_us[OP_INSERT][3][s] = (now_us()-t0)/(2.0*TRIALS);

            t0=now_us();
            for (int k=0;k<TRIALS;k++) {
                SNode *target = ss_search(head, keys[k%n]);
                if (target) { int kv=target->key; head = ss_delete(head, target); head = ss_insert(head, kv); }
            }
            results_us[OP_DELETE][3][s] = (now_us()-t0)/(2.0*TRIALS);

            while (head) { SNode *nx=head->next; free(head); head=nx; }
        }

        /* ---- Doubly linked unsorted ---- */
        {
            DNode *head=NULL;
            for (int i=0;i<n;i++) head = du_insert(head, keys[i]);
            double t0=now_us();
            for (int k=0;k<TRIALS;k++) du_search(head, keys[rand()%n]);
            results_us[OP_SEARCH][4][s] = (now_us()-t0)/TRIALS;

            t0=now_us();
            for (int k=0;k<TRIALS;k++) { head = du_insert(head,-1); head = du_delete(head, head); }
            results_us[OP_INSERT][4][s] = (now_us()-t0)/(2.0*TRIALS);

            t0=now_us();
            for (int k=0;k<TRIALS;k++) {
                DNode *target = du_search(head, keys[k%n]);
                if (target) { int kv=target->key; head = du_delete(head, target); head = du_insert(head, kv); }
            }
            results_us[OP_DELETE][4][s] = (now_us()-t0)/(2.0*TRIALS);

            while (head) { DNode *nx=head->next; free(head); head=nx; }
        }

        /* ---- Doubly linked sorted ---- */
        {
            DList L = {NULL,NULL};
            for (int i=0;i<n;i++) ds_insert(&L, keys[i]);
            double t0=now_us();
            for (int k=0;k<TRIALS;k++) ds_search(&L, keys[rand()%n]);
            results_us[OP_SEARCH][5][s] = (now_us()-t0)/TRIALS;

            t0=now_us();
            for (int k=0;k<TRIALS;k++) {
                ds_insert(&L,-1);
                DNode *target = ds_search(&L,-1);
                ds_delete(&L, target);
            }
            results_us[OP_INSERT][5][s] = (now_us()-t0)/(2.0*TRIALS);

            t0=now_us();
            for (int k=0;k<TRIALS;k++) {
                DNode *target = ds_search(&L, keys[k%n]);
                if (target) { int kv=target->key; ds_delete(&L, target); ds_insert(&L, kv); }
            }
            results_us[OP_DELETE][5][s] = (now_us()-t0)/(2.0*TRIALS);

            while (L.head) { DNode *nx=L.head->next; free(L.head); L.head=nx; }
        }

        free(keys);
        printf("n=%d done\n", n);
    }
}

static void print_csv(void) {
    const char *opnames[3] = {"Search","Insert","Delete"};
    for (int op=0; op<3; op++) {
        printf("\n== %s timing (microseconds/op) ==\n", opnames[op]);
        printf("n");
        for (int s2=0;s2<NUM_STRUCTS;s2++) printf(",%s", STRUCT_NAMES[s2]);
        printf("\n");
        for (int sz=0; sz<NUM_SIZES; sz++) {
            printf("%d", SIZES[sz]);
            for (int s2=0;s2<NUM_STRUCTS;s2++) printf(",%.3f", results_us[op][s2][sz]);
            printf("\n");
        }
    }
}

#ifdef USE_SDL
#include <SDL3/SDL.h>

static const SDL_Color COLORS[NUM_STRUCTS] = {
    {230,60,60,255},{60,180,80,255},{60,120,230,255},
    {230,170,30,255},{170,60,230,255},{40,200,200,255}
};

static void draw_plot(SDL_Renderer *r, OpType op) {
    SDL_SetRenderDrawColor(r, 20,20,25,255);
    SDL_RenderClear(r);

    int W=900, H=600, margin=60;
    SDL_SetRenderDrawColor(r, 200,200,200,255);
    SDL_RenderLine(r, margin, H-margin, W-20, H-margin); /* x axis */
    SDL_RenderLine(r, margin, H-margin, margin, 20);      /* y axis */

    double maxv = 1.0;
    for (int s=0;s<NUM_STRUCTS;s++) for (int i=0;i<NUM_SIZES;i++)
        if (results_us[op][s][i] > maxv) maxv = results_us[op][s][i];

    for (int s=0;s<NUM_STRUCTS;s++) {
        SDL_SetRenderDrawColor(r, COLORS[s].r, COLORS[s].g, COLORS[s].b, 255);
        for (int i=0;i<NUM_SIZES-1;i++) {
            float x1 = margin + (W-margin-40) * (float)i/(NUM_SIZES-1);
            float x2 = margin + (W-margin-40) * (float)(i+1)/(NUM_SIZES-1);
            float y1 = (H-margin) - (H-margin-40) * (float)(results_us[op][s][i]/maxv);
            float y2 = (H-margin) - (H-margin-40) * (float)(results_us[op][s][i+1]/maxv);
            SDL_RenderLine(r, x1,y1,x2,y2);
            SDL_FRect dot = {x1-2,y1-2,4,4};
            SDL_RenderFillRect(r, &dot);
        }
    }
    SDL_RenderPresent(r);
}

static void run_sdl(void) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *win; SDL_Renderer *ren;
    SDL_CreateWindowAndRenderer("Q1: Dictionary op growth (1=Search 2=Insert 3=Delete)", 900,600, 0, &win,&ren);
    OpType op = OP_SEARCH;
    int running=1;
    draw_plot(ren, op);
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type==SDL_EVENT_QUIT) running=0;
            if (e.type==SDL_EVENT_KEY_DOWN) {
                if (e.key.key==SDLK_1) op=OP_SEARCH;
                if (e.key.key==SDLK_2) op=OP_INSERT;
                if (e.key.key==SDLK_3) op=OP_DELETE;
                if (e.key.key==SDLK_ESCAPE) running=0;
                draw_plot(ren, op);
            }
        }
        SDL_Delay(16);
    }
    SDL_DestroyRenderer(ren); SDL_DestroyWindow(win); SDL_Quit();
}
#endif

int main(void) {
    time_all();
    print_csv();
    printf("\nLegend (colour order used in the plot): ");
    for (int s=0;s<NUM_STRUCTS;s++) printf("%d=%s  ", s+1, STRUCT_NAMES[s]);
    printf("\n");

#ifdef USE_SDL
    run_sdl();
#else
    printf("\n(Compile with -DUSE_SDL and link SDL3 to see the live graph;\n"
           " for now the raw numbers above are what would be plotted.)\n");
#endif
    return 0;
}
