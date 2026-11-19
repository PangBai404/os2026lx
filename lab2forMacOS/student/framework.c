/*=====================================================================
 * 实验二 内存管理 —— 实验程序框架（学生补全版）
 * 来源：《实验二 内存管理》五、实验程序框架
 *
 * 使用方法：
 *   1. 编译（在实验包根目录）：gcc student/framework.c -o framework
 *   2. 标有 [TODO(n)] 处即指导书要求补全的 (1)~(14)，可参考 mem.c，
 *      但须自己动手编写；
 *   3. 运行前须在当前目录创建数据文件 f1：
 *         printf '0123456789abcWXYZde' > f1
 *   4. 未完成 TODO(1) 时 page_size=0，mmap 会失败打印 error 后退出，
 *      属正常现象——请先补全 (1)。
 *====================================================================*/
#include <stdio.h>
#include <stdlib.h>     /* exit                                */
#include <string.h>     /* memmove / memchr / memset           */
#include <unistd.h>     /* getpagesize/fork/getpid/getppid/close/lseek/read */
#include <fcntl.h>      /* open / O_RDWR                       */
#include <sys/types.h>  /* pid_t                               */
#include <sys/wait.h>   /* wait                                */
#include <sys/mman.h>   /* mmap / munmap / MAP_SHARED / MAP_FAILED */

int main(void)
{
    int   fd, page_size = 0;  /* TODO(1) 前保持 0，避免未初始化 */
    pid_t pid;
    char *dst = NULL, *temp = NULL;   /* dst: 堆缓冲区  temp: 查找结果 */
    char *src = NULL;                 /* src: 共享存储区(映射区)地址   */
    (void)temp;                       /* TODO(6)(7) 补全后会真正使用   */

    /*-------------- (1) 显示当前系统的页面大小 ----------------------*/
    /* TODO(1): 调用 getpagesize() 存入 page_size, 并 printf 显示
     *          Apple Silicon macOS 返回 16384，Intel Mac 返回 4096。 */
    printf("[TODO(1)] 请补全: 显示页面大小\n");

    fd = open("./f1", O_RDWR);        /* 打开当前目录下的文件 f1 */
    if (fd == -1) { perror("open ./f1"); exit(1); }

    /* 创建共享存储区: 大小为一个页面, 将 f1 映射进内存, 地址返回到 src */
    src = (char *)mmap(NULL, page_size, PROT_READ | PROT_WRITE,
                       MAP_SHARED, fd, 0);
    if (src == MAP_FAILED)
    { printf("error\n");
      exit(1);
    }

    /*-------------- (2) 显示 src 的内容 ----------------------------*/
    /* TODO(2): printf 输出 src 指向的共享存储区内容 */
    printf("[TODO(2)] 请补全: 显示 src 的内容\n");

    /*-------------- (3) malloc 申请一个页面大小的内存, 地址返回 dst --*/
    /* TODO(3): dst = malloc(page_size); 注意判 NULL */
    printf("[TODO(3)] 请补全: malloc 申请 dst\n");

    /*-------------- (4) memmove 将共享存储区的内容复制到 dst ---------*/
    /* TODO(4): memmove(dst, src, page_size); */
    printf("[TODO(4)] 请补全: memmove src -> dst\n");

    /*-------------- (5) 显示 dst 的内容 ----------------------------*/
    /* TODO(5): printf 输出 dst 内容 */
    printf("[TODO(5)] 请补全: 显示 dst 的内容\n");

    pid = fork();                     /* 创建子进程 */

    if (pid == 0) { /* ================= 子进程空间 ================ */
        /*------ (6) memchr 在 dst 中搜索 'W', 地址返回 temp --------*/
        /* TODO(6): temp = memchr(dst, 'W', page_size); 并判空 */
        printf("[TODO(6)] 子进程: 请补全 memchr 查找 'W'\n");

        /*------ (7) 显示 temp 的内容及其内存起始地址 ---------------*/
        /* TODO(7): printf 输出 temp 内容及地址(地址用 %p) */
        printf("[TODO(7)] 子进程: 请补全显示 temp 内容及地址\n");

        /*------ (8) 将 temp 前 4 个字符("WXYZ")用学号后四位替换 -----*/
        /* TODO(8): memmove(temp, "xxxx", 4); ("xxxx"换成你的学号后四位)
                    再显示替换后的 dst 内容 */
        printf("[TODO(8)] 子进程: 请补全替换学号后四位\n");

        /*------ (9) memmove 将修改后的 dst 复制回 src --------------*/
        /* TODO(9): memmove(src, dst, page_size); */
        printf("[TODO(9)] 子进程: 请补全 memmove dst -> src\n");

        /*------ (10) 显示 src 的内容(包含 PID 及 PPID) -------------*/
        /* TODO(10): printf 输出 getpid(), getppid(), src */
        printf("[TODO(10)] 子进程: 请补全显示 src(含 PID/PPID)\n");

        /*------ (11) 关闭文件 f1, 释放 dst --------------------------*/
        /* TODO(11): close(fd); free(dst); dst = NULL; */
        printf("[TODO(11)] 子进程: 请补全 close/free\n");
    }
    else if (pid > 0) { /* ============== 父进程空间 ============== */
        wait(NULL);                   /* 父进程阻塞自己, 等待子进程 */

        /*------ (12) memset 将 src 前 2 个字符用 'Q' 替换并显示 -----*/
        /* TODO(12): memset(src, 'Q', 2);
                    显示替换后的内容(包含 PID 及 PPID) */
        printf("[TODO(12)] 父进程: 请补全 memset 'QQ'\n");

        /*------ (13) 显示文件 f1 的最终内容 ------------------------*/
        /* TODO(13): lseek(fd, 0, SEEK_SET) 后循环 read 并打印 */
        printf("[TODO(13)] 父进程: 请补全显示 f1 最终内容\n");

        /*------ (14) 显示父进程中的 dst 内容 -----------------------*/
        /* TODO(14): printf 输出父进程的 dst 内容 */
        printf("[TODO(14)] 父进程: 请补全显示 dst\n");

        free(dst);  dst = NULL;       /* 父进程释放自己的 dst */
        close(fd);
    }

    /*===== 以下为框架固定部分, 不需修改 =====*/
    printf("PID is: %d,  PPID is: %d,  The src value of end is: %s\n\n",
           getpid(), getppid(), src);  /* 显示 src 指向的共享存储区内容 */

    if (munmap(src, page_size) == 0)   /* 释放共享存储区 */
    { printf("PID is: %d,  PPID is: %d,  munmap success\n\n",
             getpid(), getppid()); }
    else
    { printf("PID is: %d,  PPID is: %d,  munmap failed\n\n",
             getpid(), getppid()); }

    return 0;
}
