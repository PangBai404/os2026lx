/*=====================================================================
 * 实验二 内存管理 · 学生实现文件
 *
 * 第一步: 在下方 ★ 处填写你的学号后四位(4 位数字)！
 *         不填或填错(非数字)，判分会直接 FAIL 并提示 reason=sid。
 *
 * 你只需补全下面 6 个函数；main 在 harness/2_harness.c 中，不要修改。
 * 每次运行 f1 内容都是随机的(判分模式)，'WXYZ' 位置不固定，
 * 必须用 memchr 真正查找，硬编码位置/内容都无法通过测试！
 *
 * ⚠ 不要在 child_work 里 free(dst)/close(fd)——释放统一放 cleanup()。
 * ⚠ 不要 printf 任何形如 "[xxx] k=v" 的行(那是 harness 专用输出)。
 *====================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "impl.h"

/* ★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★
 * ★  在这里填写你的学号后四位（4 位数字），例如 "5678"  ★
 * ★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★★ */
const char SID4[5] = "xxxx";          /* ← 改成你的学号后四位! */

/* ---- (1) 页大小 / 打开 f1 / mmap 建共享区 ----
 * 提示: ctx->pagesize = getpagesize();
 *       ctx->fd = open("./f1", O_RDWR);
 *       ctx->src = mmap(NULL, ctx->pagesize, PROT_READ|PROT_WRITE,
 *                       MAP_SHARED, ctx->fd, 0);  失败时注意 MAP_FAILED */
int setup_shared(ctx_t *ctx)
{
    (void)ctx;
    return -1;                      /* TODO(1): 补全后删除本行 */
}

/* ---- (3)(4) malloc 一个页面 + memmove 把 src 复制到 dst ---- */
int backup_to_heap(ctx_t *ctx)
{
    (void)ctx;
    return -1;                      /* TODO(3)(4): 补全后删除本行 */
}

/* ---- (6)(7)(8)(9) 子进程工作 ----
 * 提示: temp = memchr(ctx->dst, 'W', ctx->pagesize);
 *       *temp_off = temp - ctx->dst;          (7) 位置
 *       memmove(temp, SID4, 4);               (8) 把 WXYZ 换成你填的学号
 *       memmove(ctx->src, ctx->dst, ctx->pagesize);  (9) 写回共享区 */
int child_work(ctx_t *ctx, long *temp_off)
{
    (void)ctx; (void)temp_off;
    return -1;                      /* TODO(6)~(9): 补全后删除本行 */
}

/* ---- (12) memset 把 src 前 2 字节替换为 'Q' ---- */
void parent_work(ctx_t *ctx)
{
    (void)ctx;                      /* TODO(12): 补全 */
}

/* ---- (13) lseek 到文件开头 + 循环 read 到 buf，返回总字节数 ---- */
int readback_file(ctx_t *ctx, char *buf, int bufsz)
{
    (void)ctx; (void)buf; (void)bufsz;
    return -1;                      /* TODO(13): 补全后删除本行 */
}

/* ---- (11)+(14) munmap + close + free(dst)，指针置 NULL ---- */
int cleanup(ctx_t *ctx)
{
    (void)ctx;
    return -1;                      /* TODO(11)(14): 补全后删除本行 */
}
