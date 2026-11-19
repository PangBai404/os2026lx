# 实验一 · 任务 2：信号量休息厅

## 实验说明

> 「一个休息厅里面顾客比椅子多，肯定有人坐不下来，肯定有人争着抢着坐。」

我们用这个情景来实操学习，体会一下信号量的作用。

欢迎来到实验1-2，信号量休息厅。
在这里，我们的目标是：
一家休息厅只有 2 把椅子，10 位顾客先后到来——有空椅子就坐下休息，没有就等着；休息完让出椅子离开。
你要用信号量把同时在座的人数限制在 2 人。

你已经在学校的《操作系统》课程里面学过了信号量相关知识了，现在我们要通过这个实验真正开始使用P操作、V操作和信号量。
本实验需要用到 POSIX pthread 里面PV操作对应的函数：
    sem_wait()	P 操作	顾客申请椅子
    sem_post()	V 操作	顾客归还椅子

1. 如果你已经在 1-1 的时候配置好环境没问题了，就继续用。如果需要配置或更正实验的环境，参见“实验包目录”，“环境要求”和“常见问题”这些部分来获得帮助。
2. 如果你不是很清楚要怎么做，这里和代码里都会有详细的提示告诉你怎么做。

PS：你可以打开 VSCode 或者其它IDE，可以用分窗口的方式将这个文件和代码同时展示，这样使用效果更佳。

## 本部分实验教程

### 实验准备

请查看“实验包目录”，“环境要求”和“常见问题”

### 1-2的实验任务

本实验中，你需要了解信号量的创建和使用。
你要写的是 `student/impl.c`，里面有**三处 TODO**：

| TODO | 函数 | 要写什么 |
|---|---|---|
| TODO 1 | `room_init` | `sem_init`，初值为 `chairs` |
| TODO 2 | `room_acquire` | `sem_wait`，没有空位就阻塞 |
| TODO 3 | `room_release` | `sem_post`，让出椅子并唤醒等待者 |

如果不加任何同步措施，10 位顾客会一起"坐下"，椅子显然不够用——
这正是本实验要解决的问题。`solution/4.c` 是加了信号量之后的完整程序，
可以对照着看整体结构。

理解下述关键点：

1. 信号量的初值就是「最多几个人能同时坐下」这件事本身，不需要再额外写 `if` 去数人数。
2. `sem_wait` 在椅子上没有空位时会让线程阻塞，这正是"等椅子"的语义；`sem_post` 让出一把椅子并唤醒等待者。
3. 顾客的「坐下」和「离开」由测例记录，你只需要管好取椅子和还椅子这两件事。

### 本部分知识详解

你看到这段实验代码可能会些发懵，这里给你讲一下这些代码做了些什么：
给定代码会创建10个线程：

```bash
#define CUSTOMERS 10
pthread_t th[CUSTOMERS];
```

然后通过循环创建线程：

```bash
    for (long i = 0; i < CUSTOMERS; i++) {
        int rc = pthread_create(&th[i], NULL, customer, (void *)i);
    }
```

每个线程执行同一个函数：` void *customer(void *arg) `

    sem_t 是 POSIX 信号量库提供的数据类型。
    初始化信号量，用下面这个函数
        int sem_init(sem_t *sem, int pshared, unsigned int value);
        （信号量变量的地址，线程之间的共享，信号量的初始值）
    pthread_t	线程 ID 类型
    pthread_create()	创建线程
    pthread_join()	等待线程结束
    customer()	线程执行的任务函数

注意：10个线程执行的是同一个函数，但是通过arg参数区分顾客编号。

你要做的是用你在课上学到的PV操作来操作信号量，信号量是一种用于线程同步和资源管理的机制，你可以把它理解成一个资源计数器。
P操作：申请资源，在 POSIX pthread 环境中对应：
    `sem_wait(&semaphore);`
    如果信号量大于 0，就将其减 1，继续执行。
    如果信号量等于 0，就阻塞等待，直到有资源释放。
V操作：释放资源，也就是
    `sem_post(&semaphore);`
    将信号量加 1。
    如果有线程正在等待，可能唤醒其中一个线程。

那么，在本实验情境中，椅子便是资源，你需要先声明一个信号量。
在 `student/impl.c` 里，`sem_t` 类型的变量已经声明好了（`static sem_t chairs_sem;`），
这一行不用你写。

