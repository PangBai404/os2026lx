# 欢迎！（Windows 版）

## 实验介绍

操作系统听起来像一堆看不见摸不着的规则？我们从 C 程序里能亲眼观察到的现象开始，一点一点接触。
在你即将动手操作的实验一中，你会学习如何先让多个线程分头计算，再用信号量安排顾客共享椅子；接下来还会处理哲学家争用资源、生产者和消费者共享缓冲区，练习怎样避免死锁、让并发任务配合起来。

在实验二，我们会关注内存的管理应用，我们会动手使用 `mmap()`、`munmap()` 和 `malloc()`，再结合 `fork()` 看看进程如何映射、访问和共享内存。做完这些，你就能把进程、线程、同步和内存管理这些课堂概念，和真正运行起来的 C 程序连在一起。准备好和操作系统打个照面了吗？

> **平台说明**：本原版 lab1 主要面向 Windows（推荐 w64devkit）。Linux 也可直接运行。
> macOS 用户请使用 `lab1forMacOS/lab1-1/README.md` 开始（`lab1forMacOS/` 下有对应的四个实验包）；
> 原包的 lab1-2/1-4 依赖 `sem_init`，macOS 上不可用，lab2 同理~

## 准备工作

如果你已经接触过C语言程序设计了，那么你应该已经有了一套适合你的 IDE。
当然，如果还没有，而且是“小白”的状态，可以参考一下这里的入门建议。

### 除了 C 编译器，还需要两样东西

每个实验都带一个**自动判分的测例脚本**（`tests/check.py`），
它会运行你写好的程序、检查结果，并告诉你哪里不对、该怎么查。
所以除了 C 编译器，你还需要：

| 需要 | 用来做什么 | 是否必需 |
|---|---|---|
| **Python 3.8+** | 运行测例脚本 | 必需 |
| **make** | 用 `make test-student` 一条命令跑测例 | 可选 |

**没有 make 也完全可以做实验**，每个实验的 README 都给了等价的两条命令。

Python 的安装：

- **Windows**：从 <https://www.python.org/downloads/> 下载安装包，
  安装时**务必勾选「Add Python to PATH」**，否则终端里会找不到 `python`。
- **macOS / Linux**：一般已自带。在终端执行 `python3 --version` 确认。

> 注意：有些系统里命令叫 `python3` 而不是 `python`。
> 如果 `python` 报「找不到命令」，把命令换成 `python3` 即可。

### 开始前，先检查一次环境

把需要完成的软件都装好之后，建议先运行一次环境检查。脚本在 `lab1/tools/` 下（macOS 用 `lab1forMacOS/tools/`），
你需要先进入对应的实验包目录再执行：

```bash
cd lab1          # macOS 用户改为 cd lab1forMacOS
python tools/check_env.py
```

它会**真的编译并运行一个多线程小程序**，确认你的编译器、线程库、Python 都能正常工作，
而不是只看版本号能不能打印。看到 `Result: environment ready.` 就可以开始了。

如果它报某一项缺失，会直接告诉你缺什么、Windows 上该装哪个。

> 如果你已完成本部分，请前往 `lab1/lab1-1/`（macOS 用 `lab1forMacOS/lab1-1/`），
> 阅读其中的 `README.md` 以继续~

### Windows

如果你的 Windows 设备已经有配置好的IDE了，可以跳过这部分或略读，注意，Windows 在实验二可能不能原生完成任务。
适合 Windows 的IDE有很多，大学老师一般都会让装 Code::Blocks。然而大家可能更倾向于用 VS Code，这都没关系，不影响实验一的完成。
不过，到了实验二，Windows 就不能直接完成了，你可能要换用 Ubuntu 虚拟机（推荐，有图形界面）或 WSL2，或者 MacOS(如果你手头有) 来继续。这是因为实验二涉及的部分函数 Windows 没法直接用（不是没用，只是长得不一样，如果为了让 Windows 原生运行强行硬套，就改变了实验原本的技术内容。）

Code::Blocks 有集成的 C 程序开发环境，你创建好项目后直接编译并运行。 VS Code 是个代码编辑器，它默认没有专门的 C 语言 Build & Run 按钮，需要配置扩展或任务，用 GCC 来完成。想要有一个看起来很安心地 Build & Run 按钮，你可以安装一个 Code Runner（单个C程序）、C/C++ 扩展 + Build Task 扩展（或其它适合的扩展，或者直接用命令）
跑一个简单的程序看看行不行：

```bash
#include <stdio.h>

int main(void)
{   
    printf("Hello, World!\n");
    return 0;
}
```
编译：`gcc -Wall -Wextra -std=c11 hello.c -o hello.exe`
运行：`.\hello.exe`
（或者直接点按钮 Build & Run，如果有。）

> 如果你已完成本部分，请前往 `lab1/lab1-1/`，
> 阅读其中的 `README.md` 以继续~

### MacOS

如果你的 Mac 设备已经有配置好的IDE了，可以跳过这部分。
在 Mac 设备上编写C程序，需要用到 clang。有两种途径安装：

