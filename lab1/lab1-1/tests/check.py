#!/usr/bin/env python3
import re
import subprocess
import sys
from pathlib import Path

TOTAL = 200
EXPECT = TOTAL * (TOTAL + 1) // 2
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
                continue
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

    if len(tasks) != 3:
        failures.append(f"expected 3 [task] lines, got {len(tasks)}")

    if tasks:
        ids = [t[0] for t in tasks]
        if sorted(ids) != [1, 2, 3]:
            failures.append(f"thread ids should be 1,2,3, got {ids}")

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

        for tid, start, end, s, _ in tasks:
            want = (start + end) * (end - start + 1) // 2
            if s != want:
                failures.append(f"p{tid} sum wrong: {start}..{end} should be {want}, got {s}")

        tids = [t[4] for t in tasks if t[4]]
        if nonce:
            if len(tids) != 3:
                failures.append("missing tid= on [task] lines")
            else:
                if len(set(tids)) != 3:
                    failures.append(f"tids not distinct: {tids}")
                if any(int(t, 16) == 0 for t in tids):
                    failures.append("a tid is zero (run_tasks did not store it)")

    if "total" not in kv:
        failures.append("missing total=")
    elif int(kv["total"]) != EXPECT:
        failures.append(f"total should be {EXPECT}, got {kv['total']}")

    if "distinct_threads" in kv and kv["distinct_threads"] != "yes":
        failures.append("threads not distinct (distinct_threads != yes)")

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
