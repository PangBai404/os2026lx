#!/usr/bin/env python3
"""
实验一 · 任务 2 的测例。

用法：
    python3 tests/check.py                       # 测 build/4_harness（答案）
    python3 tests/check.py build/4_student       # 测学生提交
    python3 tests/check.py build/4_solution      # 测完整单文件版（仅看能否跑通）

判据（不比对完整输出，只检查不变量）：
    1. 10 位顾客全部完成，没有死锁（能正常退出且不超过 30 秒）；
    2. 同时在座人数从未超过 2（violations 必须为 0）；
    3. 同时在座的最大人数恰好是 2（不是 1，说明两把椅子都用上了）；
    4. 连续运行多次结果一致。
"""

import re
import subprocess
import sys
from pathlib import Path

CUSTOMERS = 10
CHAIRS = 2
ROOT = Path(__file__).resolve().parent.parent

NONCE_RE = re.compile(r"^nonce=([0-9a-f]{8})$")
KV_RE = re.compile(r"^(\w+)=(\S+)$")


def find_binary(arg):
    if arg:
        p = Path(arg)
        if not p.is_absolute():
            p = ROOT / p
    else:
        p = ROOT / "build" / "4_harness"

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
        return None, "timeout"

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

    result, nonce = run_once(binary)
    if result == "timeout":
        return ["timeout after 30s (possible deadlock)"], "", "timeout"

    rc, kv, out, err = result
    mode = "harness (nonce)" if nonce else "standalone (no nonce)"

    if rc != 0:
        failures.append(f"exit code {rc} (expected 0)")
    if err.strip():
        failures.append(f"stderr not empty: {err.strip()[:200]}")

    # 1. 全部完成
    served = kv.get("served")
    if served is None:
        failures.append("missing served=")
    elif int(served) != CUSTOMERS:
        failures.append(f"served should be {CUSTOMERS}, got {served}")

    # 2. 不超员
    viol = kv.get("violations")
    if viol is None:
        failures.append("missing violations=")
    elif int(viol) != 0:
        failures.append(
            f"{viol} customer(s) sat down while all {CHAIRS} chairs were taken "
            f"(the semaphore is not limiting the room)"
        )

    # 3. 两把椅子都用上了
    peak = kv.get("peak_seats")
    if peak is None:
        failures.append("missing peak_seats=")
    else:
        peak = int(peak)
        if peak > CHAIRS:
            failures.append(f"peak_seats should never exceed {CHAIRS}, got {peak}")
        elif peak < CHAIRS:
            failures.append(
                f"only {peak} chair was ever in use at the same time "
                f"(expected {CHAIRS}) - check the initial value passed to sem_init"
            )

    # 4. 多次运行稳定
    for _ in range(5):
        r2, _ = run_once(binary)
        if r2 == "timeout":
            failures.append("unstable across runs: timeout on a later run")
            break
        _, kv2, _, _ = r2
        if kv2.get("served") != kv.get("served") or kv2.get("peak_seats") != kv.get("peak_seats"):
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
