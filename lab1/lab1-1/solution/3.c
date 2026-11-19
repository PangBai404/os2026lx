/*
 * 实验一 · 任务 1：三线程累加（参考答案）
 *
 * 目标：用 3 个线程分别完成 p1、p2、p3 的计算任务，
 *       把 1 到 200 的累加和算出来，总和与原程序保持一致。
 *
 * 关键点：
 *   1. 每个线程只写自己那份 task 结构，彼此不共享可写变量 —— 没有数据竞争。
 *   2. 主线程必须 join，不能提前退出。
 *   3. 段落的划分由 main 负责，线程函数本身不关心 1..200 怎么切。
 *
 * 编译：gcc -Wall -Wextra -O2 -pthread solution/3.c -o build/3_solution
 */

#include <stdio.h>
#include <pthread.h>

#define TOTAL    200
#define NTHREADS 3

typedef struct {
    int       id;      /* 线程序号，1..NTHREADS */
    int       start;   /* 本段起点（含） */
    int       end;     /* 本段终点（含） */
    long      sum;     /* 本段结果，只由本线程写入 */
    pthread_t tid;     /* pthread_create 填充，主线程用于 join */
} task_t;

/* 三个线程共用的计算体：求 [start, end] 的整数和 */
static void *run_segment(void *arg)
{
    task_t *t = (task_t *)arg;
    // TODO1的答案
    long s = 0;

    for (int i = t->start; i <= t->end; i++) {
        s += i;
    }
    t->sum = s;
    return NULL;
}

/* 题目要求的三段任务，各自入口一个函数 */
static void *p1(void *arg) { return run_segment(arg); }
static void *p2(void *arg) { return run_segment(arg); }
static void *p3(void *arg) { return run_segment(arg); }

int main(void)
{
    task_t tasks[NTHREADS];
    void *(*entry[NTHREADS])(void *) = { p1, p2, p3 };
    const int base = TOTAL / NTHREADS;
    const int rem  = TOTAL % NTHREADS;
    int cur = 1;

    /* 1. 切分区间：余数分给前 rem 个线程 */
    for (int i = 0; i < NTHREADS; i++) {
        int n = base + (i < rem ? 1 : 0);

        tasks[i].id    = i + 1;
        tasks[i].start = cur;
        tasks[i].end   = cur + n - 1;
        tasks[i].sum   = 0;
        cur += n;
    }

    /* 2. 创建线程 */
    for (int i = 0; i < NTHREADS; i++) {
        // TODO2 的答案
        int rc = pthread_create(&tasks[i].tid, NULL, entry[i], &tasks[i]);
        if (rc != 0) {
            fprintf(stderr, "pthread_create failed: thread %d, rc=%d\n", i + 1, rc);
            return 1;
        }
    }

    /* 3. 等待全部结束，否则主线程退出会杀掉子线程 */
    for (int i = 0; i < NTHREADS; i++) {
        // TODO3 的答案
        pthread_join(tasks[i].tid, NULL);
    }

    /* 4. 汇总并输出（格式固定，便于测例解析） */
    long total = 0;
    for (int i = 0; i < NTHREADS; i++) {
        printf("[task] p%d range=%d..%d sum=%ld\n",
               tasks[i].id, tasks[i].start, tasks[i].end, tasks[i].sum);
        total += tasks[i].sum;
    }

    /* 5. 验证确实跑了 3 个互不相同的线程 */
    int distinct = 1;
    for (int i = 0; i < NTHREADS; i++) {
        for (int j = i + 1; j < NTHREADS; j++) {
            if (pthread_equal(tasks[i].tid, tasks[j].tid)) {
                distinct = 0;
            }
        }
    }
    printf("distinct_threads=%s\n", distinct ? "yes" : "no");
    printf("total=%ld\n", total);
    printf("expect=%d\n", TOTAL * (TOTAL + 1) / 2);
    printf("result=%s\n", total == TOTAL * (TOTAL + 1) / 2 ? "PASS" : "FAIL");

    return total == TOTAL * (TOTAL + 1) / 2 ? 0 : 1;
}
