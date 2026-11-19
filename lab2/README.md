# 实验二 · 内存管理：mmap 共享存储区

## 实验说明

> 「文件躺在磁盘上，程序要读写它，最直接的办法是 read() 和 write()，
> 数据要在内核缓冲区和用户缓冲区之间来回拷贝两趟。
> 但是，如果把文件直接"映射"进内存，像访问数组一样读写它呢？」

欢迎来到实验 2，内存映射文件与共享存储区。
在这里，我们的目标是：
用 mmap() 把文件 f1 变成父子进程的共享存储区——
子进程把其中的 "WXYZ" 替换成**你的学号后四位**，
父进程再把开头两个字符替换成 "QQ"，
最终文件内容变成 QQxxxxxxx...，两个进程就这样完成了一次基于共享内存的"通信"。

实验会用到 mmap / fork 等 POSIX 系统调用，需要 POSIX 环境（Linux / macOS；Windows 用 WSL2）、C 语言和 python 3。
mmap 可以先理解为：把磁盘上的文件"搬"到进程的虚拟地址空间，之后对这段地址的
读写就等价于对文件的读写，磁盘访问变成了内存访问。
这个例子可以帮助你进一步理解，普通 C 程序读改写一个文件要这么做：

    buf = malloc(len);
    read(fd, buf, len);              /* 文件 → 用户缓冲区（第一次拷贝） */
    ... 修改 buf ...                 /* 在缓冲区里改 */
    lseek(fd, 0, SEEK_SET);
    write(fd, buf, len);             /* 用户缓冲区 → 文件（第二次拷贝） */

两次拷贝、两份存储。而用 mmap 之后：

    src = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    ... 直接修改 src[i] ...          /* 磁盘访问变成了内存访问 */

1. 完成本实验前，你需要学习：基本 C 语言编程（建议至少学到指针和结构体部分），
   以及进程的基本概念。
2. 你需要先准备好实验环境，参见"实验包目录""环境要求"和"常见问题"。
   ⚠ 与实验一不同：**本实验必须 POSIX 环境**（Linux / macOS；Windows 用 WSL2），原因见环境要求。
3. 如果你不是很清楚，这里和代码里都有详细的提示告诉你怎么做。
4. 写完代码后，可以查看本文档"快速开始"部分运行测例，看看自己写的对不对。

PS：你可以打开 VSCode 或者其它 IDE，用分窗口的方式把这个文件和代码同时展示，
这样使用效果更佳。

## 本部分实验教程

### 实验准备

请查看"实验包目录""环境要求"和"常见问题"。

### 第 0 步（热身，10 分钟）：先跑一遍完整程序

自动测例把 main 拆走后，第一次看可能不知道整体在干什么。建议先用教学完整版
mem.c 跑一遍，建立整体印象（**报告截图也从这里来**）：

    cd <实验包根目录>                     # 包含 README.md 的目录
    cp solution/mem.c ./mem.c             # 复制工作副本，避免修改参考实现
    # 编辑 ./mem.c：把 "2345" 改成你的学号后四位；
    # macOS 还需删除 #include <malloc.h>（或改成 <stdlib.h>）
    printf '0123456789abcWXYZde' > f1     # 准备数据文件（19 字节）
    gcc mem.c -o mem                      # 编译教学版
    ./mem                                 # 运行
    cat f1                                # 看被修改后的文件

你会看到：mmap 建好共享区 → 复制到堆 → fork 出子进程 → 子进程找到 W 换成学号
并写回共享区 → 父进程 wait 后把开头换成 QQ → 文件最终变成 QQxxxxxxx...。

报告需要的三张截图：
① ./mem 的完整输出；② cat f1 的结果；
③ 思考题现象（把 mem.c 里父进程的 wait(NULL) 注释掉后重新编译。
   建议截两张：一张是父进程 (12)(13) 的输出，另一张是进程结束后 cat f1。
   在 macOS 上多次运行可能都由父进程先执行，此时重点展示“父进程看到的内容”
   与“最终文件内容”不一致；跑完记得恢复）。

热身完成后，进入正题：你在 student/impl.c 里要做的事，和 mem.c 一一对应，
只是 main（以及 fork/wait、随机数据、全部判分输出）被拆进了测例骨架。

### 实验任务

你需要读懂并补完代码，在 TODO1 ~ TODO6 指定的位置补全；动手前先在 student/impl.c 顶部把 SID4 填成你的学号后四位，否则判分直接 FAIL(reason=sid)。
理解下述关键点：

