# CBAKey — Hướng dẫn sử dụng

Bộ gõ tiếng Việt cho Linux (Fcitx5), hỗ trợ Telex và VNI, chạy trên Ubuntu/Debian với Wayland và X11.

---

## Mục lục

1. [Cài đặt](#1-cài-đặt)
2. [Kích hoạt trong Fcitx5](#2-kích-hoạt-trong-fcitx5)
3. [Gõ thử](#3-gõ-thử)
4. [Tùy chỉnh](#4-tùy-chỉnh)
5. [Từ điển cá nhân & Gõ tắt](#5-từ-điển-cá-nhân--gõ-tắt)
6. [Lịch sử Clipboard](#6-lịch-sử-clipboard)
7. [Smart Templates — Mẫu gõ động](#7-smart-templates--mẫu-gõ-động)
8. [Tính năng nâng cao](#8-tính-năng-nâng-cao)
9. [Câu hỏi thường gặp](#9-câu-hỏi-thường-gặp)
10. [Báo lỗi](#10-báo-lỗi)

---

## 1. Cài đặt

### Yêu cầu

- Linux amd64 (Ubuntu 22.04+ hoặc Debian 12+)
- Fcitx5 đã cài và đang chạy
- Không cần gỡ bộ gõ khác — CBAKey là addon riêng

### Bước 1 — Build gói .deb

```bash
git clone <repo-url> CBAKey
cd CBAKey
bash scripts/build_deb.sh
```

Gói sẽ ở: `build_deb/cbakey_0.1.0_amd64.deb`

### Bước 2 — Cài đặt

```bash
sudo apt install ./build_deb/cbakey_0.1.0_amd64.deb
```

> Dùng `apt install ./...` (có dấu `./`) để apt tự cài luôn các gói phụ thuộc.

---

## 2. Kích hoạt trong Fcitx5

```bash
# Restart Fcitx5
pkill fcitx5 || true; sleep 0.5; fcitx5 -d

# Mở cấu hình
fcitx5-configtool
```

Trong cửa sổ cấu hình: **Input Method** → nhấn **+** → tìm **CBAKey** → **Add**.

---

## 3. Gõ thử

Nhấn `Ctrl+Alt+Z` để bật chế độ tiếng Việt, sau đó thử:

**Telex:**

| Gõ | Kết quả |
|----|---------|
| `xin chao ` | `xin chào ` |
| `vieejt nam ` | `việt nam ` |
| `tieengs vieejt ` | `tiếng việt ` |
| `ddaatj daauj ` | `đặt dấu ` |

**VNI** (cần đổi sang VNI trong config trước):

| Gõ | Kết quả |
|----|---------|
| `xin cha2o ` | `xin chào ` |
| `vie65t nam ` | `việt nam ` |
| `tie61ng vie65t ` | `tiếng việt ` |

Nhấn `Ctrl+Alt+Z` lần nữa để về chế độ English.

---

## 4. Tùy chỉnh

File cấu hình: `~/.config/cbakey/cbakey.conf`  
Được tạo tự động lần đầu khi khởi động. Sau khi sửa, restart Fcitx5 để áp dụng.

```ini
# Phương thức gõ: telex hoặc vni
method=telex

# Bật/tắt từ điển cá nhân
enable_user_dictionary=true

# Phím tắt chuyển EN/VI
toggle_hotkey=Ctrl+Alt+Z

# Chế độ hiển thị preedit (mặc định auto là tốt nhất)
fcitx5_preedit_mode=auto
```

### Đổi sang VNI

Sửa `method=vni`, lưu file, restart Fcitx5.

### Đổi phím tắt chuyển EN/VI

Ví dụ đổi sang `Ctrl+Space`:

```ini
toggle_hotkey=Ctrl+Space
```

---

## 5. Từ điển cá nhân & Gõ tắt

Tạo file `~/.config/cbakey/user_dict.json` với các cặp trigger → expansion:

```json
{"trigger": "ko", "expansion": "không"}
{"trigger": "dk", "expansion": "được"}
{"trigger": "mk", "expansion": "mình"}
{"trigger": "btv", "expansion": "Ban Tổ chức"}
{"trigger": "bch", "expansion": "Ban Chấp Hành"}
{"trigger": "em", "expansion": "your.email@company.com"}
```

**Cách dùng:** gõ trigger (ví dụ `ko`) rồi nhấn `Space` hoặc `Enter` → expansion được chèn vào.

**Lưu ý:**
- Trigger phải là ASCII (a-z, 0-9, gạch dưới)
- Phím kích hoạt: `Space`, `Enter`, `Tab` — **không** phải dấu câu
- Dict chỉ hoạt động khi đang ở chế độ tiếng Việt
- Nếu không muốn expansion: xóa entry khỏi file

### Quản lý bằng giao diện (Dictionary Manager)

Mở từ systray CBAKey → **Dictionary Manager**:

- Tab **📖 Abbreviations**: xem, thêm, sửa, xóa gõ tắt; tìm kiếm; Import/Export CSV
- Bật/tắt từ điển bằng nút toggle **Enable user dictionary** ở trên cùng

**Gõ tắt theo mode:**

| Mode | Mô tả |
|------|-------|
| `VI` | Chỉ expand khi đang ở chế độ tiếng Việt |
| `EN` | Chỉ expand khi đang ở chế độ English |
| `Both` | Expand ở cả hai chế độ |

Xem thêm: [`docs/user_dict.md`](user_dict.md)

---

## 6. Lịch sử Clipboard

CBAKey tích hợp clipboard history kiểu Windows (Ctrl+Win+V).

### Mở popup

- **Ctrl+Super+V** (GNOME Wayland — đã đăng ký tự động khi cài)
- Hoặc click systray CBAKey → **Clipboard History**

### Tính năng

- Lưu lịch sử text và ảnh (tối đa 50 mục)
- Tìm kiếm trong popup
- Click/Enter để copy mục về clipboard → nhấn **Ctrl+V** để paste
- 📌 Pin để giữ mục không bị xóa
- Kéo header để di chuyển popup (không tự đóng khi kéo)
- "Clear all" xóa toàn bộ (trừ mục đã pin)

### Tự động khởi động

Daemon clipboard chạy tự động từ lúc login (qua `~/.config/autostart/cbakey-clipboard.desktop`).  
Lịch sử lưu tại: `~/.local/share/cbakey/clipboard_history.json`

### Auto-paste (nếu có cài tool)

| Session | Tool cần có |
|---------|------------|
| Wayland | `ydotool` + `ydotoold` hoặc `wtype` |
| X11 | `xdotool` |

Nếu chưa có tool, popup hiển thị `✓ Copied — press Ctrl+V to paste` trong 0.5 giây.

---

## 7. Smart Templates — Mẫu gõ động

Cho phép định nghĩa **template có tham số** — gõ trigger ngắn, engine sinh ra text phức tạp.

### Mở giao diện

Systray CBAKey → **Dictionary Manager** → tab **⚡ Smart Templates**

### Cách hoạt động

1. Định nghĩa template với pattern chứa `{varname}` làm placeholder
2. Bọc trigger trong `[` `]`, rồi nhấn `Space`/`Enter` → engine expand tự động

> **Tại sao cần `[...]`?** Cặp ngoặc vuông này tránh kích hoạt nhầm — ví dụ từ "update" chứa "date" nhưng sẽ **không** bị expand; chỉ `[date]` đứng riêng mới kích hoạt.

**Ví dụ:**

| Pattern | Expansion (Jinja2) | Gõ | Kết quả |
|---------|-------------------|----|---------|
| `{n}++` | `{{ numbered(n) }}` | `[5++]` + Space | `1↵2↵3↵4↵5` |
| `for{n}` | `for (let i = 1; i <= {{ n\|int }}; i++) {↵    ↵}` | `[for3]` + Space | vòng for JS |
| `date` | `{{ today() }}` | `[date]` + Space | `18/05/2026` |
| `hello{name}` | `Kính gửi {{ name\|title }},↵↵Thân gửi,` | `[helloderrick]` + Space | email template |

### Built-in functions

```
numbered(n, start=1, sep='\n')   → danh sách số từ start đến n
today(fmt='%d/%m/%Y')            → ngày hôm nay
now(fmt='%H:%M:%S')              → giờ hiện tại
repeat(text, n, sep='')          → lặp chuỗi n lần
```

Ngoài ra có thể dùng toàn bộ **Jinja2** filters và control flow:  
→ [Jinja2 Documentation](https://jinja.palletsprojects.com/)

### CLI nhanh

```bash
cbakey-template list                         # xem danh sách
cbakey-template add '{n}++' '{{ numbered(n) }}' --mode en
cbakey-template expand '5++'                # test expand (không cần [] ở CLI)
cbakey-template path                         # xem file path
```

> **Lưu ý:** cặp `[...]` chỉ cần khi gõ trong ứng dụng. Lệnh CLI `expand` dùng trigger trực tiếp (không cần ngoặc vuông).

File template: `~/.config/cbakey/templates.json`

### Mode

Giống gõ tắt: `en` / `vi` / `both` — kiểm soát template chỉ expand trong mode nào.

---

## 8. Tính năng nâng cao

### C1 — Sửa từ đã gõ (thử nghiệm)

Cho phép sửa âm tiết vừa commit mà không cần xóa và gõ lại.

**Ví dụ:** gõ `ban` → commit `ban` → con trỏ ngay sau → nhấn Telex `s` → sửa thành `bán`.

Bật trong config:
```ini
fcitx5_committed_rewrite=true
```

> Tính năng này ở trạng thái thử nghiệm. Hoạt động tốt trên hầu hết app GTK và Chromium. Trên VSCode (file `.txt`) và Google Docs có thể chưa ổn định.

---

## 9. Câu hỏi thường gặp

**Bộ gõ không xuất hiện sau khi cài?**

```bash
ls /usr/share/fcitx5/addon/cbakey.conf   # kiểm tra file addon
pkill fcitx5; sleep 0.5; fcitx5 -d       # restart Fcitx5
# Mở fcitx5-configtool → thêm CBAKey vào danh sách
```

**Phím tắt `Ctrl+Alt+Z` không chuyển được?**

Kiểm tra xem phím tắt có bị ứng dụng khác chiếm không. Đổi sang tổ hợp khác trong `cbakey.conf`.

**Dấu bị đặt sai chỗ?**

Thử đổi `fcitx5_preedit_mode=panel` trong config (chế độ chẩn đoán). Nếu panel đúng mà client sai, đây là giới hạn của app — ghi nhận và báo lỗi.

**Lỗi phụ thuộc khi `dpkg -i`?**

```bash
sudo apt install ./build_deb/cbakey_0.1.0_amd64.deb
# apt sẽ tự cài libfcitx5core và các phụ thuộc khác
```

**Muốn gỡ cài đặt?**

```bash
sudo dpkg -r cbakey
pkill fcitx5; sleep 0.5; fcitx5 -d
```

Xem thêm các trường hợp troubleshooting chi tiết: [`deploy/RUNBOOK.md`](../deploy/RUNBOOK.md)

---

## 10. Báo lỗi

Tạo issue trên repo với những thông tin sau:

- Phiên bản hệ điều hành (`lsb_release -a`)
- Phiên bản Fcitx5 (`fcitx5 --version`)
- Tên ứng dụng gặp lỗi
- Chuỗi gõ tái hiện lỗi (ví dụ: `tieengs` → kết quả mong đợi vs thực tế)
- Log Fcitx5: `journalctl --user -u fcitx5 -n 50`
