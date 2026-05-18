# CBAKey — Deployment Runbook

## Yêu cầu hệ thống

- Linux amd64 (Ubuntu 22.04+ / Debian 12+ khuyến nghị)
- Fcitx5 ≥ 5.0 đã cài và đang chạy
- Không cần gỡ bộ gõ khác (CBAKey là addon riêng, không xung đột)

---

## 1. Cài đặt từ .deb (khuyến nghị cho production)

### Bước 1 — Build .deb

```bash
git clone <repo-url> CBAKey && cd CBAKey
bash scripts/build_deb.sh
# .deb sẽ ở build_deb/cbakey_0.1.0_amd64.deb
```

### Bước 2 — Cài đặt

```bash
sudo dpkg -i build_deb/cbakey_0.1.0_amd64.deb
# Nếu báo thiếu phụ thuộc (unconfigured):
sudo apt-get install -f
```

Hoặc gộp một lệnh (apt cài luôn phụ thuộc): `sudo apt install ./build_deb/cbakey_0.1.0_amd64.deb`  
(trên Ubuntu/Debian mới; đường dẫn phải có `./` vì là file `.deb` local.)

### Bước 3 — Kích hoạt trong Fcitx5

```bash
# Restart Fcitx5
pkill fcitx5 || true; sleep 0.5; fcitx5 -d

# Mở cấu hình Fcitx5
fcitx5-configtool
# → "Input Method" tab → "+" → tìm "CBAKey" → Add
```

### Bước 4 — Kiểm tra hoạt động

1. Mở bất kỳ ứng dụng có ô nhập text (gedit, Firefox address bar, Terminal)
2. Nhấn `Ctrl+Alt+Z` để chuyển sang chế độ tiếng Việt (hoặc dùng Fcitx5 systray)
3. Gõ thử: `xin ` → "xin ", `chaof ` → "chào "
4. Gõ Telex: `vieejt nam` → "việt nam"

---

## 2. Cài đặt local (dành cho dev/test)

```bash
# Build Debug
cmake -S . -B build && cmake --build build --parallel

# Cài vào ~/.local
bash scripts/install_local_fcitx5.sh

# Restart Fcitx5
bash scripts/restart_fcitx5.sh
```

---

## 3. Gỡ cài đặt

### Gỡ .deb

```bash
sudo dpkg -r cbakey
pkill fcitx5 || true; sleep 0.5; fcitx5 -d
```

### Gỡ local install

```bash
bash scripts/uninstall_local_fcitx5.sh
bash scripts/restart_fcitx5.sh
```

---

## 4. Cấu hình

File cấu hình: `~/.config/cbakey/cbakey.conf`  
Được tạo tự động với giá trị mặc định lần cài đầu tiên.

```ini
method=telex                    # telex | vni
enable_user_dictionary=true     # bật từ điển cá nhân
toggle_hotkey=Ctrl+Alt+Z        # phím chuyển EN/VI
fcitx5_preedit_mode=auto        # auto | client | panel
fcitx5_committed_rewrite=false  # C1: sửa từ đã commit (thử nghiệm)
```

Sau khi sửa config: restart Fcitx5 để áp dụng.

### Từ điển cá nhân

File: `~/.config/cbakey/user_dict.json`

```json
{"trigger": "ko", "expansion": "không"}
{"trigger": "dk", "expansion": "được"}
{"trigger": "btv", "expansion": "Ban Tổ chức"}
```

Xem thêm: [docs/user_dict.md](../docs/user_dict.md)

---

## 5. Troubleshooting

### Lỗi: `cbakey depends on libfcitx5core10 | …` (không có gói nào được cài)

Trên Ubuntu 24.04 và một số bản Debian, thư viện lõi Fcitx5 có tên **`libfcitx5core7`**, không phải 8/9/10. **Hãy build lại `.deb` từ repo mới nhất** (đã khai báo thêm `libfcitx5core7` trong `Depends`).

Nếu gói đang ở trạng thái lỗi sau lần cài cũ:

```bash
sudo dpkg --remove --force-remove-reinstreq cbakey 2>/dev/null || true
sudo apt-get install -f
```

Sau đó build lại rồi cài:

```bash
bash scripts/build_deb.sh
sudo dpkg -i build_deb/cbakey_0.1.0_amd64.deb
# hoặc: sudo apt install ./build_deb/cbakey_0.1.0_amd64.deb
```

### Không thấy CBAKey trong danh sách input method

```bash
# Kiểm tra file addon
ls /usr/share/fcitx5/addon/cbakey.conf
ls /usr/lib/x86_64-linux-gnu/fcitx5/libcbakey.so

# Kiểm tra Fcitx5 log
journalctl --user -u fcitx5 -n 50
# hoặc
fcitx5 --verbose --debug 2>&1 | head -100
```

Nếu file thiếu: `sudo dpkg -i build_deb/cbakey_*.deb` lại.

### Bộ gõ không phản hồi phím

```bash
# Kiểm tra Fcitx5 đang chạy
pgrep fcitx5

# Kiểm tra CBAKey đã được chọn làm input method hiện tại
fcitx5-remote -n   # in ra tên input method đang active
```

Nếu không active: mở `fcitx5-configtool` và chuyển CBAKey lên đầu danh sách.

### Preedit bị lệch / ký tự hiển thị sai

- VSCode / Electron: thử gõ trực tiếp trong app — nếu dấu lệch là hành vi đã biết của client preedit. Workaround: dùng `fcitx5_preedit_mode=panel` trong config.
- LibreOffice: tương tự, thử `panel` mode nếu client mode có vấn đề.

### C1 (sửa từ đã commit) không hoạt động

Tính năng `fcitx5_committed_rewrite` mặc định tắt. Bật:
```ini
fcitx5_committed_rewrite=true
```
Lưu ý: chỉ hoạt động khi app hỗ trợ `SurroundingText`. Xem `docs/fcitx5_app_matrix.md`.

### Rollback về phiên bản cũ

```bash
sudo dpkg -i cbakey_<old-version>_amd64.deb
```

---

## 6. Kiểm tra sau cài đặt (checklist)

- [ ] `fcitx5-remote -n` trả về "cbakey" khi đang gõ tiếng Việt
- [ ] Gõ `chaof ` → "chào " trong một app
- [ ] Gõ `vni16 ` (VNI) → "vní " nếu method=vni  
- [ ] `Ctrl+Alt+Z` chuyển sang English mode, ký tự gõ thẳng
- [ ] Gõ trigger từ điển (nếu đã tạo file) → expansion đúng

---

## 7. Liên hệ / báo lỗi

- Tạo issue trên repo với: log Fcitx5, app bị lỗi, phiên bản OS, chuỗi gõ tái hiện lỗi.
- Tham khảo: `docs/fcitx5_app_matrix.md` (danh sách app đã test).
