# 实验一 · 任务 4：生产者与消费者

## 实验说明

> 「生产者一个劲地往桌子上放菜，消费者一个劲地吃，桌子只有那么大——放不下就得等，没菜就得等。」
（嗯，生产者其实可以叫厨师或者传菜员，随意。）

欢迎来到实验1-4，生产者与消费者。
在这里，我们的目标是：
5 个生产者、5 个消费者，共享一个容量为 8 的环形缓冲池。每个生产者生产 20 个产品，一共 100 个。
产品用 4 位编号：最高位是生产者编号，其余是该生产者的产品序号。
你要用信号量 + 互斥锁，保证缓冲池既不溢出也不空读，且每个产品恰好被消费一次。

1. 完成本实验前，你需要学习：基本C语言编程（建议至少学到指针和结构体部分），以及上一关的信号量
2. 你需要先准备好做实验的环境，参见“实验包目录”，“环境要求”和“常见问题”这些部分来获得帮助。
3. 如果你不是很清楚，这里和代码里都会有详细的提示告诉你怎么做。

PS：你可以打开 VSCode 或者其它IDE，可以用分窗口的方式将这个文件和代码同时展示，这样使用效果更佳。

## 本部分实验教程

### 实验准备

请查看“实验包目录”，“环境要求”和“常见问题”

### 1-4的实验任务

你要写的是 `student/impl.c`，里面有**三处 TODO**：

| TODO | 函数 | 要写什么 |
|---|---|---|
| TODO 1 | `pc_init` | 初始化两个信号量：空位初值 `bufsize`，产品数初值 0 |
| TODO 2 | `pc_put` | 等空位 → 加锁 → 写入 → 解锁 → 通知有产品 |
| TODO 3 | `pc_get` | 等产品 → 加锁 → 读取 → 解锁 → 通知有空位 |

先看最小的模型：1 个生产者、1 个消费者、1 个缓冲位。
这种情况只需要两个信号量，**不需要互斥锁**——因为同一时刻只有一个线程会碰缓冲。

```c
static int slot;
static sem_t can_put;   /* 初值 1：有一个空位 */
static sem_t can_get;   /* 初值 0：没有产品 */

void *producer(void *arg) {
    for (int i = 0; i < ITEMS; i++) {
        sem_wait(&can_put);
        slot = 1000 + i;
        sem_post(&can_get);
    }
    return NULL;
}

void *consumer(void *arg) {
    for (int i = 0; i < ITEMS; i++) {
        sem_wait(&can_get);
        int v = slot;
        sem_post(&can_put);
    }
    return NULL;
}
```

本实验从 1 对 1 扩展到 5 对 5、从 1 个位置扩展到 8 个位置，
于是就多了一件事：**必须保证同一时刻只有一个线程在改缓冲**。
`solution/4.c` 是完整的参考程序，可以对照着看这一步是怎么加的。

理解下述关键点：

1. **两个信号量管"能不能进"，互斥锁管"同时只有一个人在动缓冲"。** 少任何一个都会出问题。
2. **等待信号量必须在加锁之前。** 如果先加锁再等空位，持锁的线程会阻塞住所有其他线程，直接死锁。
3. `lab_buf_push` / `lab_buf_pop` 由测例提供，**故意不加锁**——互斥必须由你保证。
4. 产品编号最高位是生产者编号，这一条由测例在你提交产品后核对，写错会被判失败。

## 实验包目录

```
lab1-4/
├── Makefile
├── README.md          也就是你在看的这个
│
├── student/impl.c     ★ 你要写的文件（三处 TODO，不含 main）
├── tests/harness.c      测例骨架：自带 main，持有缓冲池并核对每个产品
├── tests/check.py       测例：检查不变量，而不是比对整段输出
│
├── solution/4.c         完整单文件版答案（含 main）：5 对 5、8 个缓冲位
└── solution/impl.c      函数版答案，与你写的文件结构一致
```