1. src 是父子进程之间**唯一共享**的可写内存（MAP_SHARED 映射区）；
   dst 是各进程**私有**的堆内存（fork 写时复制）——测例会专门检查父进程的 dst
   是否仍是原始内容，来验证你真的理解了这一点。
2. child_work 里**不要** free(dst) 或 close(fd)：释放统一放在 cleanup()。
   测例会在 child_work 返回后立即快照内存，并在 cleanup 之后重读文件；
   提前 free 属于未定义行为，可能读到已回收的内存，请不要这样做。
3. 判分模式下 f1 的内容**每轮随机**（WXYZ 的位置也在变），学号后四位填在
   `student/impl.c` 顶部的 `SID4`；harness 的命令行参数只用于传入 19 字节
   测试数据，不是学号——函数里不允许出现任何硬编码的位置或答案。
4. mmap 失败的返回值是 **MAP_FAILED**（即 (void *)-1），不是 NULL，别判错对象。

TODO1：补完 int setup_shared(ctx_t *ctx)　——对应教学版 (1)
1. ctx->pagesize = getpagesize(); 获取本机页面大小（Linux 通常 4096；Apple Silicon macOS 为 16384）；
2. ctx->fd = open("./f1", O_RDWR); 打开当前目录下的 f1，失败返回 -1；
3. ctx->src = mmap(NULL, ctx->pagesize, PROT_READ | PROT_WRITE, MAP_SHARED,
   ctx->fd, 0); 把 f1 映射成共享存储区，与 MAP_FAILED 比较判断失败，失败返回 -1。

TODO2：补完 int backup_to_heap(ctx_t *ctx)　——对应教学版 (3)(4)
1. ctx->dst = malloc(ctx->pagesize); 申请一个页面的堆内存，失败返回 -1；
2. memmove(ctx->dst, ctx->src, ctx->pagesize); 把共享区整页复制到 dst；
3. 返回 0。

TODO3：补完 int child_work(ctx_t *ctx, long *temp_off)
——对应教学版 (6)(7)(8)(9)，在 fork 出的子进程里被调用
1. char *temp = memchr(ctx->dst, 'W', ctx->pagesize); 在整页范围内找 'W'，
   找不到返回 -1；
2. *temp_off = temp - ctx->dst; 记录 'W' 的偏移量（测例会核对它）；
3. memmove(temp, SID4, 4); 把 WXYZ 换成你在文件顶部填的学号；
4. memmove(ctx->src, ctx->dst, ctx->pagesize); 把改好的 dst 写回共享区；
5. 返回 0。再次强调：不要在这里 free / close！

TODO4：补完 void parent_work(ctx_t *ctx)　——对应教学版 (12)
1. memset(ctx->src, 'Q', 2); 把共享区前 2 个字节换成 'Q'，就这一行。

TODO5：补完 int readback_file(ctx_t *ctx, char *buf, int bufsz)　——对应 (13)
1. lseek(ctx->fd, 0, SEEK_SET); 把读写位置拨回文件头；
2. 循环 read 把文件内容读进 buf，最多读 bufsz-1 字节，读完补 '\0'；
3. 返回读到的总字节数，失败返回 -1。

TODO6：补完 int cleanup(ctx_t *ctx)　——对应教学版 (11)(14)
1. munmap(ctx->src, ctx->pagesize); 解除映射（length 必须和 mmap 时一致）；
2. close(ctx->fd); free(ctx->dst); 释放后把指针置 NULL，防止野指针；
3. 全部成功返回 0，有失败返回 -1。

### 参考和知识详解

本实验涉及的函数如下：

    getpagesize()      获取内存页面大小（字节）
    mmap()             把文件映射进内存（本实验核心）
    munmap()           解除映射
    malloc()/free()    申请/释放堆内存
    memmove()          复制内存区域（允许重叠）
    memchr()           在内存前 n 字节中查找指定字节
    memset()           把内存区域填充为指定字节
    fork()             创建子进程（测例负责调用）
    wait()             等待子进程结束（测例负责调用）
    getpid()/getppid() 获取当前 / 父进程的 PID
    lseek()/read()/close()  移动文件读写位置 / 读文件 / 关闭文件

