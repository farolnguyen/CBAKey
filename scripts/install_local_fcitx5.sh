#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/build"
PLUGIN_PATH="${BUILD_DIR}/libfarolkey.so"

if [[ ! -f "${PLUGIN_PATH}" ]]; then
  echo "Plugin not found at ${PLUGIN_PATH}"
  echo "Build first: cmake -S . -B build -G Ninja && cmake --build build"
  exit 1
fi

ADDON_DIR="${HOME}/.local/share/fcitx5/addon"
IM_DIR="${HOME}/.local/share/fcitx5/inputmethod"
LIB_DIR="${HOME}/.local/lib/fcitx5"
LIB_DIR_MULTIARCH="${HOME}/.local/lib/x86_64-linux-gnu/fcitx5"
CONF_DIR="${HOME}/.config/farolkey"
ICON_DIR="${HOME}/.local/share/farolkey/icons"
BIN_DIR="${HOME}/.local/bin"

mkdir -p "${ADDON_DIR}" "${IM_DIR}" "${LIB_DIR}" "${LIB_DIR_MULTIARCH}" "${CONF_DIR}" "${ICON_DIR}" "${BIN_DIR}"

# Mode indicator icons (VI/EN badge SVGs).
cp "${ROOT_DIR}/deploy/icons/mode_vi.svg" "${ICON_DIR}/mode_vi.svg"
cp "${ROOT_DIR}/deploy/icons/mode_en.svg" "${ICON_DIR}/mode_en.svg"

# Dictionary Manager GUI
cp "${ROOT_DIR}/src/gui/farolkey-dict-gui" "${BIN_DIR}/farolkey-dict-gui"
chmod +x "${BIN_DIR}/farolkey-dict-gui"

# Smart Templates CLI
cp "${ROOT_DIR}/src/template/farolkey-template" "${BIN_DIR}/farolkey-template"
chmod +x "${BIN_DIR}/farolkey-template"

# Clipboard History daemon + autostart
cp "${ROOT_DIR}/src/clipboard/farolkey-clipboard" "${BIN_DIR}/farolkey-clipboard"
chmod +x "${BIN_DIR}/farolkey-clipboard"

AUTOSTART_DIR="${HOME}/.config/autostart"
mkdir -p "${AUTOSTART_DIR}"
cp "${ROOT_DIR}/deploy/autostart/farolkey-clipboard.desktop" "${AUTOSTART_DIR}/farolkey-clipboard.desktop"
echo "  - Autostart registered: ${AUTOSTART_DIR}/farolkey-clipboard.desktop"

# Start daemon now (if not already running) so clipboard history begins immediately
if ! pgrep -f "farolkey-clipboard --daemon" > /dev/null 2>&1; then
  nohup "${BIN_DIR}/farolkey-clipboard" --daemon > /dev/null 2>&1 &
  echo "  - Clipboard daemon started (PID $!)"
else
  echo "  - Clipboard daemon already running"
fi

# Register GNOME custom keybinding: Ctrl+Super+V → farolkey-clipboard --show
# Works on GNOME Wayland (and GNOME X11). Silent no-op on other desktops.
if command -v gsettings >/dev/null 2>&1; then
  _SCHEMA="org.gnome.settings-daemon.plugins.media-keys"
  _PATH="/org/gnome/settings-daemon/plugins/media-keys/custom-keybindings/farolkey-clipboard/"
  _EXISTING="$(gsettings get "${_SCHEMA}" custom-keybindings 2>/dev/null || echo '@as []')"
  if ! echo "${_EXISTING}" | grep -q "farolkey-clipboard"; then
    if [ "${_EXISTING}" = "@as []" ] || [ "${_EXISTING}" = "[]" ]; then
      _NEW="['${_PATH}']"
    else
      # Insert before closing ']'
      _NEW="$(echo "${_EXISTING}" | sed "s|]$|, '${_PATH}']|")"
    fi
    gsettings set "${_SCHEMA}" custom-keybindings "${_NEW}" 2>/dev/null || true
  fi
  gsettings set "${_SCHEMA}.custom-keybinding:${_PATH}" name    'FarolKey Clipboard History'  2>/dev/null || true
  gsettings set "${_SCHEMA}.custom-keybinding:${_PATH}" command 'farolkey-clipboard --show'   2>/dev/null || true
  gsettings set "${_SCHEMA}.custom-keybinding:${_PATH}" binding '<Control><Super>v'         2>/dev/null || true
  echo "  - GNOME keybinding registered: Ctrl+Super+V → farolkey-clipboard --show"
fi

cp "${ROOT_DIR}/deploy/fcitx5/inputmethod/farolkey.conf" "${IM_DIR}/farolkey.conf"
cp "${PLUGIN_PATH}" "${LIB_DIR}/libfarolkey.so"
cp "${PLUGIN_PATH}" "${LIB_DIR_MULTIARCH}/libfarolkey.so"

# Use absolute library path to avoid distro-specific addon loader path mismatch.
cat > "${ADDON_DIR}/farolkey.conf" <<EOF
[Addon]
Name=FarolKey Vietnamese Input Method
Category=InputMethod
Version=0.1.0
Enabled=True
Library=${LIB_DIR}/libfarolkey
Type=SharedLibrary
OnDemand=True
Configurable=True

[Dependencies]
0=core:5.0.0
EOF

if [[ ! -f "${CONF_DIR}/farolkey.conf" ]]; then
  cat > "${CONF_DIR}/farolkey.conf" <<'EOF'
method=telex
enable_user_dictionary=true
enable_static_expansion=true
toggle_hotkey=Ctrl+Alt+Z
fcitx5_preedit_mode=auto
EOF
fi

echo "Installed FarolKey plugin to user paths:"
echo "  - ${LIB_DIR}/libfarolkey.so"
echo "  - ${LIB_DIR_MULTIARCH}/libfarolkey.so"
echo "  - ${ADDON_DIR}/farolkey.conf"
echo "  - ${IM_DIR}/farolkey.conf"
echo ""
echo "Next:"
echo "1) Restart fcitx5 (recommended: wait until old process exits):"
echo "     bash ${ROOT_DIR}/scripts/restart_fcitx5.sh"
echo "   Or manually: pkill -x fcitx5; sleep 0.3; fcitx5 -d"
echo "2) Open Fcitx5 Configuration and add input method: FarolKey"
