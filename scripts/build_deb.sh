#!/usr/bin/env bash
# M10.1: Build a .deb package for CBAKey Fcitx5 plugin.
# Usage: bash scripts/build_deb.sh [build_dir]
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-${ROOT_DIR}/build_deb}"

echo "=== CBAKey .deb packaging ==="
echo "Root: ${ROOT_DIR}"
echo "Build dir: ${BUILD_DIR}"

# Configure Release build with plugin + install targets.
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCBAKEY_BUILD_FCITX5_PLUGIN=ON \
    -DCBAKEY_BUILD_TESTS=OFF \
    -DCBAKEY_BUILD_CORPUS_TOOLS=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr \
    "$@"

# Build the plugin.
cmake --build "${BUILD_DIR}" --parallel "$(nproc)" --target cbakey_fcitx5plugin

# Generate .deb via CPack.
(cd "${BUILD_DIR}" && cpack -G DEB)

DEB_FILE="$(ls "${BUILD_DIR}"/cbakey*.deb 2>/dev/null | head -1 || true)"
if [[ -z "${DEB_FILE}" ]]; then
    echo "ERROR: .deb not found in ${BUILD_DIR}"
    exit 1
fi

echo ""
echo "=== Package built ==="
echo "  ${DEB_FILE}"
echo ""
echo "Install:"
echo "  sudo dpkg -i ${DEB_FILE}"
echo ""
echo "Verify (optional):"
echo "  dpkg-deb --info ${DEB_FILE}"
echo "  dpkg-deb --contents ${DEB_FILE}"
