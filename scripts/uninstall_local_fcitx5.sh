#!/usr/bin/env bash
set -euo pipefail

rm -f "${HOME}/.local/share/fcitx5/addon/cbakey.conf"
rm -f "${HOME}/.local/share/fcitx5/inputmethod/cbakey.conf"
rm -f "${HOME}/.local/lib/fcitx5/cbakey.so"
rm -f "${HOME}/.local/lib/fcitx5/libcbakey.so"
rm -f "${HOME}/.local/lib/x86_64-linux-gnu/fcitx5/libcbakey.so"

echo "Removed local CBAKey Fcitx5 files."
echo "Restart fcitx5 to apply changes: pkill fcitx5; fcitx5 -d"