**为什么 main 在测例里？**

`student/impl.c` 只包含三个函数，`main` 在 `tests/harness.c` 里，
缓冲池也由测例持有。这样「池子里同时有几个、每个产品被谁取走」
都由测例统计，你不碰这些代码，测例才判得准。

建议先读 `solution/4.c` 理解整体流程，再回到 `student/impl.c` 补 TODO。

## 环境要求

只需要两样东西：

| 组件 | 用途 | 是否必需 |
|---|---|---|
| 支持 pthread 的 C 编译器 | 编译实验代码 | 必需 |
| Python 3.8+ | 运行测例 | 必需 |
| make | 简化命令 | 可选 |
| RISC-V 交叉工具链 + qemu-user | `make riscv` 目标 | 可选 |

**不需要 MSYS2、不需要 WSL、不需要虚拟机。**

**macOS 说明（本版本）**：macOS 没有可用的 `sem_init`。实验包在
`../include/sem_compat.h` 中把它映射到 `sem_open`；你仍然按题目写
`sem_init` / `sem_wait` / `sem_post`。Makefile 已加 `-I../include`，不需要
改代码；Linux / Windows 上该头文件是空操作。

### Windows

推荐 **w64devkit**：单个压缩包，解压后含 gcc / make / gdb，把 `bin` 目录加进 PATH 即可。
<https://github.com/skeeto/w64devkit/releases>

也可以用 MSYS2（`pacman -S mingw-w64-ucrt-x86_64-gcc make`）。
Visual Studio 的 MSVC 不支持 `-pthread`，本实验未做适配。

### Linux / macOS

```bash
# Debian / Ubuntu
sudo apt install build-essential python3
# macOS（先装 Xcode Command Line Tools 或者你称手的IDE，搞好clang即可）
xcode-select --install
```

### 检查环境

```bash
python3 ../tools/check_env.py
```

脚本会实际编译并运行一个 pthread + semaphore 小例子，确认线程库和信号量
真的可用，而不是只看 `gcc --version` 能不能打印。

## 快速开始

```bash
make test-student  # 编译你的实现并运行测例（改完 student/impl.c 后跑这个）
make test          # 编译参考答案并运行测例（用来对照）
```

**如果机器上没有 make**，两条命令同样有效：

```bash
gcc -I../include -pthread student/impl.c tests/harness.c -o 4
python3 tests/check.py 4
```

想看完整单文件程序（含 main）：

```bash
make demo && ./build/4_solution  # 5 对 5、8 个缓冲位
```

## 测例判什么

测例会创建 5 个生产者（每个生产 20 个）和 5 个消费者，缓冲池容量 8。
不比对完整输出，只检查与实现方式无关的不变量：

| 检查项 | 说明 |
|---|---|
| `produced` / `consumed` | 都必须等于 100，即没有丢失也没有卡死 |
| `peak_occupancy` | 缓冲池被同时占用的最大数量，绝不能超过 8 |
| `overflow` | 必须为 0，即没有在池满时写入 |
| `underflow` | 必须为 0，即没有在池空时读取 |
| `duplicates` | 必须为 0，即没有产品被消费两次 |
| `missing` | 必须为 0，即每个产品都被消费到了 |
| `unexpected` | 必须为 0，即产品编号都符合"最高位是生产者"的规则 |
| `leftover` | 必须为 0，即结束时缓冲池是空的 |
| 30 秒超时 | 死锁时直接判失败 |
| 连续 5 次运行 | 结果一致 |

### 为什么这些检查能抓到典型的错误

测例为了把问题暴露出来，刻意做了两件事：

- **生产者不等待、消费者每个歇 1 毫秒**，缓冲池会被真的填满。
  如果漏掉 `empty` 信号量，`overflow` 立刻暴涨（实测 91 次）。
