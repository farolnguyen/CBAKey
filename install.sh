#!/usr/bin/env bash
# FarolKey — Installation script
# Usage: bash install.sh
#
# This script installs FarolKey and all required dependencies.
# Run as a regular user (not root) — sudo is called internally when needed.

set -euo pipefail

# ── Configuration ─────────────────────────────────────────────────────────────
# Update GITHUB_REPO and VERSION when releasing a new version.
GITHUB_REPO="farolnguyen/FarolKey"           # <--- update with your GitHub repo
VERSION="0.1.0"
DEB_NAME="farolkey_${VERSION}_amd64.deb"
DEB_URL="https://github.com/${GITHUB_REPO}/releases/download/v${VERSION}/${DEB_NAME}"

# ── Colors ────────────────────────────────────────────────────────────────────
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; BOLD='\033[1m'; NC='\033[0m'
ok()   { echo -e "${GREEN}✓${NC} $*"; }
warn() { echo -e "${YELLOW}!${NC} $*"; }
err()  { echo -e "${RED}✗${NC} $*"; exit 1; }
hdr()  { echo -e "\n${BOLD}=== $* ===${NC}"; }

# ── Checks ────────────────────────────────────────────────────────────────────
hdr "FarolKey Installer v${VERSION}"
echo "GitHub: https://github.com/${GITHUB_REPO}"
echo ""

[ "$(id -u)" -eq 0 ] && err "Do not run as root. Run as a normal user: bash install.sh"

if ! command -v apt &>/dev/null; then
    err "This script requires apt (Ubuntu/Debian). Other distros are not supported yet."
fi

ARCH=$(dpkg --print-architecture)
[ "$ARCH" != "amd64" ] && err "Only amd64 (x86_64) is supported. Detected: $ARCH"

# ── Step 1: Dependencies ──────────────────────────────────────────────────────
hdr "Step 1/4: Installing dependencies"

PKGS=(
    # Fcitx5 input method framework
    fcitx5
    fcitx5-config-qt

    # Python GTK3 bindings (Dictionary Manager + Clipboard History)
    python3-gi
    python3-gi-cairo
    gir1.2-gtk-3.0
    gir1.2-gdkpixbuf-2.0

    # Jinja2 template engine (Smart Templates)
    python3-jinja2

    # Screenshot tool dependencies
    gnome-screenshot      # Screen capture on GNOME Wayland
    grim                  # Screen capture on wlroots Wayland (Sway/Hyprland)
    slurp                 # Region selector on wlroots Wayland
    python3-pynput        # Global hotkey listener

    # X11 global hotkey (Clipboard History Ctrl+Super+V on X11 sessions)
    python3-xlib

    # Notifications
    libnotify-bin         # notify-send
)

MISSING=()
for pkg in "${PKGS[@]}"; do
    if ! dpkg -l "$pkg" &>/dev/null 2>&1 | grep -q "^ii"; then
        MISSING+=("$pkg")
    fi
done

if [ ${#MISSING[@]} -eq 0 ]; then
    ok "All dependencies already installed."
else
    echo "Installing: ${MISSING[*]}"
    # Prompt for sudo password once here — subsequent sudo calls reuse the cached credential.
    sudo -v
    sudo apt update -qq
    sudo apt install -y "${MISSING[@]}"
    ok "Dependencies installed."
fi

# ── Step 2: Download .deb ─────────────────────────────────────────────────────
hdr "Step 2/4: Downloading FarolKey package"

TMP_DEB="/tmp/${DEB_NAME}"

if command -v wget &>/dev/null; then
    wget -q --show-progress -O "$TMP_DEB" "$DEB_URL" || \
        err "Download failed. Check your internet connection or the URL:\n  $DEB_URL"
elif command -v curl &>/dev/null; then
    curl -L --progress-bar -o "$TMP_DEB" "$DEB_URL" || \
        err "Download failed. Check your internet connection or the URL:\n  $DEB_URL"
else
    err "Neither wget nor curl found. Install one and retry."
fi

ok "Downloaded: $TMP_DEB"

# ── Step 3: Install .deb ──────────────────────────────────────────────────────
hdr "Step 3/4: Installing FarolKey"

# Use apt instead of dpkg to auto-resolve any remaining library deps
sudo apt install -y "$TMP_DEB"
rm -f "$TMP_DEB"
ok "FarolKey installed."

# ── Step 4: Set Fcitx5 as default input method ────────────────────────────────
hdr "Step 4/4: Configuring input method"

if command -v im-config &>/dev/null; then
    im-config -n fcitx5
    ok "Fcitx5 set as default input method."
else
    warn "im-config not found. You may need to set Fcitx5 manually."
fi

# Ensure environment variables are set for the current user
IM_ENV_FILE="$HOME/.profile"
IM_BLOCK="# FarolKey/Fcitx5 input method environment"
if ! grep -q "FarolKey/Fcitx5" "$IM_ENV_FILE" 2>/dev/null; then
    cat >> "$IM_ENV_FILE" << 'EOF'

# FarolKey/Fcitx5 input method environment
export GTK_IM_MODULE=fcitx
export QT_IM_MODULE=fcitx
export XMODIFIERS=@im=fcitx
EOF
    ok "Environment variables added to ~/.profile"
else
    ok "Environment variables already configured."
fi

# ── Done ──────────────────────────────────────────────────────────────────────
echo ""
echo -e "${GREEN}${BOLD}╔══════════════════════════════════════════╗"
echo -e "║      FarolKey installed successfully!      ║"
echo -e "╚══════════════════════════════════════════╝${NC}"
echo ""
echo "Next steps:"
echo "  1. Log out and log back in (to apply input method settings)"
echo "  2. Open Fcitx5 Configuration:  fcitx5-configtool"
echo "     → Input Method tab → click '+' → search 'FarolKey' → Add"
echo "  3. Press Ctrl+Alt+Z to toggle Vietnamese / English mode"
echo ""
echo "Optional — open Dictionary Manager (abbreviations + smart templates):"
echo "  farolkey-dict-gui"
echo ""
echo "Documentation: https://github.com/${GITHUB_REPO}"
