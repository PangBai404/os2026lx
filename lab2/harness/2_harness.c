/*=====================================================================
 * 实验二 内存管理 · 测例骨架（自带 main，学生不可修改）
 *
 * 用法:
 *   ./2_harness [19字节数据]
 *   缺省数据 0123456789abcWXYZde(与教学版一致)；
 *   判分时 check.py 每轮传入随机数据，杜绝硬编码答案。
 *
 * 学号来源: student/impl.c 顶部的 SID4（extern 读取）。
 *   SID4 不是 4 位数字时程序判 FAIL reason=sid，提示学生填写。
 *
 * 数据流(与教学版 mem.c 对应):
 *   harness 写 f1 → 学生 setup(mmap) → 学生 backup(malloc+memmove)
 *   → fork → 子进程: 学生 child_work(memchr/替换SID4/写回src)
 *   → 父进程: wait → 学生 parent_work(memset QQ) → 学生 readback_file
 *   → 学生 cleanup(munmap/close/free) → harness 重读 f1 → 自检 result
 *====================================================================*/
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include "impl.h"

#define DATALEN 19

static unsigned g_nonce;

/* 每行输出都带本次运行随机 nonce 前缀: 学生代码拿不到 nonce, 无法伪造 */
static void hprintf(const char *fmt, ...)
{
    va_list ap;
    printf("%08x ", g_nonce);
    va_start(ap, fmt); vprintf(fmt, ap); va_end(ap);
    putchar('\n'); fflush(stdout);
}

/* 把不可打印字节替换为 '?', 防止脏内存破坏输出行结构 */
static void sanitize(char *b, int n)
{
    for (int i = 0; i < n; i++) {
        unsigned char c = (unsigned char)b[i];
        if (c < 32 || c > 126) b[i] = '?';
    }
}

/* 安全快照: 从进程内存取 DATALEN 字节并清洗(不依赖 NUL) */
static void snap(char out[], const char *p)
{
    if (!p) { strcpy(out, "(null)"); return; }
    memcpy(out, p, DATALEN);
    out[DATALEN] = '\0';
    sanitize(out, DATALEN);
}

/* 期望串1: data 中 WXYZ -> SID4;  期望串2: 前两字节 -> 'Q' */
static void build_expect(char e1[], char e2[],
                         const char *data, int wpos, const char *sid4)
{
    memcpy(e1, data, DATALEN); e1[DATALEN] = '\0';
    memcpy(e1 + wpos, sid4, 4);
    memcpy(e2, e1, DATALEN + 1);
    e2[0] = e2[1] = 'Q';
}

static int sid_ok(const char *s)
{
    return strlen(s) == 4 && strspn(s, "0123456789") == 4;
}