TODO1 涉及的 mmap() 是本实验的主角：

    void *mmap(void *start, size_t length, int prot, int flags,
               int fd, off_t offset);
    start   通常填 NULL，让内核自己挑地址
    length  映射长度，本实验填一个页面大小
    prot    PROT_READ | PROT_WRITE：可读可写
    flags   MAP_SHARED：你的写入会同步到文件，并且被映射同一文件的其他进程
            看到；换成 MAP_PRIVATE 就是私有副本，这正是它和 malloc 的区别
    fd      open("./f1", O_RDWR) 返回的文件描述符
    offset  从文件哪个字节开始映射，填 0
    成功返回映射区起始地址，失败返回 MAP_FAILED

fork() 负责造出"两个进程"，wait() 负责等它们汇合。
fork 一次调用、两次返回：在子进程里得到 0，在父进程里得到子进程的 PID。
fork 之后子进程继承父进程的全部内存（写时复制），但共享与否要看类型：

    - 对 MAP_SHARED 映射区的修改：父子双方都看得见 → 这就是"共享存储区"，
      也是本实验实现进程通信的原理；
    - 对 malloc 出来的 dst 的修改：只在各自进程内可见 → 这是"私有内存"。

这两条正是本实验要你亲眼验证的现象，也是测例重点检查的不变量。

mmap 与 malloc 的三点区别：
① mmap 涉及文件和内存，malloc 只涉及内存；
② mmap 以页为单位分配，malloc 以字节为单位；
③ mmap 是系统调用，malloc 是库函数。
mmap 与 munmap 配对、malloc 与 free 配对，防止"内存泄漏"；
释放后把指针置 NULL，防止"野指针"。

## 实验包目录

    lab2/
    ├── Makefile
    ├── README.md             也就是你在看的这个
    │
    ├── student/impl.c        ★ 你要写的文件（TODO1~TODO6，不含 main）
    ├── student/framework.c   教学框架版（TODO(1)~(14)），可选练习
    ├── harness/2_harness.c   测例骨架：自带 main，负责 fork、输出与校验
    ├── harness/impl.h        接口契约：6 个函数的签名与注意事项
    ├── tests/check.py        测例：检查不变量，而不是比对整段输出
    │
    ├── solution/impl.c       函数版答案，与你写的文件结构一致
    ├── solution/mem.c        教学完整版（含 main），热身与报告截图用
    └── build/                编译产物（make 自动创建）

> 第 0 步会把 `solution/mem.c` 复制成根目录下的 `mem.c` 作为工作副本；
> 它不是实验包自带文件。

**为什么 main 在测例里？**

student/impl.c 只包含 6 个函数，main 在 harness/2_harness.c 里。
这样输出格式、进程关系检查、数值校验都不在你手里，测例才判得准。
本实验还多一层考虑：fork/wait 的时机、每一轮 f1 的随机数据都由测例统一安排，
保证所有同学在完全相同的条件下被判分，结果可复现。

建议先读 solution/mem.c（或第 0 步复制出的 mem.c）理解整体流程，再回到 student/impl.c 补 TODO。

## 环境要求

| 组件 | 用途 | 是否必需 |
|---|---|---|
| POSIX 系统（Linux / macOS）+ C 编译器（gcc/clang） | 编译运行实验代码 | 必需 |
| Python 3.4+ | 运行测例 | 必需 |
| make | 简化命令 | 可选 |

**⚠ 与实验一的重要差异：本实验不需要"跨平台"，而是"必须 POSIX"。**
fork、mmap、wait 都是 Unix 系统调用，Windows 原生编译器（MinGW/MSVC）
没有这些接口，所以实验一那种"Windows 直连"的路线在这里走不通：

- **Windows 用户**：安装 WSL2（管理员 PowerShell 执行 wsl --install），
  或使用实验室的 VMware / VirtualBox Ubuntu 虚拟机（本实验默认环境）；
- **Linux**：原生支持，sudo apt install build-essential python3 即可；
- **macOS**：原生支持 POSIX 接口，xcode-select --install 即可。注意：
  `solution/mem.c` 与 `student/framework.c` 目前包含非 POSIX 的
  `<malloc.h>`；macOS 上没有该头文件，编译这两个文件前请删除该行
  （`<stdlib.h>` 已包含 malloc/free 声明）。主判分链路 `student/impl.c`
  不受影响。

### 检查环境

    getconf PAGESIZE        # Linux 通常 4096；Apple Silicon macOS 为 16384
    python3 --version       # 3.4+（建议 3.8+）
    make test               # 参考答案应 RESULT: PASSED——这是环境 OK 的最终判据

