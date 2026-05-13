#!/usr/bin/env bash
# Configure + build CBAKey, then install the fcitx5 plugin to user paths (~/.local/...).
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

cmake -S . -B build -DCBAKEY_BUILD_FCITX5_PLUGIN=ON
cmake --build build
bash "${ROOT_DIR}/scripts/install_local_fcitx5.sh"
