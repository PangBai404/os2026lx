/*
 * 实验一 · 任务 4：生产者与消费者（参考答案 · 完整单文件版）
 *
 * 与 student/impl.c 等价，只是把 main 和环形缓冲也写了进来，便于阅读整体流程。
 * 判分用的是 solution/impl.c + tests/harness.c，不是这个文件。
 *
 * 和 README 里那个 1 对 1 的最小模型相比，这里有三处变化：
 *   1. 缓冲从 1 个位置扩展到 8 个（环形缓冲，用 head/tail 两个下标）
 *   2. 生产者、消费者各 5 个
 *   3. 多线程同时读写缓冲区，必须用互斥锁保护 head/tail 和数据
 *
 * 运行后可以看到 100 个产品全部被消费，且每个恰好一次。
 *
 * 编译运行：
 *   make demo && ./build/4_solution
 */

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

/* ---------------- 共享的缓冲池 ---------------- */

static int buf[BUFSIZE];
static int head = 0;
static int tail = 0;

static sem_t empty_sem;                     /* 空位数量，初值 = BUFSIZE */
static sem_t full_sem;                      /* 已放产品数量，初值 = 0 */
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

/* ---------------- 生产者 / 消费者 ---------------- */

static void *producer(void *arg)
{
    int pid = (int)(intptr_t)arg;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int product = pid * 1000 + i;       /* 4 位编号，最高位是生产者编号 */

        sem_wait(&empty_sem);               /* 1. 先等空位 */
        pthread_mutex_lock(&buf_mtx);       /* 2. 再拿锁 */
        buf_push(product);                  /* 3. 写入 */
        pthread_mutex_unlock(&buf_mtx);     /* 4. 放锁 */
        sem_post(&full_sem);                /* 5. 通知消费者 */

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

/* ---------------- 主程序 ---------------- */

int main(void)
{
    pthread_t prod[PRODUCERS];
    pthread_t cons[CONSUMERS];

    sem_init(&empty_sem, 0, BUFSIZE);   /* 一开始有 BUFSIZE 个空位 */
    sem_init(&full_sem, 0, 0);          /* 一开始没有产品 */

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