int main(int argc, char **argv)
{
    /* 学号后四位来自学生 impl.c 顶部的 SID4 —— 判分前请先填写! */
    const char *sid4 = SID4;
    const char *argd = (argc > 1) ? argv[1] : NULL;
    char data[DATALEN + 1];

    static int anchor;
    g_nonce = (unsigned)time(NULL) ^ (unsigned)clock()
            ^ (unsigned)(uintptr_t)&anchor ^ (unsigned)getpid();

    printf("nonce=%08x\n", g_nonce); fflush(stdout);

    if (argd) {
        if ((int)strlen(argd) != DATALEN) { fprintf(stderr, "bad data length\n"); return 1; }
        strcpy(data, argd);
    } else {
        strcpy(data, "0123456789abcWXYZde");
    }
    const char *w = strstr(data, "WXYZ");
    if (!w || strstr(w + 1, "WXYZ")) { fprintf(stderr, "data must contain exactly one WXYZ\n"); return 1; }
    int wpos = (int)(w - data);

    hprintf("[data] wpos=%d len=%d content=%s", wpos, DATALEN, data);

    /* 学号检查: 不是 4 位数字 → 判 FAIL 并给出明确提示 */
    hprintf("[sid] value=%s isdigits=%s", sid4, sid_ok(sid4) ? "true" : "false");
    if (!sid_ok(sid4)) {
        hprintf("[final] result=FAIL reason=sid");
        fprintf(stderr,
            "SID4 is not 4 digits. Edit student/impl.c TOP:\n"
            "    const char SID4[5] = \"xxxx\";  <-- put YOUR last-4 student id here\n");
        return 1;
    }

    /* 每次运行自动重建 f1, 内容为本次(随机)data */
    int wfd = open("./f1", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (wfd < 0) { perror("open f1"); return 1; }
    if (write(wfd, data, DATALEN) != DATALEN) { perror("write f1"); close(wfd); return 1; }
    close(wfd);

    char expect1[DATALEN + 1], expect2[DATALEN + 1];
    build_expect(expect1, expect2, data, wpos, sid4);

    ctx_t ctx; memset(&ctx, 0, sizeof ctx);
    char s1[DATALEN + 1], s2[DATALEN + 1];

    /* ---- 学生(1): 页大小/open/mmap ---- */
    int real_ps = (int)getpagesize();
    int rc = setup_shared(&ctx);
    int setup_ok = (rc == 0 && ctx.src && ctx.fd > 0 && ctx.pagesize == real_ps);
    snap(s1, ctx.src);
    hprintf("[setup] rc=%d pagesize=%d expected_pagesize=%d", rc, ctx.pagesize, real_ps);
    hprintf("[initial] src=%s", s1);
    if (!setup_ok) { hprintf("[final] result=FAIL reason=setup"); return 1; }

    /* ---- 学生(3)(4): malloc + memmove 备份 ---- */
    rc = backup_to_heap(&ctx);
    snap(s2, ctx.dst);
    int backup_ok = (rc == 0 && ctx.dst && memcmp(ctx.dst, data, DATALEN) == 0);
    hprintf("[backup] rc=%d dst=%s", rc, s2);
    if (!backup_ok) { hprintf("[final] result=FAIL reason=backup"); return 1; }

    fflush(stdout);
    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }

    if (pid == 0) {                      /* ============ 子进程 ============ */
        long temp_off = -1;
        int crc = child_work(&ctx, &temp_off);         /* 学生(6)(7)(8)(9) */
        snap(s1, ctx.dst); snap(s2, ctx.src);
        hprintf("[proc] role=child pid=%d ppid=%d", (int)getpid(), (int)getppid());
        hprintf("[child] rc=%d temp_off=%ld dst=%s src=%s", crc, temp_off, s1, s2);
        hprintf("[end] role=child src=%s", s2);
        int ok = (crc == 0 && temp_off == wpos &&
                  strcmp(s1, expect1) == 0 && strcmp(s2, expect1) == 0);
        _exit(ok ? 0 : 2);               /* _exit 不刷新 stdio, 防止重复输出 */
    }

    /* ============ 父进程 ============ */
    int status = 0;
    wait(&status);                       /* 同步: 保证先看到子进程输出 */
    int child_rc = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

    parent_work(&ctx);                   /* 学生(12): memset 'QQ' */
    snap(s1, ctx.src); snap(s2, ctx.dst);
    hprintf("[proc] role=parent pid=%d ppid=%d", (int)getpid(), (int)getppid());
    hprintf("[parent] src=%s dst=%s", s1, s2);

    /* ---- 学生(13): lseek+read 显示文件内容 ---- */
    char buf[512]; memset(buf, 0, sizeof buf);
    int n = readback_file(&ctx, buf, (int)sizeof buf);
    if (n < 0) n = 0;
    if (n > (int)sizeof buf - 1) n = (int)sizeof buf - 1;
    sanitize(buf, n);
    hprintf("[file] n=%d content=%.*s", n, n, buf);
    hprintf("[end] role=parent src=%s", s1);

    /* ---- 学生(11)(14): munmap/close/free ---- */
    int crc2 = cleanup(&ctx);
    hprintf("[cleanup] rc=%d", crc2);

    /* harness 独立重读 f1: 假 mmap(不真映射)在这里必露馅 */
    int rfd = open("./f1", O_RDONLY);
    char fb[256]; memset(fb, 0, sizeof fb);
    int fn = (rfd >= 0) ? (int)read(rfd, fb, sizeof fb - 1) : -1;
    if (rfd >= 0) close(rfd);
    if (fn < 0) fn = 0;
    sanitize(fb, fn);
    hprintf("[file_after] n=%d content=%.*s", fn, fn, fb);

    /* harness 自检(check.py 仍会独立复核全部行) */
    int pass = (child_rc == 0) &&
               (strcmp(s1, expect2) == 0) &&   /* 父进程 src: QQ 已叠加       */
               (strcmp(s2, data)    == 0) &&   /* 父进程 dst: 私有内存未被污染 */
               (n >= DATALEN && strncmp(buf, expect2, DATALEN) == 0) &&
               (crc2 == 0) &&
               (fn >= DATALEN && strncmp(fb, expect2, DATALEN) == 0);
    hprintf("[final] result=%s", pass ? "PASS" : "FAIL");
    return pass ? 0 : 1;
}
