/*
 * 测例骨架：本文件自带 main，负责创建顾客、计时与校验。
 *
 * 学生只实现 student/impl.c 里的三个函数，碰不到这里的任何一行；
 * 顾客的进出、同时在座人数、总服务数都由测例统计，学生代码无法伪造。
 *
 * 编译方式见 Makefile：
 *   make test          用 solution/impl.c 链接本文件
 *   make test-student  用 student/impl.c 链接本文件
 */

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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

/* ---------------- 学生实现的部分 ---------------- */

/* 初始化休息厅：chairs 把椅子（= 同时可容纳的人数） */
void room_init(int chairs);

/* 找一把空椅子坐下；没有空椅子就等待 */
void room_acquire(void);

/* 让出椅子并离开 */
void room_release(void);

/* ---------------- 以下由测例负责 ---------------- */

static pthread_mutex_t monitor_lock = PTHREAD_MUTEX_INITIALIZER;
static int seats_in_use = 0;   /* 当前在座人数 */
static int peak_seats = 0;     /* 同时在座的最大人数 */
static int violations = 0;     /* 超出椅子数的次数 */
static int served = 0;         /* 完成休息并离开的人数 */

static void monitor_enter(void)
{
    pthread_mutex_lock(&monitor_lock);
    seats_in_use++;
    if (seats_in_use > peak_seats) {
        peak_seats = seats_in_use;
    }
    if (seats_in_use > CHAIRS) {
        violations++;
    }
    pthread_mutex_unlock(&monitor_lock);
}

static void monitor_leave(void)
{
    pthread_mutex_lock(&monitor_lock);
    seats_in_use--;
    served++;
    pthread_mutex_unlock(&monitor_lock);
}

static void *customer(void *arg)
{
    (void)arg;

    room_acquire();       /* 学生实现：取椅子 */
    monitor_enter();      /* 测例记录：坐下 */
    sleep_ms(REST_MS);    /* 休息 */
    monitor_leave();      /* 测例记录：离开 */
    room_release();       /* 学生实现：还椅子 */

    return NULL;
}

int main(void)
{
    pthread_t th[CUSTOMERS];

    /* 本次运行的随机前缀：学生代码无从得知，因此无法伪造输出行 */
    unsigned nonce = (unsigned)time(NULL) ^ (unsigned)(uintptr_t)th ^ (unsigned)clock();

    printf("nonce=%08x\n", nonce);
    printf("%08x customers=%d chairs=%d rest_ms=%d\n",
           nonce, CUSTOMERS, CHAIRS, REST_MS);
    fflush(stdout);

    room_init(CHAIRS);

    clock_t t0 = clock();
    for (int i = 0; i < CUSTOMERS; i++) {
        int rc = pthread_create(&th[i], NULL, customer, NULL);
        if (rc != 0) {
            printf("%08x error=pthread_create_failed_%d\n", nonce, rc);
            return 1;
        }
    }
    for (int i = 0; i < CUSTOMERS; i++) {
        pthread_join(th[i], NULL);
    }
    clock_t t1 = clock();
    long elapsed_ms = (long)((t1 - t0) * 1000 / CLOCKS_PER_SEC);

    printf("%08x peak_seats=%d\n", nonce, peak_seats);
    printf("%08x violations=%d\n", nonce, violations);
    printf("%08x served=%d\n", nonce, served);
    printf("%08x elapsed_ms=%ld\n", nonce, elapsed_ms);

    int pass = (violations == 0) && (served == CUSTOMERS) && (peak_seats == CHAIRS);
    printf("%08x result=%s\n", nonce, pass ? "PASS" : "FAIL");

    return pass ? 0 : 1;
}
