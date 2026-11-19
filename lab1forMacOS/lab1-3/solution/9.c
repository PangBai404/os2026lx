#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

#if defined(_WIN32)
#include <windows.h>
static void sleep_ms(int ms) { Sleep((DWORD)ms); }
#else
#include <unistd.h>
static void sleep_ms(int ms) { usleep((useconds_t)ms * 1000); }
#endif

#define NPHIL  5
#define MEALS  5
#define EAT_MS 15

static pthread_mutex_t mon = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  free_cv[NPHIL];
static int holder[NPHIL];
static int eaten = 0;

static void fork_acquire(int phid, int fork)
{
    pthread_mutex_lock(&mon);
    while (holder[fork] != -1) {
        pthread_cond_wait(&free_cv[fork], &mon);
    }
    holder[fork] = phid;
    pthread_mutex_unlock(&mon);
}

static void fork_release(int phid, int fork)
{
    (void)phid;
    pthread_mutex_lock(&mon);
    holder[fork] = -1;
    pthread_cond_signal(&free_cv[fork]);
    pthread_mutex_unlock(&mon);
}

static void *philosopher(void *arg)
{
    int id = (int)(intptr_t)arg;
    int left  = id;
    int right = (id + 1) % NPHIL;

    for (int r = 0; r < MEALS; r++) {
        int first  = left;
        int second = right;

        if (first > second) {
            int tmp = first;
            first   = second;
            second  = tmp;
        }

        fork_acquire(id, first);
        fork_acquire(id, second);

        printf("philosopher %d is eating (round %d)\n", id, r + 1);
        fflush(stdout);
        sleep_ms(EAT_MS);

        pthread_mutex_lock(&mon);
        eaten++;
        pthread_mutex_unlock(&mon);

        fork_release(id, second);
        fork_release(id, first);
    }

    return NULL;
}

int main(void)
{
    pthread_t th[NPHIL];

    for (int i = 0; i < NPHIL; i++) {
        holder[i] = -1;
        pthread_cond_init(&free_cv[i], NULL);
    }

    for (int i = 0; i < NPHIL; i++) {
        int rc = pthread_create(&th[i], NULL, philosopher, (void *)(intptr_t)i);
        if (rc != 0) {
            fprintf(stderr, "pthread_create failed: %d\n", rc);
            return 1;
        }
    }
    for (int i = 0; i < NPHIL; i++) {
        pthread_join(th[i], NULL);
    }

    printf("all philosophers finished: %d meals in total\n", eaten);
    return 0;
}
