#!/usr/bin/env python3
"""
Build canonical corpus/final/*.jsonl from curated + generated sources.

Rules:
- Prefer curated/sample files first, then append unique generated entries.
- Deduplicate by behavior-bearing fields (ignore id, merge tags).
- Reassign stable sequential IDs for final corpus files.
"""

from __future__ import annotations

import json
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent
CORPUS_ROOT = REPO_ROOT / "corpus"
FINAL_ROOT = CORPUS_ROOT / "final"


def read_jsonl(path: Path) -> list[dict]:
    rows: list[dict] = []
    for raw in path.read_text(encoding="utf-8").splitlines():
        raw = raw.strip()
        if not raw:
            continue
        rows.append(json.loads(raw))
    return rows


def dedupe_key(obj: dict) -> str:
    key_obj = {
        "corpus_schema_version": obj.get("corpus_schema_version"),
        "config": obj.get("config"),
        "sequence": obj.get("sequence"),
        "expect": obj.get("expect"),
        "expect_trace": obj.get("expect_trace"),
        "expect_accumulated_commits": obj.get("expect_accumulated_commits"),
        "skip": obj.get("skip", False),
        "skip_reason": obj.get("skip_reason"),
    }
    return json.dumps(key_obj, ensure_ascii=False, sort_keys=True)


def merge_entries(paths: list[Path], prefix: str) -> list[dict]:
    seen: dict[str, dict] = {}
    ordered: list[dict] = []

    for path in paths:
        for obj in read_jsonl(path):
            key = dedupe_key(obj)
            if key in seen:
                prev_tags = list(seen[key].get("tags", []))
                new_tags = list(obj.get("tags", []))
                merged_tags = []
                for tag in prev_tags + new_tags:
                    if tag not in merged_tags:
                        merged_tags.append(tag)
                if merged_tags:
                    seen[key]["tags"] = merged_tags
                continue

            copied = json.loads(json.dumps(obj, ensure_ascii=False))
            seen[key] = copied
            ordered.append(copied)

    for idx, obj in enumerate(ordered, start=1):
        obj["id"] = f"FINAL-{prefix}-{idx:06d}"

    return ordered


def write_jsonl(path: Path, rows: list[dict]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        "\n".join(json.dumps(row, ensure_ascii=False) for row in rows) + ("\n" if rows else ""),
        encoding="utf-8",
    )


def main() -> int:
    telex_paths = [
        CORPUS_ROOT / "telex.jsonl",
        CORPUS_ROOT / "generated" / "telex_wordfreq.jsonl",
    ]
    vni_paths = [
        CORPUS_ROOT / "vni.jsonl",
        CORPUS_ROOT / "generated" / "vni_wordfreq.jsonl",
    ]
    meta_paths = [CORPUS_ROOT / "engine_meta.jsonl"]

    telex_rows = merge_entries(telex_paths, "telex")
    vni_rows = merge_entries(vni_paths, "vni")
    meta_rows = merge_entries(meta_paths, "meta")

    write_jsonl(FINAL_ROOT / "telex.jsonl", telex_rows)
    write_jsonl(FINAL_ROOT / "vni.jsonl", vni_rows)
    write_jsonl(FINAL_ROOT / "engine_meta.jsonl", meta_rows)

    print(
        json.dumps(
            {
                "output_dir": str(FINAL_ROOT),
                "counts": {
                    "telex": len(telex_rows),
                    "vni": len(vni_rows),
                    "engine_meta": len(meta_rows),
                },
            },
            ensure_ascii=False,
            indent=2,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
