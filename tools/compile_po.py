#!/usr/bin/env python3
"""Compile a .po file to a .mo file — no external dependencies required."""
import struct
import sys


def compile_po(po_path: str, mo_path: str) -> None:
    entries: list[tuple[bytes, bytes]] = []
    msgid: list[str] = []
    msgstr: list[str] = []
    in_msgid = False
    in_msgstr = False

    with open(po_path, encoding="utf-8") as f:
        for raw_line in f:
            line = raw_line.rstrip("\n")
            if line.startswith("#"):
                continue
            if line.startswith("msgid "):
                if msgstr:
                    key = "".join(msgid).encode("utf-8")
                    val = "".join(msgstr).encode("utf-8")
                    entries.append((key, val))
                msgid = [_unescape(line[7:-1])]
                msgstr = []
                in_msgid = True
                in_msgstr = False
            elif line.startswith("msgstr "):
                msgstr = [_unescape(line[8:-1])]
                in_msgid = False
                in_msgstr = True
            elif line.startswith('"'):
                chunk = _unescape(line[1:-1])
                if in_msgstr:
                    msgstr.append(chunk)
                elif in_msgid:
                    msgid.append(chunk)
            else:
                in_msgid = in_msgstr = False

    if msgstr:
        key = "".join(msgid).encode("utf-8")
        val = "".join(msgstr).encode("utf-8")
        entries.append((key, val))

    # Sort by original string (required by .mo spec for binary search)
    entries.sort(key=lambda e: e[0])

    # Write .mo (GNU MO format, little-endian)
    MAGIC = 0x950412DE
    N = len(entries)
    header_size = 28          # 7 × 4-byte fields
    orig_table_off = header_size
    trans_table_off = orig_table_off + N * 8
    strings_off = trans_table_off + N * 8

    orig_table: list[tuple[int, int]] = []
    trans_table: list[tuple[int, int]] = []
    string_data = bytearray()

    for orig, trans in entries:
        orig_table.append((len(orig), strings_off + len(string_data)))
        string_data += orig + b"\x00"
        trans_table.append((len(trans), strings_off + len(string_data)))
        string_data += trans + b"\x00"

    with open(mo_path, "wb") as out:
        out.write(struct.pack("<IIIIIII",
                              MAGIC, 0, N,
                              orig_table_off, trans_table_off,
                              0, 0))
        for length, offset in orig_table:
            out.write(struct.pack("<II", length, offset))
        for length, offset in trans_table:
            out.write(struct.pack("<II", length, offset))
        out.write(string_data)


def _unescape(s: str) -> str:
    return (s.replace("\\n", "\n")
             .replace("\\t", "\t")
             .replace("\\\\", "\\")
             .replace('\\"', '"'))


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} input.po output.mo", file=sys.stderr)
        sys.exit(1)
    compile_po(sys.argv[1], sys.argv[2])
    print(f"Compiled: {sys.argv[1]} -> {sys.argv[2]}")