## 快速开始

    make test           # 编译参考答案并判分——先确认环境和工具正常（应 PASSED）
    make test-student   # 编译你的实现并判分（改完 student/impl.c 后跑这个）

**如果机器上没有 make**，等效命令：

    mkdir -p build
    gcc -Iharness harness/2_harness.c solution/impl.c -o build/2_harness
    gcc -Iharness harness/2_harness.c student/impl.c -o build/2_harness_student
    python3 tests/check.py build/2_harness_student


想手动看一次 harness 的原始输出（理解测例在查什么）：

    ./build/2_harness_student                       # 使用演示数据
    ./build/2_harness_student 0123456789abcWXYZde   # 传 19 字节数据

学号后四位从 `student/impl.c` 顶部的 `SID4` 读取，不是命令行参数。

想跑教学完整版（报告截图）：

    cp solution/mem.c ./mem.c     # 复制工作副本，避免修改参考实现
    # 编辑 ./mem.c：把 "2345" 改成你的学号后四位；
    # macOS 还需删除 #include <malloc.h>（或改成 <stdlib.h>）
    printf '0123456789abcWXYZde' > f1
    gcc mem.c -o mem
    ./mem
    cat f1

## 测例

不比对完整输出，只检查与实现方式无关的不变量：

| 检查项 | 说明 |
|---|---|
| [sid] SID4 | 必须是 4 位 ASCII 数字，否则 FAIL(reason=sid) |
| [setup] pagesize | 等于本机真实页大小（脚本独立查询，不信任程序自报） |
| [initial] src | 等于本轮随机写入 f1 的内容——mmap 必须真的做了 |
| [backup] dst | 等于 f1 原始内容——malloc + memmove 必须真的做了 |
| [child] temp_off | 等于 'W' 的真实偏移——数据是随机的，必须真用 memchr 查 |
| [child] dst/src | WXYZ 已换成学号后四位，且修改后的 dst 写回了共享区 |
| [parent] src | 在子进程的结果之上叠加了 QQ |
| [parent] dst | 仍是原始内容——私有堆没有被另一个进程污染（写时复制） |
| [proc] pid/ppid | 子进程的 PPID 必须等于父进程的 PID |
| [end] 两行 | child 行不含 QQ，parent 行含 QQ（对应教学版"最后的 printf 父子各执行一次"） |
| [file] / [file_after] | 程序读回的内容、以及程序退出后测例重新打开读到的内容，都等于 QQ+学号串 |
| [cleanup] rc | cleanup() 返回 0（契约要求 munmap/close/free 均成功） |
| 连续 6 轮 | 1 轮演示数据 + 5 轮随机数据全部通过（--rounds 可调轮数） |

单次运行的 harness 输出长这样（演示数据、SID4=2345）：

    nonce=3f7a1c02
    3f7a1c02 [data] wpos=13 len=19 content=0123456789abcWXYZde
    3f7a1c02 [sid] value=2345 isdigits=true
    3f7a1c02 [setup] rc=0 pagesize=4096 expected_pagesize=4096
    3f7a1c02 [initial] src=0123456789abcWXYZde
    3f7a1c02 [backup] rc=0 dst=0123456789abcWXYZde
    3f7a1c02 [proc] role=child pid=5126 ppid=5125
    3f7a1c02 [child] rc=0 temp_off=13 dst=0123456789abc2345de src=0123456789abc2345de
    3f7a1c02 [end] role=child src=0123456789abc2345de
    3f7a1c02 [proc] role=parent pid=5125 ppid=4890
    3f7a1c02 [parent] src=QQ23456789abc2345de dst=0123456789abcWXYZde
    3f7a1c02 [file] n=19 content=QQ23456789abc2345de
    3f7a1c02 [end] role=parent src=QQ23456789abc2345de
    3f7a1c02 [cleanup] rc=0
    3f7a1c02 [file_after] n=19 content=QQ23456789abc2345de
    3f7a1c02 [final] result=PASS

> 示例中的 `pagesize=4096` 是 Linux 常见值；Apple Silicon macOS 会显示
> 16384，属正常现象。判分脚本用本机真实页大小核对，不写死 4096。

判分脚本汇总后输出：

    round 1: data=0123456789abcWXYZde -> PASS
    round 2: data=kx7pabWXYZ9mzq1de  -> PASS
    ...
    RESULT: PASSED (6 rounds, 不变量 I0~I12 全部满足)

