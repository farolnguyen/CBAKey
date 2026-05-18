# CBAKey — APT Repository nội bộ (M10.2)

Hướng dẫn dựng và maintain APT repository để phân phối CBAKey trong nội bộ qua `apt install`.

---

## Yêu cầu

**Máy chủ repo (server):**
- Linux amd64
- Các gói: `dpkg-dev`, `gpg`, `apt-utils`

```bash
sudo apt install dpkg-dev gpg apt-utils
```

- Một GPG key để ký repo (xem phần [Tạo GPG key](#1-tạo-gpg-key) bên dưới nếu chưa có)
- Web server hoặc file server để host repo (nginx, Apache, hoặc `python3 -m http.server`)

---

## Luồng tổng quan

```
build_deb.sh → .deb → setup_apt_repo.sh → apt_repo/ → web server → apt install (client)
```

---

## 1. Tạo GPG key (nếu chưa có)

```bash
gpg --batch --gen-key <<EOF
%no-protection
Key-Type: RSA
Key-Length: 4096
Subkey-Type: RSA
Subkey-Length: 4096
Name-Real: CBAKey Internal Signing
Name-Email: cbakey-sign@internal
Expire-Date: 2y
EOF

# Lấy fingerprint
gpg --list-keys cbakey-sign@internal
# Ghi lại fingerprint dài (40 ký tự hex) hoặc dùng email làm KEY_ID
```

> **Lưu ý bảo mật:** Private key dùng để ký repo phải được bảo vệ chặt. Không commit private key vào repo.

---

## 2. Build .deb và tạo repo

```bash
# Bước 1: Build .deb
bash scripts/build_deb.sh

# Bước 2: Tạo APT repo + ký
bash scripts/setup_apt_repo.sh \
    --repo-dir /var/www/apt/cbakey \
    --key-id cbakey-sign@internal
```

Script sẽ tạo cấu trúc:

```
apt_repo/
├── pool/main/
│   └── cbakey_0.1.0_amd64.deb
├── dists/stable/
│   ├── Release
│   ├── Release.gpg
│   ├── InRelease
│   └── main/binary-amd64/
│       ├── Packages
│       └── Packages.gz
└── cbakey-archive-keyring.gpg   ← public key để distribute cho client
```

---

## 3. Host repo

### Dev/test (tạm thời)

```bash
cd /var/www/apt/cbakey
python3 -m http.server 8080
```

### Production (nginx)

```nginx
server {
    listen 80;
    server_name apt.internal;
    root /var/www/apt/cbakey;
    autoindex on;
    location / {
        try_files $uri $uri/ =404;
    }
}
```

---

## 4. Cấu hình máy client

```bash
# 1. Thêm public key
sudo mkdir -p /etc/apt/keyrings
sudo curl -fsSL http://apt.internal/cbakey-archive-keyring.gpg \
    -o /etc/apt/keyrings/cbakey-archive-keyring.gpg

# 2. Thêm sources.list
echo "deb [arch=amd64 signed-by=/etc/apt/keyrings/cbakey-archive-keyring.gpg] \
    http://apt.internal stable main" \
    | sudo tee /etc/apt/sources.list.d/cbakey.list

# 3. Cài đặt
sudo apt update
sudo apt install cbakey
```

---

## 5. Cập nhật repo khi có phiên bản mới

```bash
# Build .deb mới
bash scripts/build_deb.sh

# Chạy lại setup_apt_repo.sh — script tự copy .deb mới vào pool và regenerate Packages/Release
bash scripts/setup_apt_repo.sh \
    --repo-dir /var/www/apt/cbakey \
    --key-id cbakey-sign@internal
```

Máy client chạy `sudo apt update && sudo apt upgrade` sẽ nhận phiên bản mới.

---

## 6. Troubleshooting

### `apt update` báo lỗi NO_PUBKEY

Public key chưa được thêm vào máy client. Chạy lại bước 4.1.

### `apt update` báo `Release file ... is not valid yet`

Đồng hồ hệ thống lệch. Kiểm tra: `timedatectl status`. Đảm bảo NTP đồng bộ.

### `dpkg-scanpackages: command not found`

```bash
sudo apt install dpkg-dev
```

### Gói cũ vẫn còn trong pool

Xóa thủ công rồi chạy lại script:
```bash
rm /var/www/apt/cbakey/pool/main/cbakey_<old-version>_amd64.deb
bash scripts/setup_apt_repo.sh --repo-dir /var/www/apt/cbakey --key-id cbakey-sign@internal
```

---

## Liên quan

- [deploy/RUNBOOK.md](../deploy/RUNBOOK.md) — cài từ .deb trực tiếp (không cần APT repo)
- [scripts/build_deb.sh](../scripts/build_deb.sh) — build .deb package
- [scripts/setup_apt_repo.sh](../scripts/setup_apt_repo.sh) — script tạo repo
