#!/usr/bin/env python3
"""
FarolKey Frontend Probe
Theo dõi D-Bus để xác định frontend và capability mà một app dùng với fcitx5.

Cách dùng:
  python3 farolkey_frontend_probe.py
  → Click vào ô nhập liệu trong app cần kiểm tra (Edge, Firefox,...)
  → Ctrl+C để dừng
"""

import subprocess
import re
import sys

# Fcitx5 CapabilityFlag bit values (từ fcitx-utils/capabilityflags.h)
CAP_BITS = {
    1 << 0:  ("ClientSideUI",          ""),
    1 << 1:  ("Preedit",               "← inline preedit, Client mode"),
    1 << 2:  ("ClientSideControlState",""),
    1 << 3:  ("Password",              "← password field"),
    1 << 4:  ("FormattedPreedit",      ""),
    1 << 5:  ("ClientUnfocusCommit",   "← auto-commit preedit khi mất focus (GTK style)"),
    1 << 6:  ("SurroundingText",       "← surrounding text API"),
    1 << 7:  ("Email",                 ""),
    1 << 8:  ("Digit",                 ""),
    1 << 23: ("GetIMInfoOnFocus",      ""),
    1 << 24: ("RelativeRect",          ""),
    1 << 32: ("Terminal",              "← terminal emulator"),
    1 << 35: ("Multiline",             "← multi-line input (e.g. textarea)"),
    1 << 36: ("Sensitive",             "← sensitive / private"),
    1 << 37: ("KeyEventOrderFix",      ""),
    1 << 38: ("ReportKeyRepeat",       ""),
    1 << 39: ("ClientSideInputPanel",  ""),
    1 << 40: ("Disable",               "← IM disabled"),
    1 << 41: ("CommitStringWithCursor",""),
}

KEY_FLAGS = {
    "preedit" : 1 << 1,
    "autoCmt" : 1 << 5,
    "surround": 1 << 6,
}


def fmt_caps(v: int) -> str:
    lines = []
    for bit, (name, note) in sorted(CAP_BITS.items()):
        if v & bit:
            lines.append(f"  ✓  {name:<30} {note}")
    if not lines:
        lines.append("  (không có flag nào)")
    return "\n".join(lines)


def print_result(program: str, display: str, caps: int) -> None:
    preedit  = "1" if caps & KEY_FLAGS["preedit"]  else "0"
    auto_cmt = "1" if caps & KEY_FLAGS["autoCmt"]  else "0"
    surround = "1" if caps & KEY_FLAGS["surround"] else "0"

    print(f"\n{'━' * 55}")
    print(f"  program  = {program}")
    print(f"  display  = {display or '?'}")
    print(f"  frontend = dbus  (kết nối trực tiếp qua D-Bus)")
    print(f"  preedit  = {preedit}   autoCmt = {auto_cmt}   surround = {surround}")
    print(f"\n  Capability flags đang bật (raw = {caps} / 0x{caps:016x}):")
    print(fmt_caps(caps))
    print(f"{'━' * 55}\n")


def main() -> None:
    print("=" * 55)
    print("  FarolKey Frontend Probe")
    print("=" * 55)
    print("Đang lắng nghe D-Bus session bus...")
    print()
    print("→ Click vào ô nhập liệu trong app cần kiểm tra.")
    print("→ Nếu không thấy output sau khi click → app đang")
    print("  dùng frontend XIM hoặc fcitx4, không phải dbus.")
    print("→ Ctrl+C để dừng.\n")

    try:
        proc = subprocess.Popen(
            ["dbus-monitor", "--session",
             "type='method_call',destination='org.fcitx.Fcitx5'"],
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
            bufsize=1,
        )
    except FileNotFoundError:
        sys.exit("Lỗi: dbus-monitor không tìm thấy.\nCài: sudo apt install dbus")

    # ── State machine ─────────────────────────────────────────────────────────
    #
    # Khi một app kết nối fcitx5 dbus, sequence là:
    #   1. CreateInputContext (gửi program name, display)
    #   2. SetCapability      (gửi capability flags)
    #
    # Chúng ta track CreateInputContext để lấy program/display,
    # rồi khi SetCapability đến thì print kết quả.

    state    = "idle"
    program  = None
    display  = None
    last_key = None   # cho parser key-value pairs của CreateInputContext

    try:
        for raw_line in proc.stdout:
            line = raw_line.rstrip()

            # ── CreateInputContext ─────────────────────────────────────────────
            if ("InputMethod1" in line and "CreateInputContext" in line):
                state    = "create"
                program  = None
                display  = None
                last_key = None
                continue

            if state == "create":
                # Mỗi khi thấy method call mới → kết thúc block hiện tại
                if "method call" in line:
                    state = "idle"
                    # Xử lý dòng mới này ngay bên dưới
                else:
                    m = re.search(r'string\s+"([^"]*)"', line)
                    if m:
                        val = m.group(1)
                        if last_key is None:
                            last_key = val           # string đầu = key
                        else:
                            if last_key == "program":
                                program = val
                            elif last_key == "display":
                                display = val
                            last_key = None          # reset cho cặp tiếp theo
                    continue

            # ── SetCapability ──────────────────────────────────────────────────
            if ("InputContext1" in line and "SetCapability" in line):
                state = "caps"
                continue

            if state == "caps":
                m = re.search(r'uint64\s+(\d+)', line)
                if m:
                    caps = int(m.group(1))
                    if program:
                        print_result(program, display, caps)
                    state = "idle"
                elif line.strip() == "" or "method" in line:
                    state = "idle"
                continue

    except KeyboardInterrupt:
        pass
    finally:
        proc.terminate()

    print("\nĐã dừng.")


if __name__ == "__main__":
    main()
