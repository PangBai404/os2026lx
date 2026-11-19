#!/usr/bin/env python3
"""
实验二 内存管理 的测例（兼容 Python 3.4+，无 f-string / capture_output / text=）。

用法：
    python3 tests/check.py                            # 测 build/2_harness（教师版）
    python3 tests/check.py build/2_harness_student    # 测学生提交（同一命令判所有人）
    python3 tests/check.py --audit student/impl.c     # 源码审计(防伪造输出)

学号：学生在 student/impl.c 顶部的 SID4 填写自己的学号后四位。
判分只要求 SID4 是 4 位数字（[sid] 行），并用它独立计算期望串精确核对。

判据（只检查不变量，不比对完整输出）：
    I0  [sid] value 是 4 位 ASCII 数字（学生在 impl.c 顶部填写了学号）
    I1  exit code 0，stderr 为空
    I2  pagesize == 本机真页大小（脚本独立获取，不信任程序）
    I3  src 初始内容 == 本次随机写入 f1 的内容（mmap 真做了）
    I4  dst 备份内容 == 同上（malloc+memmove 真做了）
    I5  temp_off == 'W' 的真实偏移（memchr 真找到了）
    I6  子进程 dst、src 都 == 期望串1（WXYZ→SID4，且写回共享区）
    I7  父进程 src == 期望串2（QQ 前缀）；父进程 dst == 原始内容（私有内存未被污染）
    I8  子进程 PPID == 父进程 PID，两 PID 不同
    I9  两行 [end]：child 行=期望串1（无 QQ），parent 行=期望串2
    I10 程序内 readback 与退出后重读 f1，都 == 期望串2（MAP_SHARED 真写回文件）
    I11 cleanup rc==0（munmap/close 成功）
    I12 连续多轮（1 轮演示数据 + 5 轮随机数据）全部通过

防作弊：
    * main 在 harness 中，判分行带运行时随机 nonce，学生代码拿不到 nonce，
      学生自打印行（无 nonce）一律忽略——伪造输出无效；
    * f1 内容每轮随机（WXYZ 位置随机、字母表无大写字母），硬编码答案必挂；
    * harness 退出后重读 f1——不真做 mmap 的假实现永远过不了"文件被修改"。
"""

import argparse
import os
import random
import re
import string
import subprocess
import sys
from pathlib import Path

DATALEN  = 19
ALPHABET = string.ascii_lowercase + string.digits   # 无大写字母 → WXYZ 必唯一
DIGITS   = set("0123456789")
ROOT     = Path(__file__).resolve().parent.parent

NONCE_RE = re.compile(r"^nonce=([0-9a-f]{8})$")
LINE_RE  = re.compile(r"^\[(?P<tag>[a-z_]+)\]\s*(?P<rest>.*)$")
KV_RE    = re.compile(r"^(?P<k>[a-z_]+)=(?P<v>\S*)$")

# 学生源码中不允许出现的保留输出标记（审计用）
FORBIDDEN = ["nonce=", "[final]", "[proc]", "[child]", "[parent]", "[sid]",
             "[file", "[end]", "[setup]", "[backup]", "[initial]", "[cleanup]"]


def find_binary(arg):
    if arg:
        p = Path(arg)
        if not p.is_absolute():
            p = ROOT / p
    else:
        p = ROOT / "build" / "2_harness"

    for cand in (p, p.with_name(p.name + ".exe")):
        if cand.exists():
            return cand
    raise SystemExit('Binary not found: {}\nRun "make test" first.'.format(p))


def make_data(rng):
    """生成 19 字节随机数据：WXYZ 位于随机位置 [2,13]，字母表保证 W 唯一、无 Q。"""
    wpos  = rng.randint(2, 13)
    chars = [rng.choice(ALPHABET) for _ in range(DATALEN - 4)]
    return "".join(chars[:wpos]) + "WXYZ" + "".join(chars[wpos:])