- 在 App Store 里面搜索苹果官方的 Xcode ，里面自带 clang。
这样做一步到位。但是编者认为如果你只用它做这一个实验的话，它的体量有点太大了，Xcode 更多的是用来开发 iOS App 这样的大项目的。如果你已经习惯 VS Code 这样的IDE，就没必要为了实验二安装完整 Xcode。

- 使用 MacOS 版的 VS Code 并装配 clang。

#### 安装 Clang
1. 打开 Mac 的「终端」（Terminal）。
2. 运行：`xcode-select --install`
3. 系统会弹出安装窗口，选择「安装」，等待安装完成。

#### 检查 Clang 是否安装成功
在终端执行：`clang --version`
如果看到类似：`Apple clang version ...` , `Target: arm64-apple-darwin...` 说明安装成功。
再执行：`which clang` ,通常会显示 `/usr/bin/clang`

#### 配置 VS Code
打开 VS Code，安装两个扩展
- C/C++（Microsoft 官方扩展，提供代码补全、语法分析、调试支持。）
- Code Runner（可选，可以快速运行代码，但后续操作系统实验建议优先使用终端手动编译。）

验证是否可以跑通，写一段C程序，比如

```
#include <stdio.h>
#include <unistd.h>
int main() {
    printf("Hello macOS!\n");
    return 0;}
```

在 VS Code 中打开集成终端（菜单栏 → Terminal → New Terminal。）
输入：`clang -Wall -Wextra -std=c11 main.c -o main`
然后：`./main`
预期运行结果是打印出 `Hello macOS!`

> 如果你已完成本部分，请前往 `lab1forMacOS/lab1-1/` ，
> 阅读其中的 `README.md` 以继续~

### Linux 虚拟设备

你可以在现有系统里创建一台 Linux 虚拟机。这里以 VirtualBox + Ubuntu 为例；使用 VMware 等虚拟机软件也可以，步骤大致相同。实验二会用到 Linux 下的系统接口，建议安装 Ubuntu 桌面版，并在虚拟机里完成编译和运行。当然，用WSL2 也行，只是它没有图形界面，初次接触可能会不适应。

#### 创建 Ubuntu 虚拟机

1. 安装 VirtualBox，并从 Ubuntu 官网下载 Ubuntu 桌面版 ISO 镜像。
2. 在 VirtualBox 中新建虚拟机，类型选择 Linux、版本选择 Ubuntu（64-bit），把 ISO 镜像作为启动盘。
3. 为虚拟机分配至少 2 个 CPU 核心和 4 GB 内存；磁盘建议 25 GB 以上。资源充足时可以适当增加。
4. 启动虚拟机，按安装向导完成 Ubuntu 安装。安装完成后弹出 ISO 时，移除虚拟光盘并重新启动虚拟机。
当然，使用 VM 虚拟机是完全可以的，本实验不需要它有多么强劲的性能和多么夸张的配置。

#### 安装 C 编译工具

打开 Ubuntu 的「终端」（Terminal），更新软件包列表并安装 GCC 和常用开发工具：

```bash
sudo apt update
sudo apt install build-essential
```

检查 GCC 是否安装成功：

```bash
gcc --version
```

如果能看到 GCC 的版本信息，就可以继续。`build-essential` 会安装 GCC、Make 等常用构建工具。

#### 使用 VS Code（可选）

可以直接在 Ubuntu 中使用文本编辑器和终端；如果希望使用 VS Code，可以安装 Linux 版 VS Code，并安装 Microsoft 的 C/C++ 扩展。请在 VS Code 中打开存放代码的文件夹，再通过「Terminal → New Terminal」打开终端编译和运行程序。虚拟机内的终端与宿主机的终端是不同的，实验代码和命令都应在 Ubuntu 虚拟机里执行。

#### 编译并运行程序

新建 `hello.c`，写入下面的程序：

```c
#include <stdio.h>

int main(void)
{
    printf("Hello, Linux!\n");
    return 0;
}
```

在终端切换到 `hello.c` 所在目录，然后执行：

```bash
gcc -Wall -Wextra -std=c11 hello.c -o hello
./hello
```

如果终端打印出 `Hello, Linux!`，说明编译环境已经准备好。后续实验需要用到 Linux 系统接口时，也请在这台虚拟机中编译和运行。

## 开始做实验！

### 实验包在哪

| 你的设备 | lab1（四个任务） | lab2（内存管理） |
|---|---|---|
| Windows / Linux | `lab1/lab1-1/` ~ `lab1/lab1-4/` | `lab2/` |
| macOS | `lab1forMacOS/lab1-1/` ~ `lab1forMacOS/lab1-4/` | `lab2forMacOS/` |

每个实验包里都有一个 `README.md`，从那里开始读。

> 先前往 lab1 的第一个任务 `lab1/lab1-1/README.md`（macOS 用 `lab1forMacOS/lab1-1/README.md`）以继续~
>
> 实验一做完后前往实验二 `lab2/README.md`（macOS 用 `lab2forMacOS/README.md`）。
> 实验二需要 `fork()` / `mmap()` 等 POSIX 接口，Windows 原生做不到。
> 可以使用 Ubuntu 虚拟机（推荐，有图形界面，适合初学者）或 WSL2，
> 也可以直接在 Linux 或 macOS 上完成。