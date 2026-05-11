#!/usr/bin/env python3
"""
Filter obvious junk / non-Vietnamese tokens from generated corpus JSONL.

Goals:
- Keep clearly Vietnamese tokens, especially those with Vietnamese-specific letters.
- Drop symbols / digits / punctuation / malformed tokens.
- For ASCII-only ambiguous tokens, use a lightweight syllable heuristic and, when
  available, compare `wordfreq` Vietnamese vs English frequency.

Outputs:
- kept JSONL (same schema objects, unchanged order / ids)
- rejected JSONL with reasons for manual review

Examples:
  python3 scripts/filter_corpus_tokens.py \
    --input corpus/generated/telex_wordfreq.jsonl \
    --output corpus/generated/telex_wordfreq.filtered.jsonl \
    --reject-output corpus/generated/telex_wordfreq.rejected.jsonl

  python3 scripts/filter_corpus_tokens.py \
    --input corpus/generated/vni_wordfreq.jsonl \
    --output corpus/generated/vni_wordfreq.filtered.jsonl \
    --reject-output corpus/generated/vni_wordfreq.rejected.jsonl
"""

from __future__ import annotations

import argparse
import json
import sys
import unicodedata
from pathlib import Path

try:
    from wordfreq import zipf_frequency  # type: ignore
except ImportError:  # pragma: no cover - optional dependency
    zipf_frequency = None


VIET_BASE_VOWELS = set("aăâeêioôơuưy")
VIET_ALL_LETTERS = set(
    "abcdefghijklmnopqrstuvwxyz"
    "àáảãạăằắẳẵặâầấẩẫậ"
    "èéẻẽẹêềếểễệ"
    "ìíỉĩị"
    "òóỏõọôồốổỗộơờớởỡợ"
    "ùúủũụưừứửữự"
    "ỳýỷỹỵ"
    "đ"
)

ONSETS = (
    "ngh",
    "ch",
    "gh",
    "gi",
    "kh",
    "ng",
    "nh",
    "ph",
    "qu",
    "th",
    "tr",
    "b",
    "c",
    "d",
    "đ",
    "g",
    "h",
    "k",
    "l",
    "m",
    "n",
    "p",
    "q",
    "r",
    "s",
    "t",
    "v",
    "x",
    "",
)

CODAS = ("ch", "ng", "nh", "c", "m", "n", "p", "t", "")

LATIN_VOWELS = set("aeiouy")
VALID_ASCII_LETTERS = set("abcdefghijklmnopqrstuvwxyzđ")


def strip_commit_suffix(text: str) -> str:
    return text.rstrip(" \n\t")


def remove_tone_marks(text: str) -> str:
    mapped = text.translate(str.maketrans({"đ": "d", "Đ": "d"}))
    nfd = unicodedata.normalize("NFD", mapped)
    return "".join(ch for ch in nfd if unicodedata.category(ch) != "Mn").lower()


def has_vietnamese_specific_letter(text: str) -> bool:
    skeleton = remove_tone_marks(text)
    return any(ch != sk for ch, sk in zip(text.lower(), skeleton)) or "đ" in text.lower()


def is_all_letters(token: str) -> bool:
    return bool(token) and all(ch.lower() in VIET_ALL_LETTERS for ch in token)


def ascii_syllable_like(base: str) -> bool:
    if not base or any(ch not in VALID_ASCII_LETTERS for ch in base):
        return False
    if any(ch in "fwjz" for ch in base):
        return False

    for onset in ONSETS:
        if not base.startswith(onset):
            continue
        rest = base[len(onset) :]
        for coda in CODAS:
            if not rest.endswith(coda):
                continue
            nucleus = rest[: len(rest) - len(coda)] if coda else rest
            if not nucleus:
                continue
            if len(nucleus) > 4:
                continue
            if any(ch not in LATIN_VOWELS for ch in nucleus):
                continue
            if onset == "q" and not nucleus.startswith("u"):
                continue
            if onset == "qu" and nucleus.startswith("u"):
                continue
            if onset == "gi" and nucleus.startswith("i"):
                continue
            return True
    return False


