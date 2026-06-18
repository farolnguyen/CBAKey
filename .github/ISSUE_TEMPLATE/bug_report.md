---
name: Báo lỗi
about: Gõ sai / crash / hành vi không mong muốn
labels: bug
---

## Khu vực bị lỗi

<!-- Chọn một hoặc nhiều, xóa dòng không liên quan -->
- [ ] Gõ tiếng Việt (Telex/VNI/VIQR/Simple Telex/Microsoft/Tốc ký/Free Layout...)
- [ ] Từ điển cá nhân / Gõ tắt (User Dictionary)
- [ ] Smart Templates
- [ ] Clipboard History
- [ ] Chụp màn hình (Screenshot)
- [ ] Settings GUI
- [ ] Cài đặt / gỡ cài đặt
- [ ] Khác: <!-- mô tả -->

## Mô tả lỗi

<!-- Mô tả ngắn gọn lỗi xảy ra -->

## Cách tái hiện

<!-- Nếu là lỗi gõ tiếng Việt, ghi rõ chuỗi phím đã gõ -->
```
ví dụ: t-h-u-o-c (Telex) → kỳ vọng: "thuốc", thực tế: "..."
```

1.
2.
3.

## Kỳ vọng vs Thực tế

| | Kỳ vọng | Thực tế |
|--|---------|---------|
| Preedit | | |
| Commit | | |

## Môi trường

- Version FarolKey: <!-- xem Settings → About, ví dụ v0.1.7 -->
- OS: <!-- Ubuntu 24.04 / Debian 12 / ... -->
- Desktop Environment: <!-- GNOME / KDE / XFCE ... -->
- Session: <!-- Wayland / X11 -->
- Fcitx5: <!-- fcitx5 --version -->
- Ứng dụng bị lỗi: <!-- VSCode / Chrome / LibreOffice / Terminal ... -->
- Kiểu gõ đang dùng: <!-- Telex / VNI / VIQR / Simple Telex / Microsoft / Tốc ký / Free Layout ... -->
- Cấu hình đặc biệt: <!-- Output encoding khác Unicode? Hotkey tùy chỉnh? -->

## Log

```
# journalctl --user -u fcitx5 -n 30
# hoặc: Settings → About → 📦 Export bug report log
```

## Severity

<!-- Chọn một: -->
- [ ] **S1 — Blocker**: không gõ được, crash, mất dữ liệu
- [ ] **S2 — Major**: sai hoàn toàn cho một nhóm ký tự / app phổ biến / tính năng không hoạt động
- [ ] **S3 — Minor**: lệch dấu / cursor trong một trường hợp cụ thể
- [ ] **S4 — Cosmetic**: hiển thị lạ nhưng không ảnh hưởng output
