#!/usr/bin/env bash
# Wait for fcitx5 to exit before starting again so the old process releases the IME plugin
# (.so). Rapid "pkill; fcitx5 -d" can otherwise load a mix of old/new state or the wrong inode.
set -euo pipefail

pkill -x fcitx5 2>/dev/null || true
for _ in $(seq 1 50); do
  if ! pgrep -x fcitx5 >/dev/null 2>&1; then
    break
  fi
  sleep 0.05
done
sleep 0.15
exec fcitx5 -d
