#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
PLUGIN_PATH="${BUILD_DIR}/libcbakey.so"

if [[ ! -f "${PLUGIN_PATH}" ]]; then
  echo "Plugin not found at ${PLUGIN_PATH}"
  echo "Build first: cmake -S . -B build -G Ninja && cmake --build build"
  exit 1
fi

ADDON_DIR="${HOME}/.local/share/fcitx5/addon"
IM_DIR="${HOME}/.local/share/fcitx5/inputmethod"
LIB_DIR="${HOME}/.local/lib/fcitx5"
LIB_DIR_MULTIARCH="${HOME}/.local/lib/x86_64-linux-gnu/fcitx5"
CONF_DIR="${HOME}/.config/cbakey"

mkdir -p "${ADDON_DIR}" "${IM_DIR}" "${LIB_DIR}" "${LIB_DIR_MULTIARCH}" "${CONF_DIR}"

cp "${ROOT_DIR}/deploy/fcitx5/inputmethod/cbakey.conf" "${IM_DIR}/cbakey.conf"
cp "${PLUGIN_PATH}" "${LIB_DIR}/libcbakey.so"
cp "${PLUGIN_PATH}" "${LIB_DIR_MULTIARCH}/libcbakey.so"

# Use absolute library path to avoid distro-specific addon loader path mismatch.
cat > "${ADDON_DIR}/cbakey.conf" <<EOF
[Addon]
Name=CBAKey Vietnamese Input Method
Category=InputMethod
Version=0.1.0
Enabled=True
Library=${LIB_DIR}/libcbakey
Type=SharedLibrary
OnDemand=True
Configurable=True

[Dependencies]
0=core:5.0.0
EOF

if [[ ! -f "${CONF_DIR}/cbakey.conf" ]]; then
  cat > "${CONF_DIR}/cbakey.conf" <<'EOF'
method=telex
enable_user_dictionary=true
enable_static_expansion=true
toggle_hotkey=Ctrl+Alt+Z
fcitx5_preedit_mode=auto
EOF
fi

echo "Installed CBAKey plugin to user paths:"
echo "  - ${LIB_DIR}/libcbakey.so"
echo "  - ${LIB_DIR_MULTIARCH}/libcbakey.so"
echo "  - ${ADDON_DIR}/cbakey.conf"
echo "  - ${IM_DIR}/cbakey.conf"
echo ""
echo "Next:"
echo "1) Restart fcitx5: pkill fcitx5; fcitx5 -d"
echo "2) Open Fcitx5 Configuration and add input method: CBAKey"
