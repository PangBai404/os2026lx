/*
 * 测例骨架：本文件自带 main，创建哲学家、提供取叉/放叉接口并统计。
 *
 * 学生只实现 student/impl.c 里的 philosopher_run，碰不到这里的任何一行。
 * 取叉顺序、持叉情况、用餐次数都由测例记录，学生代码无法伪造。
 *
 * 编译方式见 Makefile：
 *   make test          用 solution/impl.c 链接本文件
 *   make test-student  用 student/impl.c 链接本文件
 */

#include <pthread.h>
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

#define NPHIL   5
#define MEALS   5
#define EAT_MS  15

/* ---------------- 学生实现的部分 ---------------- */

/* 哲学家 id 的第 r 轮：取叉、吃饭、放叉。共 rounds 轮。 */
void philosopher_run(int id, int nphil, int rounds);

/* ---------------- 测例提供给学生的接口 ---------------- */

/* 阻塞申请编号为 fork 的叉子；已被别人持有就等待 */
int lab_fork_acquire(int phid, int fork);

/* 释放编号为 fork 的叉子；必须由当前持有者调用 */
void lab_fork_release(int phid, int fork);

/* 开始吃饭：此时必须已经握着两把叉子 */
void lab_meal(int phid);

/* ---------------- 以下由测例负责 ---------------- */

static pthread_mutex_t mon = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  free_cv[NPHIL];

static int holder[NPHIL];       /* -1 = 空闲，否则为持有者 id */
static int held_count[NPHIL];   /* 每位哲学家当前握着的叉子数 */
static int first_fork[NPHIL];   /* 本轮第一次申请的叉子编号 */

static int order_violations   = 0;  /* 先拿了编号大的叉子 */
static int hold_violations    = 0;  /* 没握齐两把叉子就吃饭 */
static int release_violations = 0;  /* 释放了不归自己持有的叉子 */
static int meals_done         = 0;  /* 完成的用餐次数 */

/* 记录第一个顺序违规的例子，便于报错时说明 */
static int bad_phid = -1, bad_first = -1, bad_second = -1;

int lab_fork_acquire(int phid, int fork)
{
    pthread_mutex_lock(&mon);
    while (holder[fork] != -1) {
        pthread_cond_wait(&free_cv[fork], &mon);
    }
    holder[fork] = phid;

    if (held_count[phid] == 0) {
        first_fork[phid] = fork;
    } else if (fork < first_fork[phid]) {
        order_violations++;
        if (bad_phid < 0) {
            bad_phid   = phid;
            bad_first  = first_fork[phid];
            bad_second = fork;
        }
    }
    held_count[phid]++;

    pthread_mutex_unlock(&mon);
    return 0;
}

void lab_fork_release(int phid, int fork)
{
    pthread_mutex_lock(&mon);
    if (holder[fork] != phid) {
        release_violations++;
    } else {
        holder[fork] = -1;
        held_count[phid]--;
        pthread_cond_signal(&free_cv[fork]);
    }
    pthread_mutex_unlock(&mon);
}

void lab_meal(int phid)
{
    pthread_mutex_lock(&mon);
    if (held_count[phid] != 2) {
        hold_violations++;
    }
    meals_done++;
    pthread_mutex_unlock(&mon);

    sleep_ms(EAT_MS);
}

static void *philosopher_thread(void *arg)
{
    int id = (int)(intptr_t)arg;
    philosopher_run(id, NPHIL, MEALS);
    return NULL;
}

int main(void)
{
    pthread_t th[NPHIL];

    /* 本次运行的随机前缀：学生代码无从得知，因此无法伪造输出行 */
    unsigned nonce = (unsigned)time(NULL) ^ (unsigned)(uintptr_t)th ^ (unsigned)clock();

    for (int i = 0; i < NPHIL; i++) {
        holder[i] = -1;
        pthread_cond_init(&free_cv[i], NULL);
    }

    printf("nonce=%08x\n", nonce);
    printf("%08x philosophers=%d meals_each=%d\n", nonce, NPHIL, MEALS);
    fflush(stdout);

    for (long i = 0; i < NPHIL; i++) {
        int rc = pthread_create(&th[i], NULL, philosopher_thread, (void *)(intptr_t)i);
        if (rc != 0) {
            printf("%08x error=pthread_create_failed_%d\n", nonce, rc);
            return 1;
        }
    }
    for (int i = 0; i < NPHIL; i++) {
        pthread_join(th[i], NULL);
    }

    printf("%08x eaten=%d\n", nonce, meals_done);
    printf("%08x expect_eaten=%d\n", nonce, NPHIL * MEALS);
    printf("%08x order_violations=%d\n", nonce, order_violations);
    if (bad_phid >= 0) {
        printf("%08x order_example=phil%d_%d_then_%d\n",
               nonce, bad_phid, bad_first, bad_second);
    } else {
        printf("%08x order_example=none\n", nonce);
    }
    printf("%08x hold_violations=%d\n", nonce, hold_violations);
    printf("%08x release_violations=%d\n", nonce, release_violations);

    int pass = (meals_done == NPHIL * MEALS) &&
               (order_violations == 0) &&
               (hold_violations == 0) &&
               (release_violations == 0);
    printf("%08x result=%s\n", nonce, pass ? "PASS" : "FAIL");

    return pass ? 0 : 1;
}
