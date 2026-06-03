# FarolKey VIQR — Internal Specification

Tài liệu nội bộ chốt toàn bộ rules trước khi implement `applyViqrTransform` (M14.3).

Tham chiếu: RFC 1456 (VIQR standard), tuy nhiên một số quyết định là FarolKey-specific.

---

## 1. Diacritic transforms

Áp dụng khi key đặc biệt theo sau base vowel/consonant trong preedit buffer.

| Input sequence | Output | Ghi chú |
|----------------|--------|---------|
| `a^` | â | circumflex |
| `e^` | ê | circumflex |
| `o^` | ô | circumflex |
| `a(` | ă | breve |
| `o+` | ơ | horn |
| `u+` | ư | horn |
| `dd` | đ | stroke — onset only |

**Các base letter KHÔNG có diacritic** (key đặc biệt → pass-through nếu base không hợp lệ):

| Invalid sequence | Hành vi |
|-----------------|---------|
| `e(`, `o(`, `i(`, `u(` | `(` pass-through (không có breve cho e/o/i/u) |
| `i^`, `y^`, `u^` | `^` pass-through (không có circumflex cho i/y/u) |
| `a+`, `e+`, `i+` | `+` pass-through (không có horn cho a/e/i) |

---

## 2. Tone marks (postfix)

Áp dụng trên vowel trong preedit buffer. M17 tone normalization tự di chuyển tone về đúng vị trí sau mỗi push.

| Key | Tone | Ký hiệu ngữ âm |
|-----|------|----------------|
| `'` | sắc  | acute (´) |
| `` ` `` | huyền | grave (`) |
| `?` | hỏi  | hook above (ả) |
| `~` | ngã  | tilde (~) |
| `.` | nặng | dot below (ạ) |
| (none) | ngang | flat |

---

## 3. Thứ tự diacritic + tone — cả hai chiều đều hợp lệ

FarolKey chấp nhận diacritic trước hoặc tone trước, cho cùng kết quả:

| Sequence | Output |
|----------|--------|
| `a^'` | ấ (circumflex trước, tone sau) |
| `a'^` | ấ (tone trước, diacritic sau) |
| `o+~` | ỡ (horn trước, ngã sau) |
| `o~+` | ỡ (ngã trước, horn sau) |
| `u+'` | ứ |
| `u'+'` | — (nếu tone đã có, `'` thứ hai → double-key escape ra `'` literal) |

**Cơ chế:** M17 `normalizeSyllableTonePlacement` chạy sau mỗi push/transform → tone tự di chuyển về đúng vị trí âm chính bất kể thứ tự nhập.

---

## 4. Double-key escape

Nhấn key đặc biệt **hai lần liên tiếp** → emit ký tự literal thay vì transform.
Cơ chế tương tự Telex repeat-key (`aa`→`â`, `aaa`→`aa`).

