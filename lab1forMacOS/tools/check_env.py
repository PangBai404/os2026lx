#!/usr/bin/env python3
"""
Environment check for the OS labs.

Output is ASCII on purpose: the Windows console codepage would garble Chinese.

    python3 tools/check_env.py            # check required + optional
    python3 tools/check_env.py --strict   # optional items also count as failures

Only inspects the environment; installs nothing.
"""

import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

OK, WARN, FAIL = "OK  ", "WARN", "FAIL"

ROOT = Path(__file__).resolve().parent.parent
INCLUDE_DIR = ROOT / "include"

# 最小 pthread 程序，用来确认编译器能正确链接线程库
PTHREAD_PROBE = r"""
#include <pthread.h>
#include <stdio.h>
static void *work(void *x) { return x; }
int main(void) {
    pthread_t t;
    if (pthread_create(&t, NULL, work, NULL) != 0) return 1;
    pthread_join(t, NULL);
    printf("pthread-ok\n");
    return 0;
}
"""

# 最小信号量程序：macOS 上通过 include/sem_compat.h 映射到 sem_open
SEM_PROBE = r"""
#include <pthread.h>
#include <semaphore.h>
#include "sem_compat.h"
#include <stdio.h>

int main(void) {
    sem_t s;
    if (sem_init(&s, 0, 1) != 0) {
        printf("sem_init-failed\n");
        return 2;
    }
    if (sem_wait(&s) != 0) {
        printf("sem_wait-failed\n");
        return 3;
    }
    if (sem_post(&s) != 0) {
        printf("sem_post-failed\n");
        return 4;
    }
    sem_destroy(&s);
    printf("semaphore-ok\n");
    return 0;
}
"""


def _build_env(tmpdir):
    """MSYS2 下 make 可能不传 TMP/TEMP，导致 gcc 去 C:\\WINDOWS 建临时文件。"""
    env = os.environ.copy()
    env["TMPDIR"] = str(tmpdir)
    env["TMP"] = str(tmpdir)
    env["TEMP"] = str(tmpdir)
    return env


def find_compiler():
    for name in ("gcc", "cc", "clang"):
        path = shutil.which(name)
        if path:
            return name, path
    return None, None


def compiler_version(cc):
    try:
        out = subprocess.run(
            [cc, "--version"], capture_output=True, text=True, timeout=20
        )
        return (out.stdout or out.stderr).splitlines()[0].strip()
    except Exception:
        return "unknown version"


def probe_pthread(cc, tmpdir):
    """编译并运行一个 pthread 小例子，确认线程库真的可用。"""
    src = Path(tmpdir) / "probe.c"
    exe = Path(tmpdir) / ("probe.exe" if sys.platform == "win32" else "probe")
    src.write_text(PTHREAD_PROBE, encoding="utf-8")

    build = subprocess.run(
        [cc, "-pthread", str(src), "-o", str(exe)],
        capture_output=True, text=True, timeout=60,
        env=_build_env(tmpdir),
    )
    if build.returncode != 0:
        first = (build.stderr or "").strip().splitlines()[:1]
        return False, f"compile failed: {first}"

    run = subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)
    if run.returncode != 0 or "pthread-ok" not in run.stdout:
        return False, "compiled but did not run correctly"

    return True, "compiled and ran a pthread test"


def probe_semaphore(cc, tmpdir):
    """编译并运行一个 sem_init/sem_wait/sem_post 小例子。"""
    src = Path(tmpdir) / "semprobe.c"
    exe = Path(tmpdir) / ("semprobe.exe" if sys.platform == "win32" else "semprobe")
    src.write_text(SEM_PROBE, encoding="utf-8")

    build = subprocess.run(
        [cc, "-pthread", "-I", str(INCLUDE_DIR), str(src), "-o", str(exe)],
        capture_output=True, text=True, timeout=60,
        env=_build_env(tmpdir),
    )
    if build.returncode != 0:
        first = (build.stderr or "").strip().splitlines()[:1]
        return False, f"compile failed: {first}"

    run = subprocess.run([str(exe)], capture_output=True, text=True, timeout=30)
    if run.returncode != 0 or "semaphore-ok" not in run.stdout:
        return False, "compiled but did not run correctly"

    return True, "compiled and ran a semaphore test"


