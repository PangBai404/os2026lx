# 实验一 · 任务 1：三线程累加
#
#   make test          编译「答案 + 测例骨架」并运行测例      ← 判分入口
#   make test-student  编译「学生实现 + 测例骨架」并运行测例  ← 学生自测
#   make demo          编译完整单文件版答案（便于阅读）
#   make riscv         交叉编译并在 qemu-user 下运行
#   make clean
#
# 说明：判分用 student/impl.c + tests/harness.c，main 在测例侧，
#       学生改不到输出格式；solution/ 下的 3.c 是完整程序，
#       只用于阅读和演示，不参与判分。

# make 内置 CC 默认为 cc；MSYS2 下 cc 包装器可能异常，这里显式用 gcc。
# 需要换编译器时：make CC=clang
CC      := gcc
CFLAGS  ?= -Wall -Wextra -O2
BUILD   := build

# Windows/MSYS2 下 make 可能不传递 TMP/TEMP，gcc 会退回到 C:\WINDOWS 建临时文件而失败。
# 显式指定一个可写的临时目录。
TMPROOT := $(CURDIR)/$(BUILD)/tmp
export TMPDIR := $(TMPROOT)
export TMP    := $(TMPROOT)
export TEMP   := $(TMPROOT)

PTHREAD := -pthread
# macOS / 部分 Linux 上只有 python3，没有 python。自动探测。
PYTHON ?= $(shell command -v python3 2>/dev/null || command -v python 2>/dev/null || echo python)

.PHONY: all test test-student demo riscv clean

all: test demo

$(BUILD):
	@mkdir -p $(BUILD)/tmp

# ---------- 判分用：学生实现 + 测例骨架 ----------

$(BUILD)/3_harness: $(BUILD) solution/impl.c tests/harness.c
	$(CC) $(CFLAGS) $(PTHREAD) solution/impl.c tests/harness.c -o $@

$(BUILD)/3_student: $(BUILD) student/impl.c tests/harness.c
	$(CC) $(CFLAGS) $(PTHREAD) student/impl.c tests/harness.c -o $@

test: $(BUILD)/3_harness
	@$(PYTHON) tests/check.py $(BUILD)/3_harness

test-student: $(BUILD)/3_student
	@$(PYTHON) tests/check.py $(BUILD)/3_student

# ---------- 阅读用：完整单文件程序 ----------

demo: $(BUILD)
	$(CC) $(CFLAGS) $(PTHREAD) solution/3.c -o $(BUILD)/3_solution

# ---------- RISC-V ----------

riscv:
	@command -v riscv64-linux-gnu-gcc >/dev/null 2>&1 || { \
		echo "未找到 riscv64-linux-gnu-gcc。安装：sudo apt install gcc-riscv64-linux-gnu qemu-user"; exit 1; }
	@command -v qemu-riscv64 >/dev/null 2>&1 || { \
		echo "未找到 qemu-riscv64。安装：sudo apt install qemu-user"; exit 1; }
	@mkdir -p $(BUILD)
	riscv64-linux-gnu-gcc $(CFLAGS) $(PTHREAD) solution/impl.c tests/harness.c -o $(BUILD)/3_riscv
	@echo "--- 在 qemu-user 下运行 ---"
	qemu-riscv64 $(BUILD)/3_riscv

clean:
	@rm -rf $(BUILD)
	@echo "已清理 $(BUILD)/"
