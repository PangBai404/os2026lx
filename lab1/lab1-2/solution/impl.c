/*
 * 实验一 · 任务 2：信号量休息厅（参考答案 · 函数版）
 *
 * 与 student/impl.c 结构完全相同，只是三处 TODO 已补完。
 * main 在 tests/harness.c 中，本文件不含 main。
 *
 * 自测：make test
 */

#include <semaphore.h>

static sem_t chairs_sem;

/* TODO 1 的答案：初值 = 椅子数 */
void room_init(int chairs)
{
    sem_init(&chairs_sem, 0, (unsigned)chairs);
}

/* TODO 2 的答案：P 操作，没有空位就阻塞 */
void room_acquire(void)
{
    sem_wait(&chairs_sem);
}

/* TODO 3 的答案：V 操作，唤醒等待者 */
void room_release(void)
{
    sem_post(&chairs_sem);
}
