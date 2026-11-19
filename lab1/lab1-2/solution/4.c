/*
 * 实验一 · 任务 2：信号量休息厅（参考答案 · 完整单文件版）
 *
 * 与 student/impl.c 等价，只是把 main 也写了进来，便于阅读整体流程。
 * 判分用的是 solution/impl.c + tests/harness.c，不是这个文件。
 *
 * 编译运行：
 *   make demo && ./build/4_solution
 */

#include <pthread.h>
#include <semaphore.h>
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

/* 1. 声明一个信号量：值 = 当前空椅子的数量 */
sem_t chairs; //TODO1的答案

void *customer(void *arg)
{
    long id = (long)(intptr_t)arg;

    /* 2. 申请椅子：没有空位就在这里阻塞 */
    sem_wait(&chairs); // TODO2的答案

    printf("customer %2ld sits down\n", id);
    fflush(stdout);
    sleep_ms(REST_MS);
    printf("customer %2ld leaves\n", id);
    fflush(stdout);

    /* 3. 让出椅子：唤醒一位等待者 */
    sem_post(&chairs); // TODO3的答案

    return NULL;
}

int main(void)
{
    pthread_t th[CUSTOMERS];

    /* 4. 初始化信号量，初值为椅子数 */
    sem_init(&chairs, 0, CHAIRS); //TODO4的答案

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

    /* 5. 销毁信号量 */
    sem_destroy(&chairs);

    printf("all customers served\n");
    return 0;
}
