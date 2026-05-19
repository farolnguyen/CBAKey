#!/usr/bin/env bash
# FarolKey — Setup APT repository nội bộ
#
# Sử dụng: bash scripts/setup_apt_repo.sh [--repo-dir DIR] [--key-id KEY_ID]
#
# Yêu cầu: dpkg-dev, gpg, apt-utils (cài: sudo apt install dpkg-dev gpg apt-utils)
#
# Luồng:
#   1. Build .deb (nếu chưa có)
#   2. Tạo cấu trúc APT repo tại REPO_DIR
#   3. Ký Packages + Release bằng GPG key chỉ định
#   4. In hướng dẫn cấu hình máy client

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

# --- Defaults ---
REPO_DIR="${REPO_DIR:-$PROJECT_ROOT/apt_repo}"
KEY_ID="${KEY_ID:-}"        # GPG key fingerprint hoặc email
DEB_DIR="$PROJECT_ROOT/build_deb"
CODENAME="stable"
ARCH="amd64"
COMPONENT="main"

# --- Parse args ---
while [[ $# -gt 0 ]]; do
    case $1 in
        --repo-dir) REPO_DIR="$2"; shift 2 ;;
        --key-id)   KEY_ID="$2";   shift 2 ;;
        --help)
            echo "Usage: $0 [--repo-dir DIR] [--key-id KEYID]"
            echo "  --repo-dir  Thư mục đầu ra cho APT repo (mặc định: apt_repo/)"
            echo "  --key-id    GPG key fingerprint hoặc email để ký repo"
            exit 0 ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

# --- Kiểm tra công cụ ---
for tool in dpkg-scanpackages gpg apt-ftparchive; do
    if ! command -v "$tool" &>/dev/null; then
        echo "Lỗi: '$tool' chưa cài. Chạy: sudo apt install dpkg-dev gpg apt-utils"
        exit 1
    fi
done

# --- Build .deb nếu chưa có ---
DEB_FILE="$(ls "$DEB_DIR"/farolkey_*.deb 2>/dev/null | sort -V | tail -1 || true)"
if [[ -z "$DEB_FILE" ]]; then
    echo "==> .deb chưa có, chạy build_deb.sh..."
    bash "$SCRIPT_DIR/build_deb.sh"
    DEB_FILE="$(ls "$DEB_DIR"/farolkey_*.deb | sort -V | tail -1)"
fi
echo "==> Dùng gói: $DEB_FILE"

# --- Cấu trúc repo ---
POOL_DIR="$REPO_DIR/pool/$COMPONENT"
DISTS_DIR="$REPO_DIR/dists/$CODENAME/$COMPONENT/binary-$ARCH"
mkdir -p "$POOL_DIR" "$DISTS_DIR"

cp -f "$DEB_FILE" "$POOL_DIR/"

# --- Packages ---
echo "==> Tạo Packages..."
(cd "$REPO_DIR" && dpkg-scanpackages "pool/$COMPONENT" /dev/null > "dists/$CODENAME/$COMPONENT/binary-$ARCH/Packages" 2>/dev/null)
gzip -k -f "$DISTS_DIR/Packages"

# --- Release ---
echo "==> Tạo Release..."
cat > "$REPO_DIR/dists/$CODENAME/Release" <<EOF
Origin: FarolKey Internal
Label: FarolKey
Codename: $CODENAME
Architectures: $ARCH
Components: $COMPONENT
Description: FarolKey Vietnamese IME (internal build)
Date: $(date -u '+%a, %d %b %Y %H:%M:%S UTC')
EOF

(cd "$REPO_DIR/dists/$CODENAME" && apt-ftparchive release . >> Release)

# --- Ký repo ---
if [[ -n "$KEY_ID" ]]; then
    echo "==> Ký Release bằng GPG key: $KEY_ID"
    gpg --default-key "$KEY_ID" \
        --armor --detach-sign \
        --output "$REPO_DIR/dists/$CODENAME/Release.gpg" \
        "$REPO_DIR/dists/$CODENAME/Release"
    gpg --default-key "$KEY_ID" \
        --clearsign \
        --output "$REPO_DIR/dists/$CODENAME/InRelease" \
        "$REPO_DIR/dists/$CODENAME/Release"
    echo "==> Đã ký: Release.gpg + InRelease"
else
    echo ""
    echo "CẢNH BÁO: --key-id không được cung cấp. Repo chưa được ký."
    echo "  Để ký sau: gpg --armor --detach-sign -o dists/$CODENAME/Release.gpg dists/$CODENAME/Release"
    echo ""
fi

# --- Export public key ---
if [[ -n "$KEY_ID" ]]; then
    gpg --armor --export "$KEY_ID" > "$REPO_DIR/farolkey-archive-keyring.gpg"
    echo "==> Public key: $REPO_DIR/farolkey-archive-keyring.gpg"
fi

# --- Tóm tắt ---
echo ""
echo "======================================"
echo " APT repo tạo xong tại: $REPO_DIR"
echo "======================================"
echo ""
echo "Để serve repo (tạm thời, dev):"
echo "  cd $REPO_DIR && python3 -m http.server 8080"
echo ""
echo "Cấu hình máy client (thay <server> bằng IP/hostname thực):"
echo ""
echo "  # Copy public key về máy client:"
echo "  sudo curl -fsSL http://<server>:8080/farolkey-archive-keyring.gpg \\"
echo "    -o /etc/apt/keyrings/farolkey-archive-keyring.gpg"
echo ""
echo "  # Thêm sources.list:"
echo "  echo 'deb [arch=amd64 signed-by=/etc/apt/keyrings/farolkey-archive-keyring.gpg] \\"
echo "    http://<server>:8080 $CODENAME $COMPONENT' \\"
echo "    | sudo tee /etc/apt/sources.list.d/farolkey.list"
echo ""
echo "  sudo apt update"
echo "  sudo apt install farolkey"
