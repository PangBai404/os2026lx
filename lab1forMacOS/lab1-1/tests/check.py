#!/usr/bin/env python3
"""
实验一 · 任务 1 的测例。

用法：
    python3 tests/check.py                       # 测 build/3_harness（答案）
    python3 tests/check.py build/3_student       # 测学生提交
    python3 tests/check.py build/3_solution      # 测完整单文件版

两种输出格式都支持：
  * 带 nonce 前缀的（tests/harness.c 生成）—— 学生无法伪造，推荐用于判分
  * 不带前缀的（solution/3.c 完整程序）—— 仅用于演示和自测

判据（不比对完整输出，只检查不变量）：
    1. 打印了 3 行 [task]，编号分别为 p1/p2/p3；
    2. 三段区间首尾相接，合起来正好覆盖 1..200，互不重叠；
    3. 每段 sum 等于该区间的实际整数和（由本脚本独立计算）；
    4. total 等于 20100；
    5. 三个 tid 互不相同且非空（确实跑了 3 个线程）；
    6. 连续运行多次结果一致（没有数据竞争导致的抖动）。
"""

import re
import subprocess
import sys
from pathlib import Path

TOTAL = 200
EXPECT = TOTAL * (TOTAL + 1) // 2     # 20100
ROOT = Path(__file__).resolve().parent.parent

NONCE_RE = re.compile(r"^nonce=([0-9a-f]{8})$")
TASK_RE = re.compile(
    r"^\[task\]\s+p(\d+)\s+range=(\d+)\.\.(\d+)\s+sum=(-?\d+)"
    r"(?:\s+tid=([0-9a-f]+))?$"
)
KV_RE = re.compile(r"^(\w+)=(\S+)$")


def find_binary(arg):
    if arg:
        p = Path(arg)
        if not p.is_absolute():
            p = ROOT / p
    else:
        p = ROOT / "build" / "3_harness"

    for cand in (p, p.with_name(p.name + ".exe")):
        if cand.exists():
            return cand
    raise SystemExit(
        f"Binary not found: {p}\n"
        f"Run \"make test\" first, or pass a built executable explicitly."
    )


def run_once(binary):
    proc = subprocess.run(
        [str(binary)],
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
        timeout=30,
    )

    lines = proc.stdout.splitlines()
    nonce = None
    for line in lines:
        m = NONCE_RE.match(line.strip())
        if m:
            nonce = m.group(1)
            break

    tasks, kv = [], {}
    for line in lines:
        line = line.strip()
        if nonce:
            if not line.startswith(nonce):
                continue                    # 无本次随机前缀的行一律忽略
            line = line[len(nonce):].strip()

        m = TASK_RE.match(line)
        if m:
            tasks.append((
                int(m.group(1)), int(m.group(2)),
                int(m.group(3)), int(m.group(4)),
                m.group(5),
            ))
            continue
        m = KV_RE.match(line)
        if m:
            kv[m.group(1)] = m.group(2)

    return proc.returncode, tasks, kv, nonce, proc.stdout, proc.stderr


def check(binary):
    failures = []

    rc, tasks, kv, nonce, out, err = run_once(binary)

    mode = "harness (nonce)" if nonce else "standalone (no nonce)"

    if rc != 0:
        failures.append(f"exit code {rc} (expected 0)")
    if err.strip():
        failures.append(f"stderr not empty: {err.strip()[:200]}")

    # 1. 三个线程
    if len(tasks) != 3:
        failures.append(f"expected 3 [task] lines, got {len(tasks)}")

    if tasks:
        ids = [t[0] for t in tasks]
        if sorted(ids) != [1, 2, 3]:
            failures.append(f"thread ids should be 1,2,3, got {ids}")

        # 2. 区间完整、不重叠、首尾相接
        ordered = sorted(tasks, key=lambda t: t[1])
        cursor = 1
        for _, start, end, _, _ in ordered:
            if start != cursor:
                failures.append(f"gap in ranges: expected start {cursor}, got {start}")
                break
            if end < start:
                failures.append(f"invalid range {start}..{end}")
                break
            cursor = end + 1
        if cursor - 1 != TOTAL:
            failures.append(f"ranges must cover 1..{TOTAL}, stopped at {cursor-1}")

        # 3. 每段小计（本脚本独立计算，不信任学生程序）
        for tid, start, end, s, _ in tasks:
            want = (start + end) * (end - start + 1) // 2
            if s != want:
                failures.append(f"p{tid} sum wrong: {start}..{end} should be {want}, got {s}")

        # 5. tid 互不相同（仅 harness 模式会输出 tid）
        tids = [t[4] for t in tasks if t[4]]
        if nonce:
            if len(tids) != 3:
                failures.append("missing tid= on [task] lines")
            else:
                if len(set(tids)) != 3:
                    failures.append(f"tids not distinct: {tids}")
                if any(int(t, 16) == 0 for t in tids):
                    failures.append("a tid is zero (run_tasks did not store it)")

    # 4. 总计
    if "total" not in kv:
        failures.append("missing total=")
    elif int(kv["total"]) != EXPECT:
        failures.append(f"total should be {EXPECT}, got {kv['total']}")

    # 汇总自检（仅 standalone 模式有 distinct_threads 字段）
    if "distinct_threads" in kv and kv["distinct_threads"] != "yes":
        failures.append("threads not distinct (distinct_threads != yes)")

    # 6. 多次运行结果稳定
    for _ in range(5):
        rc2, _, kv2, _, _, _ = run_once(binary)
        if kv2.get("total") != kv.get("total") or rc2 != rc:
            failures.append("unstable across runs (possible data race)")
            break

    return failures, out, mode


def main():
    binary = find_binary(sys.argv[1] if len(sys.argv) > 1 else None)

    failures, out, mode = check(binary)

    print(f"Testing: {binary}")
    print(f"Mode   : {mode}")
    print("-" * 50)
    print(out.rstrip())
    print("-" * 50)

    if failures:
        print(f"RESULT: FAILED ({len(failures)} problem(s))")
        for i, f in enumerate(failures, 1):
            print(f"  {i}. {f}")
        return 1

    print("RESULT: PASSED")
    return 0


if __name__ == "__main__":
    sys.exit(main())