对于 `room_init`（TODO 1）的信号量初始化要求，要用到
`sem_init(sem_t *sem, int pshared, unsigned int value);`
它有三个参数：
    第一个	&chairs	信号量变量的地址
    第二个	0	表示在线程之间共享
    第三个	CHAIRS	信号量的初始值
1. 第一个参数：取地址。
想想怎样取你声明好的信号量的地址呢？
    为什么需要传地址？因为 sem_init() 需要知道应该初始化哪个信号量对象。
2. 第二个参数：int pshared
这个参数比较重要,他决定信号量用于同一个进程内的多个线程之间共享还是进程之间共享，也就是一个共享的范围问题。它有两种取值：
    0：线程间共享。
    非 0：进程间共享（通常要求信号量位于多个进程都能访问的共享内存中）。
我们的实验是 10 个顾客线程都属于同一个进程，想想填哪个？
3. 第三个参数：初值
我们有几把椅子？（提示：可以找一下代码里定义好的常量~）

对于 `room_acquire`（TODO 2）和 `room_release`（TODO 3），
判断要执行P、V中的哪个操作，然后用对应的函数完成就行。

## 实验包目录

```
lab1-2/
├── Makefile
├── README.md  也就是你在看的这个
│
├── student/impl.c     ★ 你要写的文件（三处 TODO，不含 main）
├── tests/harness.c     测例骨架：自带 main，创建顾客并统计同时就座人数
├── tests/check.py   测例：检查不变量，而不是比对整段输出
│
├── solution/4.c    完整单文件版答案（含 main），用来看整体结构
└── solution/impl.c   函数版答案，与你写的文件结构一致
```

**为什么 main 在测例里？**
（你可能会困惑：没有主函数怎么跑？）

`student/impl.c` 只包含三个函数，`main` 在 `tests/harness.c` 里。
这样「顾客什么时候坐下、什么时候离开、同时有几个人在座」都由测例统计，
你不碰这些代码，测例才判得准。

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
gcc -pthread student/impl.c tests/harness.c -o 4
python tests/check.py 4
```

想看完整单文件程序（含 main）：

```bash
make demo && ./build/4_solution
```

## 测例判什么

测例会创建 10 位顾客，每位坐下后休息 150 毫秒再离开。
不比对完整输出，只检查与实现方式无关的不变量：

| 检查项 | 说明 |
|---|---|
| `served` | 必须等于 10，即没有顾客被饿死，也没有死锁 |
| `violations` | 必须为 0，即从没有第 3 位顾客在椅子满员时坐下 |
| `peak_seats` | 必须恰好是 2，说明两把椅子确实都在用 |
| 连续 5 次运行 | 结果一致 |
| 30 秒超时 | 死锁时直接判失败，并提示可能是死锁 |

`peak_seats` 恰好为 2 这一条是有意加严的：如果有人用互斥锁代替信号量，
同时在座的永远只有 1 人，虽然"不超过 2"却浪费了一把椅子。
测例会明确提示这一点。

### 为什么输出不能被伪造

`main` 在 `tests/harness.c` 里，你碰不到。每次运行测例都会生成一个随机前缀
（`nonce=xxxxxxxx`），所有有效输出行都带着它；测例只认带本次前缀的行。
顾客的坐下与离开由测例自己统计，所以「直接把结果 printf 出来」不会生效。

## 常见问题

**程序卡住不退出**

多半是死锁。常见原因：`sem_wait` 调了几次 `sem_post` 却没调，或者初值设成了 0。
测例会在 30 秒后超时，并提示可能是死锁。

**`peak_seats=1`，测例说「只有 1 把椅子被同时使用」**

检查 `sem_init` 的第三个参数是不是 `chairs`（也就是 2），而不是 1。
如果你用的是互斥锁而不是信号量，同时在座的永远只有 1 人。

**`violations` 大于 0，测例说「椅子满了还有顾客坐下」**

检查是不是漏了 `sem_wait`，或者把它放到了打印之后的位置——
必须在坐下之前就申请到椅子。

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
- 顺带检查了「两把椅子确实都用上了」，这是只看最终结果无法发现的浪费。

## RISC-V（可选）

如果指导老师要求实验支持 RISC-V。同一份源码可以直接交叉编译：

```bash
sudo apt install gcc-riscv64-linux-gnu qemu-user   # WSL / Linux
make riscv
```

代码中没有平台相关的条件编译，x86-64 与 riscv64 共用同一份源码。
