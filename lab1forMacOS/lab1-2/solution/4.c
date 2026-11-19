#include <pthread.h>
#include <semaphore.h>
#include "sem_compat.h"
#include <stdio.h>

#if defined(_WIN32)
#include <windows.h>
static void sleep_ms(int ms) { Sleep((DWORD)ms); }
#else
#include <unistd.h>
static void sleep_ms(int ms) { usleep((useconds_t)ms * 1000); }
#endif

#define CUSTOMERS 10
#define CHAIRS    2
#define REST_MS   150

sem_t chairs;

void *customer(void *arg)
{
    long id = (long)(intptr_t)arg;

    sem_wait(&chairs);

    printf("customer %2ld sits down\n", id);
    fflush(stdout);
    sleep_ms(REST_MS);
    printf("customer %2ld leaves\n", id);
    fflush(stdout);

    sem_post(&chairs);

    return NULL;
}

int main(void)
{
    pthread_t th[CUSTOMERS];

    sem_init(&chairs, 0, CHAIRS);

    for (long i = 0; i < CUSTOMERS; i++) {
        int rc = pthread_create(&th[i], NULL, customer, (void *)(intptr_t)i);
        if (rc != 0) {
            fprintf(stderr, "pthread_create failed: %d\n", rc);
            return 1;
        }
    }
    for (int i = 0; i < CUSTOMERS; i++) {
        pthread_join(th[i], NULL);
    }

    sem_destroy(&chairs);

    printf("all customers served\n");
    return 0;
}
