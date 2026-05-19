#!/usr/bin/env bash
set -euo pipefail

rm -f "${HOME}/.local/share/fcitx5/addon/farolkey.conf"
rm -f "${HOME}/.local/share/fcitx5/inputmethod/farolkey.conf"
rm -f "${HOME}/.local/lib/fcitx5/farolkey.so"
rm -f "${HOME}/.local/lib/fcitx5/libfarolkey.so"
rm -f "${HOME}/.local/lib/x86_64-linux-gnu/fcitx5/libfarolkey.so"

echo "Removed local FarolKey Fcitx5 files."
echo "Restart fcitx5 to apply changes: pkill fcitx5; fcitx5 -d"