- **缓冲读写内部插入了少量延迟**，把竞争窗口拉宽。
  如果漏掉互斥锁，两个线程会同时改 `head`/`tail`，
  出现重复和丢失（实测 24 个重复）。

这两项都是实测量出来的，不是假设。

### 为什么输出不能被伪造

`main` 和缓冲池都在 `tests/harness.c` 里，你碰不到。每次运行测例都会生成一个随机前缀
（`nonce=xxxxxxxx`），所有有效输出行都带着它；测例只认带本次前缀的行。
每个产品被谁取走、池子同时占用几个，全部由测例记录，
所以「直接把结果 printf 出来」不会生效。

## 常见问题

**程序卡住不退出**

多半是死锁。最常见的原因是把 `sem_wait` 放到了 `pthread_mutex_lock` 之后：
持锁的线程在等空位，而能让它等到空位的线程又拿不到锁。

**macOS 上 `sem_init` 报 deprecated 或返回 -1？**

macOS 的 `sem_init` 不可用；本版本用 `../include/sem_compat.h` 映射到 `sem_open`。
请通过 Makefile 编译（已带 `-I../include`），不要把 `student/impl.c` 单独拷到
没有这个头文件的目录编译。

**`overflow` 大于 0，测例说「池满时还在写入」**

漏了 `sem_wait(&empty_sem)`，或者初值设错了。
`pc_init` 里 `empty_sem` 的初值必须是 `bufsize`（也就是 8），不是 0。

**`underflow` 大于 0，测例说「池空时还在读取」**

`pc_get` 里漏了 `sem_wait(&full_sem)`，或者 `full_sem` 的初值不是 0。

**`duplicates` 和 `missing` 都不为 0**

漏了互斥锁。两个线程同时改 `head`/`tail`，会让同一个槽位被读两次、
另一个槽位被跳过。检查 `pthread_mutex_lock` 是不是包住了 `lab_buf_push` / `lab_buf_pop`。

**`unexpected` 大于 0**

产品编号算错了。要求是 4 位、最高位为生产者编号，例如 3 号生产者的第 5 个产品是 3005。

**`peak_occupancy` 一直是 1 或者很小**

不影响判分，但说明你的实现把并发压成了串行——比如把整个生产消费过程都放在一把锁里。
可以想想怎么让多个生产者、多个消费者真正并行。

**`Cannot create temporary file in C:\WINDOWS\`**

MSYS2 下 `make` 可能不把 `TMP`/`TEMP` 传给子进程，gcc 便退回到 `C:\WINDOWS` 建临时文件。
本目录的 Makefile 已把临时目录指向 `build/tmp`；若在别的项目里遇到，可手动设置：

```bash
export TMPDIR="$PWD/build/tmp" TMP="$TMPDIR" TEMP="$TMPDIR"
```

**测例输出的中文变成问号或方块**

Windows 控制台默认代码页通常不是 UTF-8。本目录的测例与检查脚本刻意只用 ASCII 输出，
避免这个问题。自己写测例时建议照做。

**`make` 报 `cc: command not found` 或临时文件错误**

MSYS2 的 `cc` 包装器在某些配置下异常。Makefile 已显式指定 `CC := gcc`。

## 与原始实验的区别

- 原平台的判题标准不公开；这里的每条断言都可读、可解释。
- 原题只要求"模拟 5 个生产者和 5 个消费者"，本实验把
  「池满时写入」和「池空时读取」作为独立指标报告出来，
  即使某次运行侥幸没出事，错误也逃不掉。
- 原平台的判题器对代码格式有隐含要求（例如信号量必须写在同一行），
  那是判题器的缺陷。本实验只关心行为，不关心你写成几行。

## RISC-V（可选）

如果指导老师要求实验支持 RISC-V。同一份源码可以直接交叉编译：

```bash
sudo apt install gcc-riscv64-linux-gnu qemu-user   # WSL / Linux
make riscv
```

代码中没有平台相关的条件编译，x86-64 与 riscv64 共用同一份源码。