def parse(out):
    """解析 harness 输出：返回 (nonce, [(tag, {k:v}), ...])。无 nonce 的行一律忽略。"""
    nonce, rows = None, []
    for raw in out.splitlines():
        line = raw.strip()
        if nonce is None:
            m = NONCE_RE.match(line)
            if m:
                nonce = m.group(1)
            continue
        if not line.startswith(nonce):
            continue                        # 学生自打印行：忽略（防伪造）
        line = line[len(nonce):].strip()
        m = LINE_RE.match(line)
        if not m:
            continue
        kv = {}
        for tok in m.group("rest").split():
            km = KV_RE.match(tok)
            if km:
                kv[km.group("k")] = km.group("v")
        rows.append((m.group("tag"), kv))
    return nonce, rows


def run_cmd(cmd, timeout=30):
    """兼容 3.4 的 subprocess 封装（不依赖 capture_output/text 参数）。"""
    p = subprocess.Popen(cmd, stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE, cwd=str(ROOT))
    try:
        out, err = p.communicate(timeout=timeout)
    except subprocess.TimeoutExpired:
        p.kill()
        p.communicate()
        raise
    return (p.returncode,
            out.decode("utf-8", "replace"),
            err.decode("utf-8", "replace"))


def one_round(binary, data):
    """跑一轮，返回失败列表（空 = 通过）。SID4 与期望串由输出独立解析/计算。"""
    fails = []
    wpos = data.find("WXYZ")

    try:
        rc, out, err = run_cmd([str(binary), data])
    except subprocess.TimeoutExpired:
        return ["timeout: 程序 30s 未结束（child_work 死循环或 wait 异常）"]

    # I1
    if rc != 0:
        fails.append("exit code {} (expected 0)".format(rc))
    if err.strip():
        fails.append("stderr not empty: {}".format(err.strip()[:120]))

    nonce, rows = parse(out)
    if nonce is None:
        fails.append("no nonce line (harness 输出缺失，请勿修改 harness)")
        return fails

    tag = {}
    for t, kv in rows:
        tag.setdefault(t, []).append(kv)

    def last(t):
        return tag[t][-1] if tag.get(t) else {}

    # I0 学号：必须在 impl.c 顶部填写且为 4 位 ASCII 数字
    sid = last("sid").get("value")
    if last("sid").get("isdigits") != "true" or not sid \
            or len(sid) != 4 or any(ch not in DIGITS for ch in sid):
        fails.append('SID4 非法({}): 请在 student/impl.c 顶部把 '
                     'const char SID4[5] = "xxxx" 改成你的 4 位数字学号后四位'
                     .format(repr(sid)))
        return fails                    # 学号没填好，后续期望串无从谈起

    # 期望串（脚本独立计算，不信任程序）
    expect1 = data[:wpos] + sid + data[wpos + 4:]     # WXYZ → 学号后四位
    expect2 = "QQ" + expect1[2:]                      # 父进程再叠 QQ

    # I2 页大小（独立获取）
    real_ps = os.sysconf("SC_PAGESIZE")
    if last("setup").get("pagesize") != str(real_ps):
        fails.append("pagesize should be {}, got {}（getpagesize 未正确实现）"
                     .format(real_ps, last("setup").get("pagesize")))

    # I3/I4 数据行与初始快照
    if last("data").get("content") != data:
        fails.append("[data] content 与传入数据不一致（harness 被改动？）")
    if last("initial").get("src") != data:
        fails.append("initial src should be {}, got {}（mmap 未成功映射 f1）"
                     .format(data, last("initial").get("src")))
    if last("backup").get("dst") != data:
        fails.append("backup dst should be {}, got {}（malloc+memmove 备份未完成）"
                     .format(data, last("backup").get("dst")))

    # I5/I6 子进程
    c = last("child")
    if not c:
        fails.append("missing [child] line")
    else:
        if c.get("rc") != "0":
            fails.append("child_work rc={} (expected 0)".format(c.get("rc")))
        if c.get("temp_off") != str(wpos):
            fails.append("temp_off should be {} ('W' 的偏移), got {}"
                         "（memchr 未正确找到 W——注意数据是随机的！）"
                         .format(wpos, c.get("temp_off")))
        if c.get("dst") != expect1:
            fails.append("child dst should be {}, got {}"
                         "（WXYZ 未替换为你填的学号，或未用 SID4 而写了别的值）"
                         .format(expect1, c.get("dst")))
        if c.get("src") != expect1:
            fails.append("child src should be {}, got {}"
                         "（修改后的 dst 未 memmove 写回共享区）"
                         .format(expect1, c.get("src")))

    # I8 进程关系
    procs = {}
    for kv in tag.get("proc", []):
        procs[kv.get("role")] = kv
    ch, pa = procs.get("child"), procs.get("parent")
    if not ch or not pa:
        fails.append("missing [proc] child/parent line")
    else:
        if ch.get("ppid") != pa.get("pid"):
            fails.append("child PPID != parent PID")
        if ch.get("pid") == pa.get("pid"):
            fails.append("child PID == parent PID (impossible)")

    # I7 父进程：共享区叠加 QQ；私有 dst 未被污染
    p = last("parent")
    if not p:
        fails.append("missing [parent] line")
    else:
        if p.get("src") != expect2:
            fails.append("parent src should be {}, got {}"
                         "（memset QQ 未完成或子进程未写回共享区）"
                         .format(expect2, p.get("src")))
        if p.get("dst") != data:
            fails.append("parent dst should stay {}, got {}"
                         "（malloc 私有内存被子进程污染？不应发生）"
                         .format(data, p.get("dst")))

    # I9 两行 [end]（对应教学版最后的 printf：父子各执行一次）
    ends = {}
    for kv in tag.get("end", []):
        ends[kv.get("role")] = kv
    if ends.get("child", {}).get("src") != expect1:
        fails.append("[end] child src should be {}"
                     "（子进程结束时父进程尚未 memset，不应含 QQ）".format(expect1))
    if ends.get("parent", {}).get("src") != expect2:
        fails.append("[end] parent src should be {}".format(expect2))

    # I10 文件内容（程序内 readback + 退出后重读）
    frow = last("file")
    if frow.get("content") != expect2:
        fails.append("in-process file content should be {}, got {}"
                     "（lseek+read 未正确实现）"
                     .format(expect2, frow.get("content")))
    fa = last("file_after")
    if fa.get("content") != expect2:
        fails.append("file content after exit should be {}, got {}"
                     "（MAP_SHARED 未写回文件——是否真的做了 mmap？）"
                     .format(expect2, fa.get("content")))

    # I11
    if last("cleanup").get("rc") != "0":
        fails.append("cleanup rc != 0（munmap/close 失败）")

    # 程序自报（脚本仍已独立复核以上全部）
    if last("final").get("result") != "PASS":
        fails.append("harness self-check result != PASS")

    return fails


