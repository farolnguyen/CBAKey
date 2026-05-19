# FarolKey — Bộ gõ tiếng Việt cho Linux

Bộ gõ tiếng Việt (Telex / VNI) chạy trên Fcitx5, hỗ trợ Ubuntu/Debian với Wayland và X11.

---

## Tính năng chính

- **Telex & VNI** — đặt dấu chính xác theo mô hình âm tiết
- **Gõ tắt** — định nghĩa viết tắt riêng (vd: `ko` → `không`, `btv` → `Ban Tổ chức`)
- **Smart Templates** — mẫu gõ động với Jinja2 (vd: `[5++]` → danh sách 1–5)
- **Clipboard History** — lịch sử clipboard kiểu Windows (Ctrl+Super+V)
- **Screenshot** — chụp màn hình vùng chọn / toàn màn hình (Super+Shift+S, tự vào clipboard)
- **Dictionary Manager** — giao diện quản lý gõ tắt & template trực quan
- **C1** — sửa từ đã gõ mà không cần xóa lại

---

## Cài đặt

### Yêu cầu
- Ubuntu 22.04+ hoặc Debian 12+ (amd64)
- Fcitx5 đã được cài

### Cách 1 — Script tự động (khuyến nghị)

```bash
wget https://raw.githubusercontent.com/FarolNguyen/FarolKey/main/install.sh
bash install.sh
```

Script tự cài dependencies, tải và cài `.deb`, cấu hình Fcitx5.

### Cách 2 — Tải file .deb thủ công

1. Tải file `.deb` từ [Releases](https://github.com/FarolNguyen/FarolKey/releases/latest)
2. Cài dependencies:

```bash
sudo apt install fcitx5 fcitx5-config-qt \
    python3-gi python3-gi-cairo gir1.2-gtk-3.0 \
    gir1.2-gdkpixbuf-2.0 python3-jinja2
```

3. Cài package:

```bash
sudo apt install ./farolkey_0.1.0_amd64.deb
```

### Sau khi cài

1. **Đăng xuất và đăng nhập lại**
2. Mở **Fcitx5 Configuration** → **Input Method** → nhấn **+** → tìm **FarolKey** → Add
3. Nhấn `Ctrl+Alt+Z` để chuyển Vietnamese / English

---

## Sử dụng

### Gõ tiếng Việt

| Telex | VNI | Kết quả |
|-------|-----|---------|
| `vieejt` | `vie65t` | `việt` |
| `tieengs` | `tie61ng` | `tiếng` |
| `ddaatj` | `d9a61t` | `đặt` |

Nhấn `Ctrl+Alt+Z` để bật/tắt chế độ tiếng Việt.

### Gõ tắt

Mở **Dictionary Manager** từ systray → tab **📖 Abbreviations**.

```
ko  →  không
btv →  Ban Tổ chức
```

Gõ trigger rồi nhấn `Space` hoặc `Enter` để expand.

### Smart Templates

Mở **Dictionary Manager** → tab **⚡ Smart Templates**.

Bọc trigger trong `[` `]` để kích hoạt:

```
[5++]          →  1, 2, 3, 4, 5
[date]         →  18/05/2026
[for5]         →  for (let i = 1; i <= 5; i++) { ... }
[helloderrick] →  Kính gửi Derrick, ...
```

Ngôn ngữ template: [Jinja2](https://jinja.palletsprojects.com/)

### Clipboard History

Nhấn `Ctrl+Super+V` (hoặc systray → **Clipboard History**) để mở lịch sử clipboard.  
Hỗ trợ text và ảnh, tối đa 50 mục, có thể pin và tìm kiếm.

---

## Gỡ cài đặt

```bash
sudo dpkg -r farolkey
```

---

## Báo lỗi

Tạo issue tại: [github.com/FarolNguyen/FarolKey/issues](https://github.com/FarolNguyen/FarolKey/issues)

Vui lòng cung cấp:
- Phiên bản OS: `lsb_release -a`
- Phiên bản Fcitx5: `fcitx5 --version`
- Tên ứng dụng gặp lỗi
- Chuỗi gõ tái hiện lỗi
