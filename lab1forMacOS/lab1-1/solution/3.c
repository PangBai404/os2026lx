#include <stdio.h>
#include <pthread.h>

#define TOTAL    200
#define NTHREADS 3

typedef struct {
    int       id;
    int       start;
    int       end;
    long      sum;
    pthread_t tid;
} task_t;

static void *run_segment(void *arg)
{
    task_t *t = (task_t *)arg;

    long s = 0;

    for (int i = t->start; i <= t->end; i++) {
        s += i;
    }
    t->sum = s;
    return NULL;
}

static void *p1(void *arg) { return run_segment(arg); }
static void *p2(void *arg) { return run_segment(arg); }
static void *p3(void *arg) { return run_segment(arg); }

int main(void)
{
    task_t tasks[NTHREADS];
    void *(*entry[NTHREADS])(void *) = { p1, p2, p3 };
    const int base = TOTAL / NTHREADS;
    const int rem  = TOTAL % NTHREADS;
    int cur = 1;

    for (int i = 0; i < NTHREADS; i++) {
        int n = base + (i < rem ? 1 : 0);

        tasks[i].id    = i + 1;
        tasks[i].start = cur;
        tasks[i].end   = cur + n - 1;
        tasks[i].sum   = 0;
        cur += n;
    }

    for (int i = 0; i < NTHREADS; i++) {
        int rc = pthread_create(&tasks[i].tid, NULL, entry[i], &tasks[i]);
        if (rc != 0) {
            fprintf(stderr, "pthread_create failed: thread %d, rc=%d\n", i + 1, rc);
            return 1;
        }
    }

    for (int i = 0; i < NTHREADS; i++) {
        pthread_join(tasks[i].tid, NULL);
    }

    long total = 0;
    for (int i = 0; i < NTHREADS; i++) {
        printf("[task] p%d range=%d..%d sum=%ld\n",
               tasks[i].id, tasks[i].start, tasks[i].end, tasks[i].sum);
        total += tasks[i].sum;
    }

    int distinct = 1;
    for (int i = 0; i < NTHREADS; i++) {
        for (int j = i + 1; j < NTHREADS; j++) {
            if (pthread_equal(tasks[i].tid, tasks[j].tid)) {
                distinct = 0;
            }
        }
    }
    printf("distinct_threads=%s\n", distinct ? "yes" : "no");
    printf("total=%ld\n", total);
    printf("expect=%d\n", TOTAL * (TOTAL + 1) / 2);
    printf("result=%s\n", total == TOTAL * (TOTAL + 1) / 2 ? "PASS" : "FAIL");

    return total == TOTAL * (TOTAL + 1) / 2 ? 0 : 1;
}
