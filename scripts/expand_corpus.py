#!/usr/bin/env python3
"""
Expand Telex/VNI corpus lines using offline BFS against cbakey_core (cbakey_corpus_bfs).

Examples:
  cmake -S . -B build -DCBAKEY_BUILD_CORPUS_TOOLS=ON -DCBAKEY_BUILD_TESTS=ON
  cmake --build build

  pip install -r scripts/requirements-corpus.txt
  python3 scripts/expand_corpus.py --method telex --limit 200 --jobs 4 \\
      --out corpus/generated_telex.jsonl

  python3 scripts/expand_corpus.py --method vni --words-file corpus/sources/my_words.txt \\
      --out corpus/generated_vni.jsonl --jobs 2

Cache: corpus/.bfs_cache.json (word -> JSON line) avoids re-running slow BFS.
"""

from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import unicodedata
from concurrent.futures import ProcessPoolExecutor, as_completed
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_BFS = REPO_ROOT / "build" / "cbakey_corpus_bfs"
CACHE_PATH = REPO_ROOT / "corpus" / ".bfs_cache.json"


def ascii_key_hint(w: str) -> str:
    """
    Latin skeleton for BFS key ordering (NFKD, strip marks, map đ->d).
    Not a linguistic transliteration; only shrinks search fan-out.
    """
    w = w.translate(str.maketrans({"đ": "d", "Đ": "d"}))
    nfd = unicodedata.normalize("NFD", w)
    chars: list[str] = []
    for ch in nfd:
        if unicodedata.category(ch) == "Mn":
            continue
        o = ord(ch)
        if ("a" <= ch <= "z") or ("A" <= ch <= "Z"):
            chars.append(ch.lower())
        elif ch.isalpha() and o < 128:
            chars.append(ch.lower())
    return "".join(chars)


def load_cache() -> dict[str, str]:
    if not CACHE_PATH.is_file():
        return {}
    try:
        data = json.loads(CACHE_PATH.read_text(encoding="utf-8"))
        if isinstance(data, dict):
            return {str(k): str(v) for k, v in data.items()}
    except (json.JSONDecodeError, OSError):
        pass
    return {}


def save_cache(cache: dict[str, str]) -> None:
    CACHE_PATH.parent.mkdir(parents=True, exist_ok=True)
    tmp = CACHE_PATH.with_suffix(".tmp")
    tmp.write_text(json.dumps(cache, ensure_ascii=False, indent=0) + "\n", encoding="utf-8")
    tmp.replace(CACHE_PATH)


def iter_words_wordfreq(n: int) -> list[str]:
    from wordfreq import top_n_list  # type: ignore

    raw = top_n_list("vi", n=n * 3)
    out: list[str] = []
    seen: set[str] = set()
    for w in raw:
        w = unicodedata.normalize("NFC", w.strip())
        if not w or w in seen:
            continue
        if any(ch.isspace() for ch in w):
            continue
        if len(w) > 24:
            continue
        # Drop pure ASCII 1–2 letter noise; keep longer Latin tokens (e.g. place names).
        if all(ord(ch) < 128 for ch in w) and len(w) <= 2:
            continue
        seen.add(w)
        out.append(w)
        if len(out) >= n:
            break
    # Shorter words first → faster BFS
    out.sort(key=lambda s: (len(s.encode("utf-8")), s))
    return out[:n]


def iter_words_file(path: Path) -> list[str]:
    seen: set[str] = set()
    out: list[str] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        w = unicodedata.normalize("NFC", line.strip())
        if not w or w in seen:
            continue
        if any(ch.isspace() for ch in w):
            continue
        seen.add(w)
        out.append(w)
    out.sort(key=lambda s: (len(s.encode("utf-8")), s))
    return out


def run_bfs_one(bfs: Path, method: str, word: str, hint: str, timeout: float) -> tuple[str, str | None]:
    """
    Returns ("ok", json_line) or ("skip", reason) or ("error", message).
    """
    cmd = [str(bfs), "--one", method, word]
    if hint:
        cmd.append(hint)
    try:
        proc = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=timeout,
            cwd=str(REPO_ROOT),
        )
    except subprocess.TimeoutExpired:
        return "skip", f"timeout>{timeout}s"
    except OSError as e:
        return "error", str(e)

    out = proc.stdout.strip()
    err = proc.stderr.strip()
    if proc.returncode == 0 and out.startswith("{"):
        return "ok", out.splitlines()[-1]
    if err.startswith("SKIP"):
        return "skip", err
    return "error", err or out or f"exit {proc.returncode}"


def worker(job: tuple[str, str, str, float, str]) -> tuple[str, str, str, str | None]:
    """method, word, bfs_path, timeout, hint -> method, word, status, payload"""
    method, word, bfs_path, timeout, hint = job
    st, payload = run_bfs_one(Path(bfs_path), method, word, hint, timeout)
    return method, word, st, payload


