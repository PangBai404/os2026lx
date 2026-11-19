/*
 * 测例骨架：本文件自带 main，负责一切输出与校验。
 *
 * 学生只实现 student/impl.c 里的几个函数，碰不到这里的任何一行；
 * 因此「把期望结果直接 printf 出来」这种作弊手法会失效。
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

#define TOTAL    200
#define NTHREADS 3

typedef struct {
    int       id;      /* 1..NTHREADS */
    int       start;   /* 本段起点（含） */
    int       end;     /* 本段终点（含） */
    long      sum;     /* 学生写入 */
    pthread_t tid;     /* 由学生的 run_tasks 填写 */
} task_t;

/* ---------------- 学生实现的部分 ---------------- */
void *p1(void *arg);
void *p2(void *arg);
void *p3(void *arg);

/* 创建 NTHREADS 个线程并全部 join；成功返回 0，失败返回非 0。
 * 线程的 tid 必须写回 tasks[i].tid。 */
int run_tasks(task_t *tasks, int n);

/* ---------------- 以下由测例负责 ---------------- */

/* pthread_t 在不同平台上可能是整数或结构体，哈希后便于打印和比较 */
static unsigned long long tid_hash(pthread_t t)
{
    unsigned char buf[sizeof(pthread_t)];
    unsigned long long h = 1469598103934665603ULL;

    memcpy(buf, &t, sizeof(t));
    for (size_t i = 0; i < sizeof(t); i++) {
        h ^= buf[i];
        h *= 1099511628211ULL;
    }
    return h;
}

int main(void)
{
    task_t tasks[NTHREADS];
    const int base = TOTAL / NTHREADS;
    const int rem  = TOTAL % NTHREADS;
    int cur = 1;

    /* 本次运行的随机前缀：学生代码无从得知，因此无法伪造输出行 */
    unsigned nonce = (unsigned)time(NULL) ^ (unsigned)(uintptr_t)&tasks ^ (unsigned)clock();

    /* 切分区间：余数分给前 rem 个线程 */
    for (int i = 0; i < NTHREADS; i++) {
        int n = base + (i < rem ? 1 : 0);

        tasks[i].id    = i + 1;
        tasks[i].start = cur;
        tasks[i].end   = cur + n - 1;
        tasks[i].sum   = 0;
        cur += n;
    }

    printf("nonce=%08x\n", nonce);
    fflush(stdout);

    int rc = run_tasks(tasks, NTHREADS);
    if (rc != 0) {
        printf("%08x error=run_tasks_returned_%d\n", nonce, rc);
        return 1;
    }

    long total = 0;
    unsigned long long h[NTHREADS];

    for (int i = 0; i < NTHREADS; i++) {
        h[i] = tid_hash(tasks[i].tid);
        printf("%08x [task] p%d range=%d..%d sum=%ld tid=%016llx\n",
               nonce, tasks[i].id, tasks[i].start, tasks[i].end,
               tasks[i].sum, h[i]);
        total += tasks[i].sum;
    }

    int distinct = 1;
    for (int i = 0; i < NTHREADS; i++) {
        if (h[i] == 0) {
            distinct = 0;              /* tid 没被填写 */
        }
        for (int j = i + 1; j < NTHREADS; j++) {
            if (h[i] == h[j]) {
                distinct = 0;
            }
        }
    }

    printf("%08x tids_distinct=%s\n", nonce, distinct ? "yes" : "no");
    printf("%08x total=%ld\n", nonce, total);
    printf("%08x expect=%d\n", nonce, TOTAL * (TOTAL + 1) / 2);
    printf("%08x result=%s\n", nonce,
           (total == TOTAL * (TOTAL + 1) / 2 && distinct) ? "PASS" : "FAIL");

    return (total == TOTAL * (TOTAL + 1) / 2 && distinct) ? 0 : 1;
}
