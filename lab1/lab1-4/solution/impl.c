/*
 * 实验一 · 任务 4：生产者与消费者（参考答案 · 函数版）
 *
 * 与 student/impl.c 结构完全相同，只是三处 TODO 已补完。
 * main 在 tests/harness.c 中，本文件不含 main。
 *
 * 自测：make test
 */

#include <pthread.h>
#include <semaphore.h>

static sem_t empty_sem;
static sem_t full_sem;
static pthread_mutex_t buf_mtx = PTHREAD_MUTEX_INITIALIZER;

//测例提供的接口
void lab_buf_push(int product);
int  lab_buf_pop(void);

//TODO 1 的答案：空位初值为容量，已放产品初值为 0
void pc_init(int bufsize)
{
    sem_init(&empty_sem, 0, (unsigned)bufsize);
    sem_init(&full_sem, 0, 0);
}

//TODO 2 的答案：先等空位，再加锁写入，最后通知非空 
void pc_put(int product)
{
    sem_wait(&empty_sem);

    pthread_mutex_lock(&buf_mtx);
    lab_buf_push(product);
    pthread_mutex_unlock(&buf_mtx);

    sem_post(&full_sem);
}

//TODO 3 的答案：先等非空，再加锁读取，最后通知空位
int pc_get(void)
{
    sem_wait(&full_sem);

    pthread_mutex_lock(&buf_mtx);
    int v = lab_buf_pop();
    pthread_mutex_unlock(&buf_mtx);

    sem_post(&empty_sem);

    return v;
}