def check_compiler():
    cc, _ = find_compiler()
    if not cc:
        return FAIL, "no C compiler found (gcc / cc / clang)", None
    return OK, f"{cc}  {compiler_version(cc)}", cc


def check_pthread(cc):
    if not cc:
        return FAIL, "skipped: no compiler"
    with tempfile.TemporaryDirectory() as tmp:
        ok, detail = probe_pthread(cc, tmp)
    return (OK if ok else FAIL), detail


def check_semaphore(cc):
    if not cc:
        return FAIL, "skipped: no compiler"
    with tempfile.TemporaryDirectory() as tmp:
        ok, detail = probe_semaphore(cc, tmp)
    return (OK if ok else FAIL), detail


def check_python():
    v = sys.version_info
    if v < (3, 8):
        return FAIL, f"Python {v.major}.{v.minor}, need 3.8+"
    return OK, f"Python {v.major}.{v.minor}.{v.micro}"


def check_make():
    path = shutil.which("make")
    if path:
        return OK, path
    return WARN, "not found (optional, see below)"


def check_riscv():
    cc = shutil.which("riscv64-linux-gnu-gcc")
    qemu = shutil.which("qemu-riscv64")
    if cc and qemu:
        return OK, "cross compiler and qemu-user installed"
    missing = []
    if not cc:
        missing.append("riscv64-linux-gnu-gcc")
    if not qemu:
        missing.append("qemu-riscv64")
    return WARN, "missing " + ", ".join(missing) + " (only for the RISC-V target)"


def main():
    strict = "--strict" in sys.argv

    print("=" * 60)
    print("Environment check")
    print("=" * 60)

    print("\n[required]")
    required = []
    status, detail, cc = check_compiler()
    required.append(("C compiler", status))
    print(f"  [{status}] C compiler       {detail}")

    status, detail = check_pthread(cc)
    required.append(("pthread support", status))
    print(f"  [{status}] pthread support  {detail}")

    status, detail = check_semaphore(cc)
    required.append(("semaphore support", status))
    print(f"  [{status}] semaphore        {detail}")

    status, detail = check_python()
    required.append(("Python 3", status))
    print(f"  [{status}] Python 3         {detail}")

    print("\n[optional]")
    optional = []
    status, detail = check_make()
    optional.append(("make", status))
    print(f"  [{status}] make             {detail}")

    status, detail = check_riscv()
    optional.append(("RISC-V toolchain", status))
    print(f"  [{status}] RISC-V toolchain {detail}")

    print("\n" + "-" * 60)

    missing_required = [n for n, s in required if s == FAIL]
    missing_optional = [n for n, s in optional if s != OK]

    if missing_required:
        print("Missing required components:")
        for name in missing_required:
            print(f"  - {name}")
        print("\nWindows install hints:")
        print("  Recommended: w64devkit (single zip with gcc / make / gdb)")
        print("    https://github.com/skeeto/w64devkit/releases")
        print("  Or MSYS2: pacman -S mingw-w64-ucrt-x86_64-gcc")
        print("  Python 3: https://www.python.org/downloads/")
        print("\nmacOS note:")
        print("  macOS has no working sem_init(); this package maps it to sem_open")
        print("  through include/sem_compat.h. Make sure compile commands use")
        print("  -I../include (the lab Makefiles already do).")

    if missing_optional:
        print("Optional components missing (the basic labs still work):")
        for name in missing_optional:
            print(f"  - {name}")

    if not shutil.which("make"):
        print("\nmake is optional. Each lab's README lists the equivalent two commands,")
        print("for example inside lab1-1/:")
        print("  gcc -pthread student/impl.c tests/harness.c -o 3")
        print("  python3 tests/check.py 3")

    if missing_required or (strict and missing_optional):
        print("\nResult: environment is NOT ready.")
        return 1

    print("\nResult: environment ready.")
    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        sys.exit(130)
