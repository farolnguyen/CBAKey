---
name: Báo lỗi
about: Gõ sai / crash / hành vi không mong muốn
labels: bug
---

## Mô tả lỗi

<!-- Mô tả ngắn gọn lỗi xảy ra -->

## Cách tái hiện

Chuỗi gõ (key sequence):
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

- OS: <!-- Ubuntu 24.04 / Debian 12 / ... -->
- Session: <!-- Wayland / X11 -->
- Fcitx5: <!-- fcitx5 --version -->
- Ứng dụng bị lỗi: <!-- VSCode / Chrome / LibreOffice / Terminal ... -->
- Phương thức gõ: <!-- Telex / VNI -->
- Cấu hình đặc biệt: <!-- fcitx5_preedit_mode=? / fcitx5_committed_rewrite=? -->

## Log

```
# journalctl --user -u fcitx5 -n 30
```

## Severity

<!-- Chọn một: -->
- [ ] **S1 — Blocker**: không gõ được, crash, mất dữ liệu
- [ ] **S2 — Major**: sai hoàn toàn cho một nhóm ký tự / app phổ biến
- [ ] **S3 — Minor**: lệch dấu / cursor trong một trường hợp cụ thể
- [ ] **S4 — Cosmetic**: hiển thị lạ nhưng không ảnh hưởng output