## 为什么无法靠"打印答案"过关

1. **nonce 前缀**：main 在 harness 里，每次运行生成一个随机数 nonce，所有
   判分行都以它开头。你的代码拿不到 nonce——自己 printf 的行不带前缀，
   check.py 解析时整行忽略。
2. **数据随机化**：判分模式下 f1 的内容每轮重新随机生成，WXYZ 出现的位置
   也在变。
3. **退出后重读文件**：程序结束之后，测例会重新 open f1 读一遍。如果你用
   malloc 冒充 mmap（内存改了但文件没变），这一步必挂。
4. **源码审计**：make audit 会扫描 student/impl.c 是否试图输出 harness 的
   保留标记（[final]、nonce= 等），并作为记录。

> 上述保证以 `harness/2_harness.c` 和 `tests/check.py` 未被篡改为前提。
> 比赛判分建议使用干净的 harness/check.py 重新编译，或校验其哈希；
> `make audit` 是独立的手动检查，不会随 `make test-student` 自动运行，
> 如作为判分门槛请接入判分脚本/CI。

## 常见问题

**Windows 下 gcc 报 fork/mmap 未定义**

本实验的 fork/mmap/wait 是 POSIX 系统调用，Windows 原生编译器没有。
请改用 WSL2 或 Ubuntu 虚拟机（见"环境要求"）。

**macOS 下编译 mem.c / framework.c 报 malloc.h not found**

这两个教学文件包含非 POSIX 的 `<malloc.h>`。删除该行（或改成
`#include <stdlib.h>`）即可；`<stdlib.h>` 已声明 malloc/free。
主实验 `student/impl.c` 不包含该头文件，不受影响。

**mmap 总是失败（打印 error，或 setup 环节 FAIL）**

- f1 不在当前目录：测例运行时会自动创建 f1，但你需要 cd 到 lab2 目录再跑；
- ctx->pagesize 没赋值（为 0）：检查 TODO1 是否写了 getpagesize()。

**[child] temp_off=-1，或者演示数据过、某轮随机数据才失败**

memchr 没找到 'W'，多半是代码里写死了 0123456789abcWXYZde 或固定下标 13。
删掉硬编码，真的调用 memchr 在整页范围内查。

**手动编译报 impl.h: No such file or directory**

漏了 -Iharness。impl.h 在 harness/ 目录下，gcc 需要这个参数才能找到它。

**[final] result=FAIL reason=backup / [child] rc=2**

reason=backup：TODO2 没做对（malloc 或 memmove 的问题）；
[child] rc=2：你的 child_work 返回了非 0，看同一行里 dst/src 内容找原因。

**在 child_work 里 free/close 了会怎样？**

契约：释放统一放在 cleanup()。测例在 child_work 返回后会立即快照 dst/src；
如果提前 free 且指针未置 NULL，读取已释放内存属于未定义行为，测例不保证
一定能检测到；在子进程里 close(fd) 也不会影响父进程。请不要依赖测试是否能
抓住，按契约把释放统一放在 cleanup()。

**测例输出的中文变成问号或方块**

本实验不支持 Windows 原生编译；请在 WSL2（UTF-8 终端）或 Linux/macOS 下运行。
harness 的判分行以 ASCII 标签为主，但 check.py 的失败提示包含中文；
在非 UTF-8 控制台可能显示为问号。改用 UTF-8 终端即可，判分逻辑不受影响。

**make 报 gcc: command not found，或根本没有 make**

Makefile 已显式指定 CC := gcc。macOS 请先安装 Xcode Command Line Tools
（xcode-select --install），Linux 安装 build-essential；没有 make 就用
“快速开始”里的 gcc + python3 三条命令。

**FAIL reason=sid / SID4 非法**

你没在 student/impl.c 顶部填学号，或填的不是 4 位数字

## 与原始实验的区别

- 原实验要求把 mem.c 整个程序写出来，判分靠截图和人工比对；这里拆成 6 个
  函数 + 自动判分，每条断言都可读、可解释。
- 起点代码会**编译通过但测例失败**，学生能先看到失败长什么样。
- 判分数据随机化、输出带随机 nonce，在 harness/check.py 未被篡改的
  前提下，学生无法通过伪造输出或硬编码答案绕过检查。
- fork/wait 固定在测例中同步执行（判分需要可复现）；"删掉 wait(NULL) 观察
  父子竞争"的思考题保留在教学版 mem.c 中，由你亲手实验并截图。