| Input | Output | Giải thích |
|-------|--------|-----------|
| `''` | `'` literal | sắc key × 2 |
| ` `` ` | `` ` `` literal | huyền key × 2 |
| `??` | `?` literal | hỏi key × 2 |
| `~~` | `~` literal | ngã key × 2 |
| `..` | `.` literal | nặng key × 2 |
| `^^` | `^` literal | circumflex key × 2 |
| `((` | `(` literal | breve key × 2 |
| `++` | `+` literal | horn key × 2 |
| `ddd` | `dd` literal | `dd`→`đ` rồi `d` thứ 3 revert về `dd` |

**Lưu ý `ddd`:** `d` + `d` → `đ` (transform consumed), `d` thứ ba kích hoạt revert → kết quả là `dd` literal (hai chữ d), `d` thứ ba bị consume làm escape signal.

---

## 5. Remove-diacritics key: `\` (backslash)

**FarolKey-specific** — RFC 1456 không định nghĩa.

- `\` khi có preedit → xóa toàn bộ diacritics + tone của buffer, giữ nguyên base letters
- `\` khi preedit rỗng → pass-through (emit `\` literal)
- `\\` → `\` literal (double-key escape)

| Ví dụ | Output |
|-------|--------|
| `a^\` | `a` (xóa circumflex) |
| `ba^'\` | `ba` (xóa cả diacritic lẫn tone) |
| `ba\` | `ba` (xóa tone nếu có) |
| `\` (preedit rỗng) | `\` literal |
| `\\` | `\` literal |

---

## 6. Uppercase

Diacritic và tone áp dụng đúng với uppercase base letter:

| Input | Output |
|-------|--------|
| `A^` | Â |
| `A^'` | Ấ |
| `DD` | Đ |
| `DD'` | không hợp lệ (đ không có tone) → `Đ'` (tone pass-through) |
| `O+'` | Ớ |

---

## 7. Không hỗ trợ `w` shortcut kiểu Telex

VIQR trong FarolKey dùng RFC thuần. `ow`/`uw` **không** được alias thành `ơ`/`ư`.

| Input | VIQR output | Ghi chú |
|-------|------------|---------|
| `ow` | `ow` (literal) | Dùng `o+` để ra ơ |
| `uw` | `uw` (literal) | Dùng `u+` để ra ư |

---

## 8. U-medial context — gõ coda trước, tone sau

Các từ có 'u' medial trước nguyên âm chính (vd: **huấn**, **tuân**, **xuân**, **quân**): do cơ chế `selectToneVowelIndex` dùng `hasCoda` để xác định vị trí tone trong nucleus "ua", nên:

- Nếu gõ tone **trước coda**: `h-u-a-^-'` → 'u' có thể nhận tone sai (vì chưa có coda, offset="ua without coda"=0 → tone trên 'u').
- Nếu gõ tone **sau coda**: `h-u-a-^-n-'` → tone đặt đúng trên 'â' (coda hiện diện → offset="ua with coda"=1).

**Khuyến nghị:** với syllable có coda (huấn, tuân, quân, xuân…), gõ coda trước rồi mới gõ tone mark:

| Đúng | Sai (có thể lỗi) |
|------|----------------|
| `hua^n'` → huấn | `hua^'n` → có thể ra húân |
| `tuaa n'` → tuân (nếu dùng aa→â) | `tua^'n` → có thể ra túân |

Đây là limitation của tone-placement inference khi buffer chưa có coda. Sẽ được cải thiện ở M17.5.

---

## 9. Các edge case khác

### 8.1 `^` sau `ô` (double diacritic)

`o^^` → `o^` được escape về `ô` literal rồi `^` thứ hai pass-through? Không — cơ chế là:
- `o^` → `ô` (transform applied, repeatTransformState active với key `^`)
- `^` thứ hai → double-key revert → `o^` literal (emit `^` sau `o`, không ra `ô`)

### 8.2 Tone trên phụ âm đơn

`b'`, `n'`, `m.` v.v. — tone key khi buffer là phụ âm thuần → không có vowel → transform fails → tone key pass-through literal.

### 8.3 `+` và `(` trong context không phải VIQR base

Khi `+` hoặc `(` đứng độc lập không theo sau base vowel hợp lệ → pass-through literal. Ví dụ:
- `b+` → `b` rồi `+` literal
- `3+` → `3` commit rồi `+` literal

### 8.4 VIQR trong English mode

EN mode không apply VIQR transform. `dd`, `a^`, `'` v.v. đều là literal trong EN mode.

---

## 9. Corpus coverage yêu cầu (M14.4)

`corpus/final/viqr.jsonl` — tối thiểu 100 cases, bao gồm:

| Tag | Ví dụ sequences |
|-----|----------------|
| `basic_diacritic` | `a^`→â, `a(`→ă, `o+`→ơ, `u+`→ư, `dd`→đ |
| `tone_only` | `ba'`→bá, `ba.`→bạ, `vie^.t`→việt |
| `diacritic_then_tone` | `a^'`→ấ, `o^~`→ỗ, `u+.`→ự |
| `tone_then_diacritic` | `a'^`→ấ, `o~^`→ỗ, `u.+`→ự |
| `escape` | `''`→', `..`→., `^^`→^, `ddd`→dd |
| `remove_diacritics` | `ba^'\`→ba, `huo+\`→huo |
| `multisyllable` | `vie^.t`→việt, `dduo+`ng`→đường |
| `uppercase` | `A^'`→Ấ, `DD`→Đ |
| `invalid_diacritic` | `e(`→e( literal, `i^`→i^ literal |
| `no_preedit_escape` | `\` rỗng→\ literal |

---

## 10. Non-goals (không có trong VIQR FarolKey)

- `ow`/`uw` alias (Telex-style)
- Passthrough mode
- Bất kỳ extension nào ngoài RFC 1456 trừ `\` remove-diacritics và double-key escape
