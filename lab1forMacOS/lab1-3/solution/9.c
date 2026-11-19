/*
 * 实验一 · 任务 3：哲学家就餐与资源顺序分配（参考答案 · 完整单文件版）
 *
 * 与 student/impl.c 等价，只是把 main 和叉子的实现也写了进来，便于阅读整体流程。
 * 判分用的是 solution/impl.c + tests/harness.c，不是这个文件。
 *
 * 与起点唯一的区别：申请两把叉子之前，先把编号小的那把放到前面。
 * 这一处交换打破了「循环等待」，死锁就不再可能发生。
 *
 * 编译运行：
 *   make demo && ./build/9_solution
 */

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

/* ---------------- 叉子的实现 ---------------- */

static pthread_mutex_t mon = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  free_cv[NPHIL];
static int holder[NPHIL];      /* -1 = 空闲 */
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

/* ---------------- 哲学家 ---------------- */

static void *philosopher(void *arg)
{
    int id = (int)(intptr_t)arg;
    int left  = id;
    int right = (id + 1) % NPHIL;

    for (int r = 0; r < MEALS; r++) {
        int first  = left;
        int second = right;

        /* 1. 资源顺序分配：永远先申请编号小的那把 */
        if (first > second) {
            int tmp = first;
            first   = second;
            second  = tmp;
        }

        /* 2. 申请两把叉子 */
        fork_acquire(id, first);
        fork_acquire(id, second);

        printf("philosopher %d is eating (round %d)\n", id, r + 1);
        fflush(stdout);
        sleep_ms(EAT_MS);

        pthread_mutex_lock(&mon);
        eaten++;
        pthread_mutex_unlock(&mon);

        /* 3. 按相反顺序释放 */
        fork_release(id, second);
        fork_release(id, first);
    }

    return NULL;
}

/* ---------------- 主程序 ---------------- */

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
