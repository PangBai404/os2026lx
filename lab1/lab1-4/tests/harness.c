/*
 * 测例骨架：本文件自带 main，创建 5 个生产者与 5 个消费者，
 * 并提供环形缓冲的读写原语。
 *
 * 学生只实现 student/impl.c 里的 pc_init / pc_put / pc_get，
 * 碰不到这里的任何一行。缓冲区的容量、每个产品的编号、最终消费到的
 * 产品集合都由测例记录，学生代码无法伪造。
 *
 * 注意：lab_buf_push / lab_buf_pop 是**故意不加锁**的。
 * 数据路径上的互斥必须由调用者（也就是你）保证。
 *
 * 编译方式见 Makefile：
 *   make test          用 solution/impl.c 链接本文件
 *   make test-student  用 student/impl.c 链接本文件
 */

#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

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
#define TOTAL_ITEMS        (PRODUCERS * ITEMS_PER_PRODUCER)
#define BUFSIZE            8

/* ---------------- 学生实现的部分 ---------------- */

/* 初始化：bufsize 是缓冲池容量 */
void pc_init(int bufsize);

/* 放入一个产品；缓冲池已满时应阻塞 */
void pc_put(int product);

/* 取出一个产品；缓冲池为空时应阻塞 */
int pc_get(void);

/* ---------------- 测例提供给学生的接口 ---------------- */

/* 写入缓冲池。调用前必须确认池内有空位，并保证互斥。 */
void lab_buf_push(int product);

/* 从缓冲池读取。调用前必须确认池内非空，并保证互斥。 */
int lab_buf_pop(void);

/* ---------------- 以下由测例负责 ---------------- */

static int buf[BUFSIZE];
static int head = 0;
static int tail = 0;

/* 统计量用原子类型，避免测例自身的计数在竞态时出错 */
static _Atomic int occupancy = 0;
static _Atomic int peak_occupancy = 0;
static _Atomic int overflow = 0;
static _Atomic int underflow = 0;
static _Atomic int produced = 0;
static _Atomic int consumed = 0;

/* 故意插入一点延迟，扩大竞争窗口。
 * 有锁时线程会串行通过，无锁时两个线程就能在这里交错。 */
static void widen_window(void)
{
    volatile unsigned sink = 0;
    for (unsigned i = 0; i < 2000; i++) {
        sink += i;
    }
    (void)sink;
}

void lab_buf_push(int product)
{
    /* 数据路径不加锁：互斥由调用者负责 */
    int t = tail;
    widen_window();
    buf[t] = product;
    tail = (t + 1) % BUFSIZE;
    atomic_fetch_add(&produced, 1);

    int now = atomic_fetch_add(&occupancy, 1) + 1;
    if (now > atomic_load(&peak_occupancy)) {
        atomic_store(&peak_occupancy, now);
    }
    if (now > BUFSIZE) {
        atomic_fetch_add(&overflow, 1);
    }
}

int lab_buf_pop(void)
{
    /* 数据路径不加锁：互斥由调用者负责 */
    int h = head;
    widen_window();
    int v = buf[h];
    head = (h + 1) % BUFSIZE;
    atomic_fetch_add(&consumed, 1);

    int now = atomic_fetch_sub(&occupancy, 1) - 1;
    if (now < 0) {
        atomic_fetch_add(&underflow, 1);
    }
    return v;
}

/* ---------------- 线程函数 ---------------- */

static void *producer_thread(void *arg)
{
    int pid = (int)(intptr_t)arg;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        /* 产品编号：4 位，最高位是本生产者的编号 */
        pc_put(pid * 1000 + i);
    }
    return NULL;
}

typedef struct {
    int items[ITEMS_PER_PRODUCER];
} consumer_ctx;

static void *consumer_thread(void *arg)
{
    consumer_ctx *ctx = (consumer_ctx *)arg;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        ctx->items[i] = pc_get();
        sleep_ms(1);
    }
    return NULL;
}

int main(void)
{
    pthread_t prod[PRODUCERS];
    pthread_t cons[CONSUMERS];
    consumer_ctx ctx[CONSUMERS];

    /* 本次运行的随机前缀：学生代码无从得知，因此无法伪造输出行 */
    unsigned nonce = (unsigned)time(NULL) ^ (unsigned)(uintptr_t)ctx ^ (unsigned)clock();

    printf("nonce=%08x\n", nonce);
    printf("%08x producers=%d consumers=%d items_per_producer=%d bufsize=%d\n",
           nonce, PRODUCERS, CONSUMERS, ITEMS_PER_PRODUCER, BUFSIZE);
    fflush(stdout);

    pc_init(BUFSIZE);

    for (intptr_t i = 0; i < PRODUCERS; i++) {
        int rc = pthread_create(&prod[i], NULL, producer_thread, (void *)i);
        if (rc != 0) {
            printf("%08x error=pthread_create_failed_%d\n", nonce, rc);
            return 1;
        }
    }
    for (int i = 0; i < CONSUMERS; i++) {
        int rc = pthread_create(&cons[i], NULL, consumer_thread, &ctx[i]);
        if (rc != 0) {
            printf("%08x error=pthread_create_failed_%d\n", nonce, rc);
            return 1;
        }
    }

    for (int i = 0; i < PRODUCERS; i++) {
        pthread_join(prod[i], NULL);
    }
    for (int i = 0; i < CONSUMERS; i++) {
        pthread_join(cons[i], NULL);
    }

    /* ---- 汇总：每个产品必须恰好被消费一次 ---- */
    int seen[PRODUCERS][ITEMS_PER_PRODUCER];
    for (int p = 0; p < PRODUCERS; p++) {
        for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
            seen[p][i] = 0;
        }
    }

    int duplicates = 0;
    int bad_id = 0;
    int unexpected = 0;

    for (int c = 0; c < CONSUMERS; c++) {
        for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
            int v = ctx[c].items[i];
            int p = v / 1000;
            int seq = v % 1000;

            if (p < 0 || p >= PRODUCERS || seq < 0 || seq >= ITEMS_PER_PRODUCER) {
                unexpected++;
                bad_id++;
                continue;
            }
            seen[p][seq]++;
        }
    }

    int missing = 0;
    for (int p = 0; p < PRODUCERS; p++) {
        for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
            if (seen[p][i] == 0) {
                missing++;
            } else if (seen[p][i] > 1) {
                duplicates += seen[p][i] - 1;
            }
        }
    }

    int leftover = atomic_load(&occupancy);

    printf("%08x produced=%d\n", nonce, atomic_load(&produced));
    printf("%08x consumed=%d\n", nonce, atomic_load(&consumed));
    printf("%08x peak_occupancy=%d\n", nonce, atomic_load(&peak_occupancy));
    printf("%08x overflow=%d\n", nonce, atomic_load(&overflow));
    printf("%08x underflow=%d\n", nonce, atomic_load(&underflow));
    printf("%08x duplicates=%d\n", nonce, duplicates);
    printf("%08x missing=%d\n", nonce, missing);
    printf("%08x unexpected=%d\n", nonce, unexpected);
    printf("%08x leftover=%d\n", nonce, leftover);

    int pass = (atomic_load(&produced) == TOTAL_ITEMS) &&
               (atomic_load(&consumed) == TOTAL_ITEMS) &&
               (atomic_load(&overflow) == 0) &&
               (atomic_load(&underflow) == 0) &&
               (duplicates == 0) &&
               (missing == 0) &&
               (unexpected == 0) &&
               (leftover == 0);

    printf("%08x result=%s\n", nonce, pass ? "PASS" : "FAIL");

    return pass ? 0 : 1;
}
