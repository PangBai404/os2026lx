/*
 * 实验一 · 任务 1：三线程累加（学生提交文件）
 *
 * 这个文件由 tests/harness.c 链接使用：main 在测例那边，你只写下面三个函数。
 *
 * 需要完成：
 *   TODO 1  run_segment —— 求 [t->start, t->end] 的整数和，写回 t->sum
 *   TODO 2  run_tasks   —— 为 tasks[0..n-1] 各创建一个线程
 *   TODO 3  run_tasks   —— 等待全部线程结束
 *
 * 提示：
 *   - 三段任务分别对应 p1、p2、p3，已经写好了，它们都调用 run_segment；
 *   - 每个线程只写自己那份 task 结构，彼此不共享可写变量，因此不需要加锁；
 *   - 创建线程后必须 join，否则主线程结束时子线程会被杀死，结果来不及写回；
 *   - 线程的 tid 必须保存到 tasks[i].tid，测例要靠它判断是否真的用了 3 个线程。
 *
 * 自测：make test-student
 */

#include <pthread.h>

#define TOTAL    200
#define NTHREADS 3

typedef struct {
    int       id; 
    int       start; 
    int       end; 
    long      sum; 
    pthread_t tid; 
} task_t;

/* ============================================================
 * TODO：完成单个线程的计算任务
 *   求[t->start, t->end]的整数和，写入t->sum
 * ============================================================ */
void *run_segment(void *arg)
{
    task_t *t = (task_t *)arg;

    /* TODO: 在这里累加并写回 t->sum */
    /*
        进一步提示：
        声明一个长整数累加值，
        将结构体task_t的start作为循环起始，
        结构体task_t的end作为终止，
        执行累加，最后写回。
    */

    return NULL;
}

/* 题目要求的三段任务，各自入口一个函数 */
void *p1(void *arg) { return run_segment(arg); }
void *p2(void *arg) { return run_segment(arg); }
void *p3(void *arg) { return run_segment(arg); }

/* ============================================================
 * TODO：创建线程
 *   for (i = 0; i < n; i++)
 *       调用 pthread_create(&tasks[i].tid, NULL, entry[i], &tasks[i])
 *   其中 entry[] 依次是 p1、p2、p3
 *   返回非0表示出错（可以让 main 提前返回非 0）
 *
 * TODO 3：等待所有线程结束
 *   for (i = 0; i < n; i++)
 *       调用 pthread_join(tasks[i].tid, NULL)
 * ============================================================ */
int run_tasks(task_t *tasks, int n)
{
    void *(*entry[NTHREADS])(void *) = { p1, p2, p3 };

    (void)entry;
    (void)tasks;
    (void)n;

    //TODO: 创建线程
    //TODO: 等待线程

    return 0;
}
