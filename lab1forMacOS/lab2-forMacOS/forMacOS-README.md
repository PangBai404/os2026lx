# 实验二 · 内存管理（macOS）

> **本文件仅为路径提示。** 完整教程不在这个文件夹里，请按下面的说明前往。

## 去哪里读

实验二的完整教程位于本仓库**根目录**的 `lab2forMacOS/` 文件夹：

| 内容 | 路径 |
|---|---|
| 教程与任务说明 | `lab2forMacOS/README.md` |
| 学生要写的文件 | `lab2forMacOS/student/impl.c` |
| 编译与判分 | 在该目录下执行 `make test-student` |

## 为什么实验二需要单独的版本

实验二使用 `mmap()`、`munmap()`、`fork()`、`wait()` 等 POSIX 接口，
原生 Windows 没有对应实现。

> PS: 这不是因为 Windows 功能少，Windows 有自己的内存映射文件和进程创建 API，
> 例如 `CreateFileMapping()`、`MapViewOfFile()` 和 `CreateProcess()`，只是接口与 POSIX 不一样。
> 实验二不是普通的 C 语言算法练习，而是操作系统接口实验。目的在于使用操作系统提供的系统调用，
> 理解内存映射、进程创建和进程间通信。如果为了让 Windows 原生运行而把 `fork()` 改成 `CreateProcess()`，
> 就改变了实验原本的技术内容。

## 各平台该用哪个实验包

| 你的环境 | 使用哪个实验二 |
|---|---|
| Linux | `lab2/` |
| **Ubuntu 虚拟机**（VirtualBox / VMware） | `lab2/`，Windows 上的推荐方案，有图形界面 |
| **WSL2** | `lab2/`，代码放在 Linux 侧（如 `~/lab2`），**不要**放在 `/mnt/c` 或 `/mnt/e` |
| **macOS** | `lab2forMacOS/` |