def main() -> int:
    ap = argparse.ArgumentParser(description="Generate corpus JSONL via cbakey_corpus_bfs")
    ap.add_argument("--method", choices=("telex", "vni"), required=True)
    ap.add_argument("--out", type=Path, required=True, help="Output JSONL path")
    ap.add_argument("--limit", type=int, default=500, help="Max words to try")
    ap.add_argument("--jobs", type=int, default=max(1, (os.cpu_count() or 4) // 2))
    ap.add_argument("--bfs", type=Path, default=DEFAULT_BFS, help="Path to cbakey_corpus_bfs")
    ap.add_argument("--words-file", type=Path, default=None, help="Line-delimited words (instead of wordfreq)")
    ap.add_argument("--timeout", type=float, default=90.0, help="Per-word BFS timeout (seconds)")
    ap.add_argument("--no-cache", action="store_true", help="Ignore and overwrite merge into cache")
    ap.add_argument("--max-nodes", type=int, default=None, help="CBAKEY_CORPUS_BFS_MAX_NODES for cbakey_corpus_bfs")
    ap.add_argument("--max-depth", type=int, default=None, help="CBAKEY_CORPUS_BFS_MAX_DEPTH for cbakey_corpus_bfs")
    args = ap.parse_args()

    if args.max_nodes is not None:
        os.environ["CBAKEY_CORPUS_BFS_MAX_NODES"] = str(args.max_nodes)
    if args.max_depth is not None:
        os.environ["CBAKEY_CORPUS_BFS_MAX_DEPTH"] = str(args.max_depth)

    if not args.bfs.is_file():
        print(f"Missing {args.bfs} — build the project with -DCBAKEY_BUILD_CORPUS_TOOLS=ON", file=sys.stderr)
        return 2

    if args.words_file is not None:
        if not args.words_file.is_file():
            print(f"Words file not found: {args.words_file}", file=sys.stderr)
            return 2
        words = iter_words_file(args.words_file)
    else:
        try:
            words = iter_words_wordfreq(max(args.limit * 5, 2000))
        except ImportError:
            print(
                "wordfreq not installed. Run: pip install -r scripts/requirements-corpus.txt\n"
                "Or pass --words-file corpus/sources/your_words.txt",
                file=sys.stderr,
            )
            return 2

    words = words[: args.limit]

    cache: dict[str, str] = {} if args.no_cache else load_cache()
    stats = {"cached_hits": 0, "bfs_ok": 0, "bfs_skip": 0, "bfs_error": 0}

    jobs: list[tuple[str, str, str, float, str]] = []
    for w in words:
        key = f"{args.method}\t{w}"
        if key in cache and cache[key].startswith("{"):
            stats["cached_hits"] += 1
            continue
        hint = ascii_key_hint(w)
        jobs.append((args.method, w, str(args.bfs.resolve()), args.timeout, hint))

    if jobs:
        pending_saves = 0
        with ProcessPoolExecutor(max_workers=max(1, args.jobs)) as ex:
            futs = {ex.submit(worker, j): j for j in jobs}
            for fut in as_completed(futs):
                method, word, st, payload = fut.result()
                key = f"{method}\t{word}"
                if st == "ok" and payload:
                    cache[key] = payload
                    stats["bfs_ok"] += 1
                elif st == "skip":
                    stats["bfs_skip"] += 1
                    print(f"[skip] {method}\t{word}\t{payload}", file=sys.stderr)
                else:
                    stats["bfs_error"] += 1
                    print(f"[error] {method}\t{word}\t{payload}", file=sys.stderr)
                pending_saves += 1
                if pending_saves >= 25:
                    save_cache(cache)
                    pending_saves = 0
        if pending_saves:
            save_cache(cache)

    args.out.parent.mkdir(parents=True, exist_ok=True)
    # Rewrite full out from cache for stable deterministic order (sorted keys)
    all_lines: list[str] = []
    for w in words:
        key = f"{args.method}\t{w}"
        line = cache.get(key)
        if line and line.startswith("{"):
            all_lines.append(line)

    # Re-id sequentially to avoid duplicate GEN-one / line collisions
    re_id: list[str] = []
    for i, line in enumerate(all_lines, start=1):
        try:
            obj = json.loads(line)
        except json.JSONDecodeError:
            continue
        obj["id"] = f"GEN-{args.method}-{i:06d}"
        if "tags" in obj and isinstance(obj["tags"], list):
            if "generated.expand_corpus" not in obj["tags"]:
                obj["tags"].append("generated.expand_corpus")
        re_id.append(json.dumps(obj, ensure_ascii=False))

    args.out.write_text("\n".join(re_id) + ("\n" if re_id else ""), encoding="utf-8")

    print(
        json.dumps(
            {
                "written_file": str(args.out),
                "lines": len(re_id),
                "stats": stats,
                "cache_file": str(CACHE_PATH),
            },
            indent=2,
            ensure_ascii=False,
        )
    )
    return 0 if stats["bfs_error"] == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