def audit(src_path):
    with open(src_path, "r") as fh:
        text = fh.read()
    hits = [t for t in FORBIDDEN if t in text]
    print("Auditing: {}".format(src_path))
    if hits:
        print("WARNING: 发现保留输出标记:", ", ".join(hits))
        print("这些行是 harness 专用输出；学生代码打印它们(无 nonce)不会生效，")
        print("但请检查学生是否试图伪造结果。")
        return 1
    print("OK: 未发现伪造输出企图。")
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("binary", nargs="?", help="待测可执行文件（缺省 build/2_harness）")
    ap.add_argument("--rounds", type=int, default=5, help="随机数据轮数（默认 5）")
    ap.add_argument("--audit", metavar="IMPL_C", help="审计学生源码是否试图伪造输出")
    args = ap.parse_args()

    if args.audit:
        return audit(args.audit)

    binary = find_binary(args.binary)
    rng = random.Random()

    print("Testing: {}".format(binary))
    print("Mode   : harness (nonce), SID4 read from student/impl.c")
    print("-" * 60)

    all_fails = []
    cases = ["0123456789abcWXYZde"] + [make_data(rng) for _ in range(args.rounds)]
    for i, data in enumerate(cases, 1):
        fails = one_round(binary, data)
        verdict = "PASS" if not fails else "FAILED"
        print("round {}: data={}  -> {}".format(i, data, verdict))
        for j, msg in enumerate(fails, 1):
            print("    {}. {}".format(j, msg))
        all_fails += fails

    print("-" * 60)
    if all_fails:
        print("RESULT: FAILED ({} problem(s) across {} rounds)"
              .format(len(all_fails), len(cases)))
        return 1
    print("RESULT: PASSED ({} rounds, 不变量 I0~I12 全部满足)".format(len(cases)))
    return 0


if __name__ == "__main__":
    sys.exit(main())
