#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>

#if defined(_WIN32)
#include <windows.h>
static void sleep_ms(int ms) { Sleep((DWORD)ms); }
#else
#include <unistd.h>
static void sleep_ms(int ms) { usleep((useconds_t)ms * 1000); }
#endif

#define PRODUCERS          5
#define CONSUMERS          5
#define ITEMS_PER_PRODUCER 20
#define BUFSIZE            8

static int buf[BUFSIZE];
static int head = 0;
static int tail = 0;

static sem_t empty_sem;
static sem_t full_sem;
static pthread_mutex_t buf_mtx = PTHREAD_MUTEX_INITIALIZER;

static int consumed_total = 0;
static pthread_mutex_t stat_mtx = PTHREAD_MUTEX_INITIALIZER;

static void buf_push(int product)
{
    buf[tail] = product;
    tail = (tail + 1) % BUFSIZE;
}

static int buf_pop(void)
{
    int v = buf[head];
    head = (head + 1) % BUFSIZE;
    return v;
}

static void *producer(void *arg)
{
    int pid = (int)(intptr_t)arg;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int product = pid * 1000 + i;

        sem_wait(&empty_sem);
        pthread_mutex_lock(&buf_mtx);
        buf_push(product);
        pthread_mutex_unlock(&buf_mtx);
        sem_post(&full_sem);

        sleep_ms(1);
    }
    return NULL;
}

static void *consumer(void *arg)
{
    (void)arg;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        sem_wait(&full_sem);
        pthread_mutex_lock(&buf_mtx);
        int v = buf_pop();
        pthread_mutex_unlock(&buf_mtx);
        sem_post(&empty_sem);

        printf("consumer took %d\n", v);
        fflush(stdout);

        pthread_mutex_lock(&stat_mtx);
        consumed_total++;
        pthread_mutex_unlock(&stat_mtx);

        sleep_ms(1);
    }
    return NULL;
}

int main(void)
{
    pthread_t prod[PRODUCERS];
    pthread_t cons[CONSUMERS];

    sem_init(&empty_sem, 0, BUFSIZE);
    sem_init(&full_sem, 0, 0);

    for (intptr_t i = 0; i < PRODUCERS; i++) {
        pthread_create(&prod[i], NULL, producer, (void *)i);
    }
    for (int i = 0; i < CONSUMERS; i++) {
        pthread_create(&cons[i], NULL, consumer, NULL);
    }

    for (int i = 0; i < PRODUCERS; i++) {
        pthread_join(prod[i], NULL);
    }
    for (int i = 0; i < CONSUMERS; i++) {
        pthread_join(cons[i], NULL);
    }

    sem_destroy(&empty_sem);
    sem_destroy(&full_sem);

    printf("done: %d products consumed\n", consumed_total);
    return 0;
}
