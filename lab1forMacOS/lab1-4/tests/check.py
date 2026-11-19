#!/usr/bin/env python3
import re
import subprocess
import sys
from pathlib import Path

PRODUCERS = 5
CONSUMERS = 5
ITEMS_PER_PRODUCER = 20
TOTAL_ITEMS = PRODUCERS * ITEMS_PER_PRODUCER
BUFSIZE = 8
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
        return (["timeout after 30s (deadlock?) - check the semaphore waits"],
                "", "timeout")

    (rc, kv, out, err), nonce = result
    mode = "harness (nonce)" if nonce else "standalone (no nonce)"

    if rc != 0:
        failures.append(f"exit code {rc} (expected 0)")
    if err.strip():
        failures.append(f"stderr not empty: {err.strip()[:200]}")

    for key in ("produced", "consumed"):
        v = kv.get(key)
        if v is None:
            failures.append(f"missing {key}=")
        elif int(v) != TOTAL_ITEMS:
            failures.append(f"{key} should be {TOTAL_ITEMS}, got {v}")

    of = kv.get("overflow")
    if of is None:
        failures.append("missing overflow=")
    elif int(of) != 0:
        failures.append(
            f"{of} item(s) pushed into a full buffer "
            f"- the producer must wait on the empty semaphore (capacity {BUFSIZE})"
        )

    uf = kv.get("underflow")
    if uf is None:
        failures.append("missing underflow=")
    elif int(uf) != 0:
        failures.append(
            f"{uf} item(s) popped from an empty buffer "
            f"- the consumer must wait on the full semaphore"
        )

    peak = kv.get("peak_occupancy")
    if peak is not None and int(peak) > BUFSIZE:
        failures.append(f"peak_occupancy should never exceed {BUFSIZE}, got {peak}")

    dup = kv.get("duplicates")
    if dup is None:
        failures.append("missing duplicates=")
    elif int(dup) != 0:
        failures.append(
            f"{dup} duplicate product(s) consumed "
            f"- check the mutex around the buffer operations"
        )

    miss = kv.get("missing")
    if miss is None:
        failures.append("missing missing=")
    elif int(miss) != 0:
        failures.append(f"{miss} product(s) never consumed")

    unexp = kv.get("unexpected")
    if unexp is None:
        failures.append("missing unexpected=")
    elif int(unexp) != 0:
        failures.append(
            f"{unexp} consumed value(s) have an invalid product id"
        )

    left = kv.get("leftover")
    if left is None:
        failures.append("missing leftover=")
    elif int(left) != 0:
        failures.append(f"{left} item(s) left in the buffer at the end")

    for _ in range(4):
        r2 = run_once(binary)
        if r2 is None:
            failures.append("unstable across runs: timeout on a later run")
            break
        (_, kv2, _, _), _ = r2
        for key in ("produced", "consumed", "duplicates", "missing"):
            if kv2.get(key) != kv.get(key):
                failures.append("unstable across runs (possible data race)")
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
