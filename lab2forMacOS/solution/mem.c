/*=====================================================================
 * 实验二 内存管理 —— 参考实现 mem.c
 * 使用: 复制到实验包根目录后编译: gcc mem.c -o mem，运行: ./mem
 * 注意: 将代码中的 "2345" 改为你自己的学号后四位!
 *====================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      /* getpagesize, fork, getpid, getppid, close, lseek, read */
#include <fcntl.h>       /* open, O_RDWR */
#include <sys/types.h>   /* pid_t */
#include <sys/wait.h>    /* wait */
#include <sys/mman.h>    /* mmap, munmap, PROT_*, MAP_SHARED, MAP_FAILED */

int main(void)
{
    int   fd, page_size;
    pid_t pid;
    char *dst = NULL, *temp = NULL;

    /* ⑴ 显示当前系统的页面大小 */
    page_size = getpagesize();
    printf("(1) page_size = %d, PID = %d\n\n", page_size, getpid());

    /* 打开当前目录下的文件 f1 */
    fd = open("./f1", O_RDWR);
    if (fd == -1) { perror("open ./f1 failed"); exit(1); }

    /* 创建共享存储区: 大小为一个页面, 将 f1 映射进内存, 地址返回 src */
    char *src = (char *)mmap(NULL, page_size, PROT_READ | PROT_WRITE,
                             MAP_SHARED, fd, 0);
    if (src == MAP_FAILED) { printf("error\n"); exit(1); }

    /* ⑵ 显示 src 的内容 */
    printf("(2) src(共享存储区)的内容: %s\n\n", src);

    /* ⑶ malloc 申请一个页面大小的内存空间, 地址返回 dst */
    dst = (char *)malloc(page_size);
    if (dst == NULL) { printf("malloc error\n"); exit(1); }

    /* ⑷ memmove 将共享存储区的内容复制到 dst */
    memmove(dst, src, page_size);

    /* ⑸ 显示 dst 的内容 */
    printf("(5) dst(刚复制)的内容: %s\n\n", dst);

    pid = fork();                       /* 创建子进程 */

    if (pid == 0) {   /* ---------------- 子进程空间 ---------------- */
        /* ⑹ memchr 搜索 dst 中字符 'W' 的位置, 地址返回 temp */
        temp = (char *)memchr(dst, 'W', page_size);
        if (temp == NULL) { printf("dst 中未找到 'W'!\n"); exit(1); }

        /* ⑺ 显示 temp 的内容及其内存起始地址 */
        printf("(7) temp的内容: %s, temp的内存起始地址: %p\n\n",
               temp, (void *)temp);

        /* ⑻ 将 temp 前 4 个字符("WXYZ")用学号后四位替换(此处以 2345 为例) */
        memmove(temp, "2345", 4);
        printf("(8) 替换学号后 dst 的内容: %s\n\n", dst);

        /* ⑼ memmove 将修改后的 dst 复制回 src */
        memmove(src, dst, page_size);

        /* ⑽ 显示 src 的内容(含 PID 及 PPID) */
        printf("(10) 子进程: PID is %d, PPID is %d, src: %s\n\n",
               getpid(), getppid(), src);

        /* ⑾ 关闭文件 f1, 释放 dst(指针置 NULL 防止野指针) */
        close(fd);
        free(dst);
        dst = NULL;
        printf("(11) 子进程已关闭 f1, 释放 dst\n\n");
    }
    else if (pid > 0) {  /* ------------ 父进程空间 ------------------ */
        wait(NULL);      /* 父进程阻塞自己, 等待子进程结束 */

        /* ⑿ memset 将 src 前 2 个字符用 'Q' 替换, 显示(含 PID 及 PPID) */
        memset(src, 'Q', 2);
        printf("(12) 父进程: PID is %d, PPID is %d, src: %s\n\n",
               getpid(), getppid(), src);

        /* ⒀ 用 lseek+read 显示文件 f1 的最终内容 */
        {
            char buf[page_size + 1];
            ssize_t n;
            lseek(fd, 0, SEEK_SET);
            printf("(13) 文件 f1 的最终内容: ");
            while ((n = read(fd, buf, page_size)) > 0) {
                buf[n] = '\0';
                printf("%s", buf);
            }
            printf("\n\n");
        }

        /* (14) 显示父进程中的 dst 内容 */
        printf("(14) 父进程: dst 的内容: %s\n\n", dst);

        free(dst);  dst = NULL;   /* 父进程也释放 dst */
        close(fd);
    }

    /* 此 printf 不在分支内, 父、子进程各执行一次 */
    printf("PID is: %d, PPID is: %d, The src value of end is: %s\n\n",
           getpid(), getppid(), src);

    /* 解除文件与内存的映射 */
    if (munmap(src, page_size) == 0) {
        printf("PID is: %d, PPID is: %d, munmap success\n\n",
               getpid(), getppid());
        src = NULL;
    } else {
        printf("PID is: %d, PPID is: %d, munmap failed\n\n",
               getpid(), getppid());
    }
    return 0;
}
