# 实验一 · 任务 1：三线程累加

## 实验说明

> 「用一个简单C程序计算 1-200 的数字和，最简单的实现是按照顺序一个一个加上去，但是，如果把它们分成好几段来同时做呢？」

我们用这个情景来实操学习一下线程的操作。

欢迎来到实验1-1，三个线程累加。
在这里，我们的目标是：
用 3 个线程分别完成 `p1`、`p2`、`p3` 的计算任务，求出 1 到 200 的累加和（20100）。

实验会用到 POSIX Threads，需要 C语言 和 python 3。
POSIX Threads是一套用于创建、管理和控制多线程程序的 API（应用程序编程接口）。
你可以先把它理解为：C 语言中用来实现多线程编程的一套工具。
这个例子可以帮助你进一步理解，看这个普通C程序
    `int main(void) {p1();p2();p3();return 0;}`
在执行的时候，三个函数p1,p2和p3是依次执行的，如果我们希望三个任务能够并发执行，就可以使用 pthread 创建三个线程。

1. 完成本实验前，你需要学习：基本C语言编程（建议至少学到指针和结构体部分）
2. 你需要先准备好做实验的环境，参见“实验包目录”，“环境要求”和“常见问题”这些部分来获得帮助。
3. 如果你不是很清楚，这里和代码里都会有详细的提示告诉你怎么做。
4. 写完代码后，可以查看本文档“快速开始”部分以运行它，看看自己写的对不对

PS：你可以打开 VSCode 或者其它IDE，可以用分窗口的方式将这个文件和代码同时展示，这样使用效果更佳。

## 本部分实验教程

### 实验准备

请查看“实验包目录”，“环境要求”和“常见问题”

### 1-1的实验任务

你需要读懂并补完代码，在TODO1 TODO2 TODO3指定的位置补全。
理解下述关键点：
1. 每个线程只写自己那份 task 结构，彼此不共享可写变量 —— 没有数据竞争。
2. 主线程必须 join，不能提前退出。
3. 段落的划分由 main 负责，线程函数本身不关心 1..200 怎么切。

TODO1：补完 static void *run_segment(void *arg)
1. 完成单个线程的计算任务，求 [t->start, t->end] 的整数和，写入 t->sum
2. 声明一个长整数累加值，将结构体task_t的start作为循环起始，结构体task_t的end作为终止，执行累加，最后写回。

TODO2：补完创建线程的代码，
1. 对每个 i，调用 pthread_create(&tasks[i].tid, NULL, entry[i], &tasks[i]) 并检查返回值，失败时打印错误并返回 1
2. 声明一整形变量作为返回值接受检查，在循环中添加一 if 条件，检查返回值，return 1

TODO3：补完等待所有线程结束的代码
1. 用 pthread_join()，不 join 的话，主线程结束后子线程会被杀死，结果可能来不及写回

### 参考和知识详解

POSIX Threads 的一些函数如下
    `pthread_create()`	创建线程
    `pthread_join()`	等待线程结束
    `pthread_exit()`	退出当前线程
    `pthread_self()`	获取当前线程 ID
    `pthread_equal()`	比较两个线程是否相同
    `pthread_mutex_lock()`	加锁
    `pthread_mutex_unlock()`	解锁

TODO3 涉及到的 `pthread_create()`，
它负责分配工作，pthread_join() 负责等待工作完成。
pthread_join() 是 POSIX 线程库提供的一个函数，作用是等待指定的线程执行结束，然后主线程才能继续往下执行。
你可以把它理解为：主线程在这里等待子线程完成任务
假设你创建了三个线程：
    `pthread_create(&tasks[0].tid, NULL, p1, &tasks[0]);`
    `pthread_create(&tasks[1].tid, NULL, p2, &tasks[1]);`
    `pthread_create(&tasks[2].tid, NULL, p3, &tasks[2]);`
也就是：
    `Thread 1`	计算第一段数字之和
    `Thread 2`	计算第二段数字之和
    `Thread 3`	计算第三段数字之和
这三个线程创建后，可以并发执行。
但是主线程并不知道它们什么时候完成，所以需要使用 `pthread_join()`。

## 1-1的实验包目录

```
lab1-1/
├── Makefile
├── README.md          也就是你在看的这个
│
├── student/impl.c     ★ 你要写的文件（三处 TODO，不含 main）
├── tests/harness.c      测例骨架：自带 main，负责输出与校验
├── tests/check.py       测例：检查不变量，而不是比对整段输出
│
├── solution/3.c         完整单文件版答案（含 main），用来看整体结构
└── solution/impl.c      函数版答案，与你写的文件结构一致
```

**为什么 main 在测例里？**
（你可能会困惑：没有主函数怎么跑？）

`student/impl.c` 只包含几个函数，`main` 在 `tests/harness.c` 里。
这样输出格式、线程检查、数值校验都不在你手里，测例才判得准。

## 环境要求

只需要两样东西：

| 组件 | 用途 | 是否必需 |
|---|---|---|
| 支持 pthread 的 C 编译器 | 编译实验代码 | 必需 |
| Python 3.8+ | 运行测例 | 必需 |
| make | 简化命令 | 可选 |
| RISC-V 交叉工具链 + qemu-user | `make riscv` 目标 | 可选 |

**不需要 MSYS2、不需要 WSL、不需要虚拟机。**

### Windows

推荐 **w64devkit**：单个压缩包，解压后含 gcc / make / gdb，把 `bin` 目录加进 PATH 即可。
<https://github.com/skeeto/w64devkit/releases>

也可以用 MSYS2（`pacman -S mingw-w64-ucrt-x86_64-gcc make`）。
Visual Studio 的 MSVC 不支持 `-pthread`，本实验未做适配。

### Linux / macOS

```bash
# Debian / Ubuntu
sudo apt install build-essential python3
# macOS（先装 Xcode Command Line Tools）
xcode-select --install
```

### 检查环境

```bash
python ../tools/check_env.py
```

脚本会实际编译并运行一个 pthread 小例子，确认线程库真的可用，
而不是只看 `gcc --version` 能不能打印。

## 快速开始

```bash
make test-student  # 编译你的实现并运行测例（改完 student/impl.c 后跑这个）
make test          # 编译参考答案并运行测例（用来对照）
```

**如果机器上没有 make**，两条命令同样有效：

```bash
gcc -pthread student/impl.c tests/harness.c -o 3
python tests/check.py 3
```

想看完整单文件程序（含 main）：

```bash
make demo && ./build/3_solution
```

## 测例

不比对完整输出，只检查与实现方式无关的不变量：

| 检查项 | 说明 |
|---|---|
| 三行 `[task]` | 编号必须是 p1/p2/p3 |
| 区间连续且不重叠 | 三段首尾相接，合计覆盖 1..200 |
| 每段小计 | 等于该区间的实际整数和（测例独立计算） |
| `total` | 必须等于 20100 |
| 三个 `tid` | 互不相同且非空，即确实创建并记录了 3 个线程 |
| 连续 5 次运行 | 结果一致，排除数据竞争导致的抖动 |

用任意方式切分区间（不必是给定的），只要满足题意就能通过。

## 常见问题

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
- 起点代码会**编译通过但测例失败**，学生能先看到失败长什么样。
- 输出由测例骨架负责，学生无法通过伪造输出来绕过检查。

## RISC-V（可选）

如果指导老师要求实验支持 RISC-V。同一份源码可以直接交叉编译：

```bash
sudo apt install gcc-riscv64-linux-gnu qemu-user   # WSL / Linux
make riscv
```
代码中没有平台相关的条件编译，x86-64 与 riscv64 共用同一份源码。
