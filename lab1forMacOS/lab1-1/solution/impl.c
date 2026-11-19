/*
 * 实验一 · 任务 1：三线程累加（参考答案 · 函数版）
 *
 * 与 student/impl.c 结构完全相同，只是三处 TODO 已补完。
 * main 在 tests/harness.c 中，本文件不含 main。
 *
 * 自测：make test
 */

#include <pthread.h>
#include <stdio.h>

#define TOTAL    200
#define NTHREADS 3

typedef struct {
    int       id;
    int       start;
    int       end;
    long      sum;
    pthread_t tid;
} task_t;

//TODO 1 的答案：求 [start, end] 的整数和
void *run_segment(void *arg)
{
    task_t *t = (task_t *)arg;
    long s = 0;

    for (int i = t->start; i <= t->end; i++) {
        s += i;
    }
    t->sum = s;
    return NULL;
}

void *p1(void *arg) { return run_segment(arg); }
void *p2(void *arg) { return run_segment(arg); }
void *p3(void *arg) { return run_segment(arg); }

int run_tasks(task_t *tasks, int n)
{
    void *(*entry[NTHREADS])(void *) = { p1, p2, p3 };

    //TODO 2 的答案：创建线程
    for (int i = 0; i < n; i++) {
        int rc = pthread_create(&tasks[i].tid, NULL, entry[i], &tasks[i]);
        if (rc != 0) {
            fprintf(stderr, "pthread_create failed: thread %d, rc=%d\n", i + 1, rc);
            return rc;
        }
    }

    //TODO 3 的答案：等待全部结束
    for (int i = 0; i < n; i++) {
        pthread_join(tasks[i].tid, NULL);
    }

    return 0;
}
