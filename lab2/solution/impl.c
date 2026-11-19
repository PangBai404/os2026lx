/* 实验二 内存管理 · 参考实现（make test 应 PASSED） */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "impl.h"

/* 教师演示学号，可随意改（必须是 4 位数字） */
const char SID4[5] = "2345";

/* (1) getpagesize + open + mmap 建共享区 */
int setup_shared(ctx_t *ctx)
{
    ctx->pagesize = getpagesize();
    ctx->fd = open("./f1", O_RDWR);
    if (ctx->fd == -1) { perror("open ./f1"); return -1; }
    ctx->src = (char *)mmap(NULL, ctx->pagesize, PROT_READ | PROT_WRITE,
                            MAP_SHARED, ctx->fd, 0);
    if (ctx->src == MAP_FAILED) { perror("mmap"); ctx->src = NULL; return -1; }
    return 0;
}

/* (3)(4) malloc 一个页面 + memmove 复制共享区内容 */
int backup_to_heap(ctx_t *ctx)
{
    ctx->dst = (char *)malloc(ctx->pagesize);
    if (!ctx->dst) return -1;
    memmove(ctx->dst, ctx->src, ctx->pagesize);
    return 0;
}

/* (6)(7)(8)(9) 子进程: memchr 找 'W' → 替换学号 SID4 → 写回共享区 */
int child_work(ctx_t *ctx, long *temp_off)
{
    char *temp = (char *)memchr(ctx->dst, 'W', ctx->pagesize);
    if (!temp) return -1;
    if (temp_off) *temp_off = (long)(temp - ctx->dst);   /* (7) 位置 */

    memmove(temp, SID4, 4);                              /* (8) 替换 WXYZ */
    memmove(ctx->src, ctx->dst, ctx->pagesize);          /* (9) 写回共享区 */
    return 0;
}

/* (12) 父进程: 共享区前 2 字节替换为 'Q' */
void parent_work(ctx_t *ctx)
{
    memset(ctx->src, 'Q', 2);
}

/* (13) lseek+read 读文件内容 */
int readback_file(ctx_t *ctx, char *buf, int bufsz)
{
    if (lseek(ctx->fd, 0, SEEK_SET) == (off_t)-1) return -1;
    int total = 0;
    ssize_t n = 0;
    while (total < bufsz - 1 &&
           (n = read(ctx->fd, buf + total, bufsz - 1 - total)) > 0)
        total += (int)n;
    if (n < 0) return -1;
    buf[total] = '\0';
    return total;
}

/* (11)+(14) munmap + close + free，指针置 NULL */
int cleanup(ctx_t *ctx)
{
    int rc = 0;
    if (ctx->src && munmap(ctx->src, ctx->pagesize) != 0) rc = -1;
    if (ctx->fd >= 0 && close(ctx->fd) != 0) rc = -1;
    free(ctx->dst); ctx->dst = NULL;
    ctx->src = NULL;
    return rc;
}
