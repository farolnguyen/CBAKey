#!/usr/bin/env bash
# M10.1: Build a .deb package for FarolKey Fcitx5 plugin.
# Usage: bash scripts/build_deb.sh [build_dir]
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${1:-${ROOT_DIR}/build_deb}"

echo "=== FarolKey .deb packaging ==="
echo "Root: ${ROOT_DIR}"
echo "Build dir: ${BUILD_DIR}"

# Configure Release build with plugin + install targets.
cmake -S "${ROOT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DFAROLKEY_BUILD_FCITX5_PLUGIN=ON \
    -DFAROLKEY_BUILD_TESTS=OFF \
    -DFAROLKEY_BUILD_CORPUS_TOOLS=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr \
    "$@"

# Build the plugin.
cmake --build "${BUILD_DIR}" --parallel "$(nproc)" --target farolkey_fcitx5plugin

# Remove stale .deb files so the glob below always finds the freshly built one.
rm -f "${BUILD_DIR}"/farolkey*.deb

# Generate .deb via CPack.
(cd "${BUILD_DIR}" && cpack -G DEB)

# Read version directly from CMakeLists to construct the exact expected filename.
VERSION="$(grep -m1 'project(FarolKey VERSION' "${ROOT_DIR}/CMakeLists.txt" \
           | sed 's/.*VERSION \([0-9.]*\).*/\1/')"
DEB_FILE="${BUILD_DIR}/farolkey_${VERSION}_amd64.deb"
if [[ ! -f "${DEB_FILE}" ]]; then
    echo "ERROR: expected ${DEB_FILE} but file not found"
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