def wordfreq_allows_ascii(token: str, base: str) -> bool:
    if zipf_frequency is None:
        return True
    vi = zipf_frequency(token, "vi")
    en = zipf_frequency(base, "en")
    fr = zipf_frequency(base, "fr")
    de = zipf_frequency(base, "de")
    foreign = max(en, fr, de)

    # Keep strong Vietnamese entries, reject obvious foreign/common ASCII tokens.
    if vi >= 3.0:
        return True
    if foreign >= vi + 1.2:
        return False
    if vi >= 1.8 and foreign <= vi + 0.4:
        return True
    return False


def classify_token(token: str) -> tuple[bool, str]:
    if not token:
        return False, "empty"
    if any(ch.isspace() for ch in token):
        return False, "contains_whitespace"
    if not is_all_letters(token):
        return False, "non_letter_or_symbol"

    lower = unicodedata.normalize("NFC", token.lower())
    base = remove_tone_marks(lower)

    if not base.isascii():
        return False, "non_ascii_base_after_normalize"
    if any(ch.isdigit() for ch in base):
        return False, "contains_digit"

    if has_vietnamese_specific_letter(lower):
        return True, "keep_vietnamese_specific_letter"

    if not ascii_syllable_like(base):
        return False, "not_vietnamese_syllable_like"

    if not wordfreq_allows_ascii(lower, base):
        return False, "ascii_foreign_by_wordfreq"

    return True, "keep_ascii_vietnamese_like"


def filter_jsonl(input_path: Path, output_path: Path, reject_output_path: Path) -> int:
    kept_lines: list[str] = []
    rejected_lines: list[str] = []
    total = 0
    kept = 0

    for line_no, raw in enumerate(input_path.read_text(encoding="utf-8").splitlines(), start=1):
        raw = raw.strip()
        if not raw:
            continue
        total += 1
        try:
            obj = json.loads(raw)
        except json.JSONDecodeError as e:
            rejected_lines.append(
                json.dumps(
                    {
                        "input_file": str(input_path),
                        "line": line_no,
                        "reason": f"json_decode_error: {e.msg}",
                        "raw": raw,
                    },
                    ensure_ascii=False,
                )
            )
            continue

        commit = obj.get("expect", {}).get("commit", "")
        token = strip_commit_suffix(commit)
        ok, reason = classify_token(token)

        if ok:
            kept_lines.append(json.dumps(obj, ensure_ascii=False))
            kept += 1
        else:
            rejected_lines.append(
                json.dumps(
                    {
                        "id": obj.get("id"),
                        "token": token,
                        "reason": reason,
                        "source_commit": commit,
                        "source_line": line_no,
                    },
                    ensure_ascii=False,
                )
            )

    output_path.parent.mkdir(parents=True, exist_ok=True)
    reject_output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text("\n".join(kept_lines) + ("\n" if kept_lines else ""), encoding="utf-8")
    reject_output_path.write_text(
        "\n".join(rejected_lines) + ("\n" if rejected_lines else ""), encoding="utf-8"
    )

    print(
        json.dumps(
            {
                "input": str(input_path),
                "output": str(output_path),
                "reject_output": str(reject_output_path),
                "total": total,
                "kept": kept,
                "rejected": total - kept,
                "wordfreq_enabled": zipf_frequency is not None,
            },
            ensure_ascii=False,
            indent=2,
        )
    )
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description="Filter junk tokens from generated corpus JSONL")
    ap.add_argument("--input", type=Path, required=True)
    ap.add_argument("--output", type=Path, required=True)
    ap.add_argument("--reject-output", type=Path, required=True)
    args = ap.parse_args()

    if not args.input.is_file():
        print(f"Input not found: {args.input}", file=sys.stderr)
        return 2

    return filter_jsonl(args.input, args.output, args.reject_output)


if __name__ == "__main__":
    raise SystemExit(main())
