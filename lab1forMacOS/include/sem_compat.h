#ifndef LAB_SEM_COMPAT_H
#define LAB_SEM_COMPAT_H

/*
 * macOS 信号量兼容层
 *
 * macOS 的 sem_init() 会返回 -1 / ENOSYS，无法初始化未命名信号量；
 * sem_open() 是可用的。本头文件在 macOS 上把
 *
 *     sem_init / sem_wait / sem_post / sem_destroy
 *
 * 映射到基于 sem_open 的实现，学生代码仍然按 POSIX 的这组 API 写，
 * TODO 和判分逻辑都不用改。
 *
 * Linux / Windows（MinGW-w64）上本头文件不做任何替换，
 * 直接使用系统自带的 POSIX 信号量实现。
 */

#include <semaphore.h>

#if defined(__APPLE__)

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define LAB_SEM_MAX 8

typedef struct {
    sem_t   *key;   /* 学生代码里的 sem_t 对象地址 */
    sem_t   *os;    /* sem_open 返回的真实信号量 */
} lab_sem_slot;

static lab_sem_slot lab_sem_slots[LAB_SEM_MAX];
static unsigned    lab_sem_seq;

static inline int lab_sem_find(sem_t *sem)
{
    for (int i = 0; i < LAB_SEM_MAX; i++) {
        if (lab_sem_slots[i].key == sem) return i;
    }
    return -1;
}

static inline int lab_sem_init(sem_t *sem, int pshared, unsigned value)
{
    (void)pshared;  /* 本实验只在同一进程的线程之间共享 */

    if (lab_sem_find(sem) >= 0) {
        errno = EINVAL;
        return -1;
    }

    int slot = -1;
    for (int i = 0; i < LAB_SEM_MAX; i++) {
        if (!lab_sem_slots[i].key) {
            slot = i;
            break;
        }
    }
    if (slot < 0) {
        errno = ENOMEM;
        return -1;
    }

    char name[64];
    snprintf(name, sizeof name, "/oslab_sem_%d_%u",
             (int)getpid(), lab_sem_seq++);
    sem_unlink(name);  /* 清理同名的残留对象 */

    sem_t *os = sem_open(name, O_CREAT | O_EXCL, 0600, value);
    if (os == SEM_FAILED) return -1;
    sem_unlink(name);  /* 名字可以立刻移除，信号量在 close 前仍然有效 */

    lab_sem_slots[slot].key = sem;
    lab_sem_slots[slot].os  = os;
    return 0;
}

static inline int lab_sem_wait(sem_t *sem)
{
    int slot = lab_sem_find(sem);
    if (slot < 0) {
        errno = EINVAL;
        return -1;
    }
    return sem_wait(lab_sem_slots[slot].os);
}

static inline int lab_sem_post(sem_t *sem)
{
    int slot = lab_sem_find(sem);
    if (slot < 0) {
        errno = EINVAL;
        return -1;
    }
    return sem_post(lab_sem_slots[slot].os);
}

static inline int lab_sem_destroy(sem_t *sem)
{
    int slot = lab_sem_find(sem);
    if (slot < 0) {
        errno = EINVAL;
        return -1;
    }

    int rc = sem_close(lab_sem_slots[slot].os);
    lab_sem_slots[slot].key = NULL;
    lab_sem_slots[slot].os  = NULL;
    return rc;
}

#define sem_init(sem, pshared, value) lab_sem_init((sem), (pshared), (value))
#define sem_wait(sem)                 lab_sem_wait((sem))
#define sem_post(sem)                 lab_sem_post((sem))
#define sem_destroy(sem)              lab_sem_destroy((sem))

#endif /* __APPLE__ */

#endif /* LAB_SEM_COMPAT_H */
