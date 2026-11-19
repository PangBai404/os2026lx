#!/usr/bin/env python3
"""
实验一 · 任务 3 的测例。

用法：
    python tests/check.py                       # 测 build/9_harness（答案）
    python tests/check.py build/9_student       # 测学生提交
    python tests/check.py build/9_solution      # 测完整单文件版（仅看能否跑通）

判据（不比对完整输出，只检查不变量）：
    1. 25 顿饭全部吃完，没有死锁（30 秒内退出）；
    2. 每次申请都满足资源顺序：先申请编号小的叉子；
    3. 每次吃饭时都握着两把叉子，每次释放的都是自己持有的叉子；
    4. 连续运行多次结果一致。
"""

import re
import subprocess
import sys
from pathlib import Path

NPHIL = 5
MEALS = 5
EXPECT_EATEN = NPHIL * MEALS
ROOT = Path(__file__).resolve().parent.parent

NONCE_RE = re.compile(r"^nonce=([0-9a-f]{8})$")
KV_RE = re.compile(r"^(\w+)=(\S+)$")


def find_binary(arg):
    if arg:
        p = Path(arg)
        if not p.is_absolute():
            p = ROOT / p
    else:
        p = ROOT / "build" / "9_harness"

    for cand in (p, p.with_name(p.name + ".exe")):
        if cand.exists():
            return cand
    raise SystemExit(
        f"Binary not found: {p}\n"
        f"Run \"make test\" first, or pass a built executable explicitly."
    )


def run_once(binary):
    try:
        proc = subprocess.run(
            [str(binary)],
            capture_output=True,
            text=True,
            encoding="utf-8",
            errors="replace",
            timeout=30,
        )
    except subprocess.TimeoutExpired:
        return None

    lines = proc.stdout.splitlines()
    nonce = None
    for line in lines:
        m = NONCE_RE.match(line.strip())
        if m:
            nonce = m.group(1)
            break

    kv = {}
    for line in lines:
        line = line.strip()
        if nonce:
            if not line.startswith(nonce):
                continue
            line = line[len(nonce):].strip()

        m = KV_RE.match(line)
        if m:
            kv[m.group(1)] = m.group(2)

    return (proc.returncode, kv, proc.stdout, proc.stderr), nonce


def check(binary):
    failures = []

    result = run_once(binary)
    if result is None:
        return ["timeout after 30s (deadlock?) - check the fork acquisition order"], "", "timeout"

    (rc, kv, out, err), nonce = result
    mode = "harness (nonce)" if nonce else "standalone (no nonce)"

    if rc != 0:
        failures.append(f"exit code {rc} (expected 0)")
    if err.strip():
        failures.append(f"stderr not empty: {err.strip()[:200]}")

    # 1. 全部吃完
    eaten = kv.get("eaten")
    if eaten is None:
        failures.append("missing eaten=")
    elif int(eaten) != EXPECT_EATEN:
        failures.append(
            f"eaten should be {EXPECT_EATEN}, got {eaten} "
            f"- every philosopher must finish {MEALS} meals"
        )

    # 2. 资源顺序分配
    order = kv.get("order_violations")
    if order is None:
        failures.append("missing order_violations=")
    elif int(order) != 0:
        example = kv.get("order_example", "none")
        msg = (
            f"{order} fork acquisition(s) in descending order"
            f" - resource ordering requires taking the lower-numbered fork first"
        )
        if example != "none":
            msg += f" (first example: {example})"
        failures.append(msg)

    # 3. 持叉与释放
    hold = kv.get("hold_violations")
    if hold is None:
        failures.append("missing hold_violations=")
    elif int(hold) != 0:
        failures.append(
            f"{hold} meal(s) started without holding both forks "
            f"- check that lab_meal() is called after acquiring BOTH forks"
        )

    rel = kv.get("release_violations")
    if rel is None:
        failures.append("missing release_violations=")
    elif int(rel) != 0:
        failures.append(
            f"{rel} fork(s) released by a philosopher that did not hold them "
            f"- check the fork ids passed to lab_fork_release()"
        )

    # 4. 多次运行稳定
    for _ in range(4):
        r2 = run_once(binary)
        if r2 is None:
            failures.append("unstable across runs: timeout on a later run")
            break
        (_, kv2, _, _), _ = r2
        if kv2.get("eaten") != kv.get("eaten"):
            failures.append("unstable across runs")
            break

    return failures, out, mode


def main():
    binary = find_binary(sys.argv[1] if len(sys.argv) > 1 else None)

    failures, out, mode = check(binary)

    print(f"Testing: {binary}")
    print(f"Mode   : {mode}")
    print("-" * 50)
    if out:
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
