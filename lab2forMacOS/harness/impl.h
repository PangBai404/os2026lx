#ifndef LAB2_IMPL_H
#define LAB2_IMPL_H

/*
 * 实验二 内存管理 · 测例接口
 *
 * 学生只实现本头文件声明的 6 个函数(见 student/impl.c)，
 * main 与全部带 nonce 的输出都在 harness/2_harness.c 中，学生不可修改。
 *
 * ⚠ 契约：
 *   - child_work 在 fork 出的子进程中被调用，其中【不要】free(dst)/close(fd)，
 *     释放统一放在 cleanup()（否则快照检查会读到已释放内存）；
 *   - 学生函数内可以 printf 调试信息，但输出不带 nonce，判分脚本会忽略。
 */
typedef struct {
    char *src;        /* 共享存储区(mmap)地址   —— setup_shared 填写  */
    char *dst;        /* 堆备份(malloc)地址     —— backup_to_heap 填写 */
    int   fd;         /* 文件 f1 的描述符       —— setup_shared 填写  */
    int   pagesize;   /* 页面大小               —— setup_shared 填写  */
} ctx_t;

/* ★ 学号后四位：由学生在 student/impl.c 顶部定义并填写，
 *   harness 通过本声明读取。必须是 4 位 ASCII 数字 + '\0'，例如 "5678"。
 *   判分只要求它是 4 位数字；报告截图会显示它，请填自己的真实学号。 */
extern const char SID4[5];

/* (1)  getpagesize + open("./f1",O_RDWR) + mmap 一页 MAP_SHARED
 *      成功返回 0 并填齐 ctx 四个字段，失败返回 -1 */
int  setup_shared(ctx_t *ctx);

/* (3)(4)  malloc 一个页面 + memmove 把 src 内容复制到 dst
 *      成功返回 0，失败返回 -1 */
int  backup_to_heap(ctx_t *ctx);

/* (6)~(9)  子进程工作: memchr 在 dst 中找 'W'，把该处 4 字节(WXYZ)
 *      替换为 SID4(你在 impl.c 顶部填的学号后四位)，再把 dst 整页
 *      写回 src。temp 与 dst 的偏移量写回 *temp_off；
 *      成功返回 0，失败返回 -1 */
int  child_work(ctx_t *ctx, long *temp_off);

/* (12) 父进程工作: memset 把 src 前 2 字节替换为 'Q' */
void parent_work(ctx_t *ctx);

/* (13) lseek+read 读文件 f1 内容到 buf(保证以 '\0' 结尾)，
 *      返回读到的字节数，失败返回 -1 */
int  readback_file(ctx_t *ctx, char *buf, int bufsz);

/* (11)+(14) munmap + close + free(dst)，指针置 NULL；成功返回 0 */
int  cleanup(ctx_t *ctx);

#endif /* LAB2_IMPL_H */
