# FarolKey Progress

Tài liệu này là **nguồn sự thật cho lộ trình** (chi tiết M0–M13): mọi thay đổi phạm vi kỹ thuật đáng kể cập nhật **tại đây trước** khi triển khai; **`Project_rules.md`** giữ nguyên tắc dài hạn — khi đổi phạm vi ảnh hưởng nguyên tắc chung, rà soát và chỉnh cả hai cho khớp.

---

## 1. Thực trạng & thành thật kỹ thuật

**Mức trưởng thành hiện tại:** codebase đã có **khung** C++ + plugin Fcitx5 + một **tập luật Telex/VNI tối thiểu** và một số hành vi preedit/commit. Đây **chưa phải** IME tiếng Việt đủ độ bao phủ để “thay thế” Bamboo / Unikey / fcitx5-unikey trong ý nghĩa thương mại hay parity đầy đủ.

**Vì sao các lỗi UI / thanh điệu vẫn lặp:** fix theo “case người dùng báo” chỉ là vá cục bộ. IME chuẩn cần **mô hình âm tiết**, **bảng đặt dấu đầy đủ**, **corpus kiểm thử**, và **ma trận ứng dụng** — những phần chưa được đầu tư đủ.

**Mục tiêu tài liệu này:** cố định một **lộ trình nhiều giai đoạn**, có **deliverable** và **exit criteria**, để triển khai theo tiến độ **chất lượng** thay vì đánh dấu phase “xong” quá sớm.

**Tiến độ roadmap (ước lượng 2026-05-14):** **~72%** trên phạm vi milestone **M0–M11** (không tính **M12/M13** wishlist). Cách ước: M0–M5, M8, M9.1, M10.1+M10.3 = đóng; M4 ≈ 85% (M4.3 grapheme còn mở); M6 ≈ 75% (code adapter + C1/M6.3b xong, **M6.4 matrix / M7 QA** chưa làm); M9 ≈ 50% (còn M9.2 profiling); M10 ≈ 67% (còn M10.2 APT); M11 = 0%. **Lưu ý:** % này là **tiến độ giai đoạn trong file roadmap**, không đồng nghĩa IME đã đạt “parity thương mại” so với đoạn *Thực trạng* phía trên.

---

## 2. Tầm nhìn sản phẩm (mức nghiêm túc)

- IME tiếng Việt trên **Linux (Fcitx5)**, ưu tiên **Ubuntu + GNOME**, **Wayland + X11**.
- **Parity hành vi** với các bộ gõ phổ biến cho nhóm tính năng đã cam kết (Telex/VNI, preedit ổn định, từ điển cục bộ, gõ tắt).
- **Ưu tiên:** đúng chính tả / đặt dấu / Unicode; sau đó là trải nghiệm và dấu cách / con trỏ cross-app.
- **Không gấp:** mỗi giai đoạn lớn chỉ “đóng” khi đạt exit criteria và có **regression** rõ ràng.

---

## 3. Nguyên tắc triển khai (để không lặp lại “phase ảo”)

1. **Không gộp “đã có code” với “đạt chuẩn IME”.** Checkbox chỉ tick khi có **test + tiêu chí thoát**.
2. **Corpus-first:** mọi thay đổi luật gõ phải đi kèm **test case** (file corpus có version, không chỉ fix tay một từ).
3. **Tách rõ:** `core` (ngôn ngữ học / luật) vs `adapter_fcitx5` (DBus/cursor/preedit/capability).
4. **Không tối ưu hóa che bug:** hiệu năng có ngân sách riêng sau khi semantics ổn định.
5. **Privacy:** giữ nguyên quy tắc strict — không log nội dung gõ (xem `docs/logging_policy.md`).

---

## 4. Chuẩn “parity” với bộ gõ tham chiếu (checklist)

Dùng làm **chuẩn nội bộ** khi claim “đủ thay thế” cho nhóm tính năng V1–V2:

| Hạng mục | Bamboo / Unikey / fcitx5-* | FarolKey (mục tiêu) |
|----------|---------------------------|-------------------|
| Telex đầy đủ (âm chính + đặt dấu + xử lý xung đột) | Cao | **Phải có bảng rule + test** |
| VNI đầy đủ (thứ tự phím, tone vs 6789) | Cao | **Phải có matrix test** |
| Đặt dấu đúng âm tiết (đa nguyên âm / ieu / qu / gi…) | Cao | **Phải có syllable model hoặc bảng đầy đủ** |
| Unicode NFC ổn định | Cao | **Policy + test NFD edge** |
| Preedit/cursor/underline trên GTK/Qt/Electron | Trung–cao | **Ma trận app + workaround có tài liệu** |
| Từ điền cục bộ + gõ tắt | Phổ biến | Theo phase chức năng |
| Cấu hình & đổi mode | Phổ biến | UI/config Fcitx + file config |

---

## 5. Định nghĩa milestone “đóng giai đoạn”

Một **major milestone** chỉ được đánh dấu hoàn thành khi:

- Có **exit criteria** trong bảng phase được thỏa mãn;
- Có **ít nhất một** trong: bộ test tự động mở rộng / báo cáo QA thủ công có checklist đã ký;
- Changelog (mục 10) ghi nhận phiên bản / ngày đóng milestone.

---

## 6. Lộ trình chi tiết (Major phases)

### Milestone M0 — Hạ tầng chất lượng & corpus ✅ CLOSED

**Mục tiêu:** mọi thay đổi luật ngôn ngữ đều có “chỗ đứng” kiểm chứng.

**Checkpoint quy ước M0 (corpus + runner, B2–C2 đã chốt):** xem [`corpus/README.md`](corpus/README.md) — định dạng JSON Lines, schema version, assert mặc định / trace / meta, song ngữ tóm tắt.

**Trạng thái hiện tại (2026-05-12):** đã có bộ canonical `corpus/final/`:
- `telex.jsonl`: **654** case
- `vni.jsonl`: **634** case
- `engine_meta.jsonl`: **17** case
- `farolkey_corpus_test` ưu tiên quét `corpus/final/` khi thư mục này tồn tại, và đang pass trên local.

| ID | Hạng mục | Deliverable | Exit criteria |
|----|-----------|-------------|----------------|
| M0.1 | **Corpus kiểm thử** | Thư mục **`corpus/`** chứa source JSONL + bộ canonical **`corpus/final/`** (`telex.jsonl`, `vni.jsonl`, `engine_meta.jsonl`): `sequence` → `expect`; spec chi tiết trong `corpus/README.md` | ≥ **500** câu Telex + ≥ **500** câu VNI (tối thiểu), có tag `tone`, `multisyllable`, `edge_qu`, `edge_gi`, … |
| M0.2 | **Runner** | GoogleTest đọc corpus (theo `corpus/README.md`) | CI/local `ctest` chạy full corpus trong giới hạn thời gian cho phép |
| M0.3 | **Tham chiếu hành vi** | `docs/behavior_reference.md` song ngữ (không copy mã nguồn licence khác): mô tả expectation sản phẩm | Danh sách case được chấp nhận làm spec nội bộ |
| M0.4 | **Phân tách phase cũ** | Bảng “Đã có prototype” vs “Đạt chuẩn production” | Không còn checklist major nào chỉ dựa smoke test |

---

### Milestone M1 — Mô hình âm tiết tiếng Việt (core ngôn ngữ học) ✅ CLOSED

**Mục tiêu:** đặt dấu và biến âm dựa trên **cấu trúc âm tiết**, không chỉ “quét buffer”.

**Trạng thái audit (2026-05-12):** chốt M1 ở mức **“hết weak user-visible”**. Canonical corpus hiện giữ được **35 strong / 0 weak / 8 missing** trong internal pattern audit; 8 pattern còn `missing` (`eu`, `ie`, `ya`, `ye`, `yê`, `uye`, `ieu`, `yeu`) hiện được xem là pattern trung gian / không user-visible rõ ràng, không còn là backlog regression thực dụng để chặn việc sang M2.

| ID | Hạng mục | Deliverable | Exit criteria |
|----|-----------|-------------|----------------|
| M1.1 | **Phân tích C-V optional** | Parser: onset / nucleus / coda (ít nhất cho một âm tiết liền kề trong buffer compose) | Test: `qu`, `gi`, `ng`, `nh`, `ch`, `tr`, `ph`, … không làm sai nucleus |
| M1.2 | **Nucleus đa nguyên âm** | Bảng đặt dấu đầy đủ (đôi / ba nguyên âm: `oa`,`ao`,`ieu`,`ươ`,`uya`, …) | Corpus M0 pass rate ≥ **98%** cho nhóm tag `tone_placement` |
| M1.3 | **Âm cuối & nhịp gõ** | Xử lý coda khi user gõ phụ âm cuối trước khi commit | Không phá vỡ preedit khi gõ `ch`, `nh`, `ng`, `c`, `t`, … |
| M1.4 | **Tài liệu nội bộ** | `docs/vi_syllable_model.md` mô tả giới hạn (single-buffer vs multi-word) | Review nội bộ |

---

### Milestone M2 — Telex: luật đầy đủ & xung đột ✅ CLOSED

**Trạng thái hiện tại (2026-05-12):** **M2 đã hoàn tất** ở góc nhìn user-visible. Telex repeat-key disambiguation, `z` remove-diacritics, interaction-order hai chiều, backslash raw-escape, và direct-family matrix coverage cho `aa/ee/oo/ow/uw/dd` đều đã có regression trong smoke + canonical corpus. Vòng matrix audit cuối không phát hiện thêm gap đáng kể; điểm còn lại chỉ là câu hỏi spec-level về việc có chấp nhận thêm standalone `ww` alias hay không, không phải blocker theo spec hiện tại.

| ID | Hạng mục | Deliverable | Exit criteria |
|----|-----------|-------------|----------------|
| M2.1 | **Bảng transform** | Đủ `aa, ee, oo, ow, uw, dd, ww`, v.v. theo spec đã chọn | Corpus Telex pass ≥ **98%** |
| M2.2 | **Tone Telex** | `sf rx j` không nhầm với ký tự literal khi cần | Test matrix cho “escape” / raw mode (nếu có) |
| M2.3 | **Thứ tự tương tác** | Tone sau biến âm và ngược lại | Matrix test |

---

### Milestone M3 — VNI: luật đầy đủ & thứ tự phím ✅ CLOSED

**Trạng thái hiện tại (2026-05-12):** **M3 đã hoàn tất ở góc nhìn user-visible.** Sau 8 slice chính cho `M3.1/M3.2` (raw-escape, `uoi/ưo`, `uou/ươu`, `ue/uu`, `uay`, `VNI 0`, repeated `1..5`, và spec tạm thời cho repeated `6/7/8/9`), breadth-audit VNI đã được nới rộng thêm bằng một batch representative-order cho các family raw/full-word còn ít regression (`quoc16`, `tieng16`, `thuyen26`, `nguyen46`, `hieu16`, `yeu16`, `d9eu26`, `toi16`, `boi37`, `gui37`, `hua17`, `buou177`). Vòng closeout checklist cuối cùng chỉ còn lộ ra một gap thật ở nhánh `d9` gõ muộn cho family `đều`; nhánh này đã được sửa bằng cách cho `9` retroactively đổi onset `d` của âm tiết cuối, từ đó khóa thêm `deu962 -> đều` và `deu296 -> đều` vào smoke + corpus regression. Sau khi sửa, closeout audit trên các alternate-order candidate sinh từ curated VNI transform+tone cho kết quả **0 fail**. Canonical corpus hiện là **Telex 654 / VNI 634 / engine_meta 3**, full `ctest` xanh, `farolkey_m1_fallback_audit` vẫn giữ **0 tone fallback / 0 transform fallback** với `literal_supported: 51`, và không còn `fallback_samples`. Với trạng thái này, M3 không còn backlog user-visible đáng kể và có thể xem là đã đóng.

| ID | Hạng mục | Deliverable | Exit criteria |
|----|-----------|-------------|----------------|
| M3.1 | **Tone vs chữ số** | Phân biệt digit tone vs digit literal trong ngữ cảnh | Corpus VNI pass ≥ **98%** |
| M3.2 | **6789 + tone** | Luật “giữ tone index khi đổi hàng nguyên âm” áp dụng nhất quán | Không leak digit trên **≥ N** case regression |
| M3.3 | **Edge nucleus** | `ươ`, `iê`, … sau parser M1 | Gộp vào corpus |

---

### Milestone M4 — Unicode & chuẩn hóa ✅ M4.1/M4.2 CLOSED | M4.3 (grapheme) hoãn

**Trạng thái hiện tại (2026-05-12):** **M4 đã chốt xong `M4.1/M4.2` ở mức hiện tại, còn `M4.3` vẫn mở.** Core hiện đã có helper `normalizeVietnameseNfc()` để gom các chuỗi decomposed tiếng Việt quen thuộc (`ă â ê ô ơ ư` + tone marks) về dạng precomposed trước khi áp luật hoặc encode lại ra UTF-8. Helper này đã được nối vào các nhánh transform/remove-diacritics của `vi_syllable` và đường decode/encode của `Engine`, đồng thời smoke tests đã khóa nhiều case decomposed đại diện hơn, gồm cả các tổ hợp đảo thứ tự mark và nhiều hàng dấu khác nhau như `a\u0302\u0301 -> ấ`, `a\u0301\u0302 -> ấ`, `a\u0306\u0323 -> ặ`, `a\u0323\u0306 -> ặ`, `o\u0303\u0302 -> ỗ`, `u\u0301\u031b -> ứ`, `thu + e\u0302\u0301 -> thuế`, `nguye\u0302\u0303n -> nguyễn`, cùng regression remove-diacritics trên input decomposed. Ở mức output, smoke tests cũng đã khóa byte-level regression cho các ký tự đại diện như `ấ`, `ằ`, `đ`, `ự` ở cả Telex và VNI. Full `ctest` vẫn xanh và `farolkey_m1_fallback_audit` vẫn giữ **0 tone fallback / 0 transform fallback**. Vòng audit ở ranh giới engine/plugin cho thấy adapter Fcitx5 hiện chỉ đẩy **key event ASCII một chiều** vào core (`keySymToUTF8(...).size() == 1`) và không có đường `surrounding text` hay text Unicode từ app quay ngược vào engine; đồng thời `fcitx::Text::setCursor()` dùng **byte offset**, nên `preedit.size()` ở adapter hiện là đúng theo contract. Với bằng chứng này, `M4.1` (NFC cố định) và `M4.2` (đầu vào NFD) có thể xem là đã đạt mức đóng thực dụng cho phạm vi hiện tại; phần còn mở của M4 chủ yếu là `M4.3` / grapheme semantics.

| ID | Hạng mục | Deliverable | Exit criteria |
|----|-----------|-------------|----------------|
| M4.1 | **NFC cố định** | Toàn pipeline commit/preedit NFC | Test so sánh byte với kỳ vọng |
| M4.2 | **Đầu vào NFD** | Chuẩn hóa trước khi áp luật | Không double-dấu |
| M4.3 | **Grapheme** | Xử lý đơn vị người dùng (không chỉ `pop_back` byte) | Test surrogate/combining (nếu có đường nhập) |

---

### Milestone M5 — State machine compose & undo ✅ CLOSED

**Trạng thái hiện tại (2026-05-12):** **M5 đã đi qua 3 nhịp cho delete semantics + boundary + smart-boundary bảo thủ.** Sau quyết định sản phẩm mới, `M5.1` đã được mở lại và hiện đã đổi runtime sang hướng **visible deletion**: `BackSpace` và `DeleteForward` trong lúc compose không còn rollback theo lịch sử key event nữa, mà sẽ **xóa hẳn đơn vị đang thấy ở cuối preedit**. Điều này đưa hành vi lại gần các bộ gõ phổ biến như `fcitx5-unikey` hơn và loại bỏ path `ế -> ê -> e` / `ấ -> â` vốn không còn được xem là spec đích. Các nhịp còn lại của M5 vẫn giữ giá trị: engine theo **explicit boundary policy** cho các boundary cứng (`space`, `tab/newline`, dấu câu, navigation), và slice smart-boundary đầu tiên vẫn chạy theo hướng **bảo thủ trong core**: chỉ auto-commit prefix khi toàn bộ buffer tách được thành nhiều âm tiết hợp lệ và suffix cuối đã đủ giống một âm tiết mới có onset riêng. Vì vậy, các case như `xincha -> commit "xin" + preedit "cha"`, `xinchao -> commit "xin" + preedit "chao"`, `thuo61cba -> commit "thuốc" + preedit "ba"` vẫn là baseline đúng của M5 sau khi semantics xóa đã được chỉnh lại.

| ID | Hạng mục | Deliverable | Exit criteria |
|----|-----------|-------------|----------------|
| M5.1 | **Delete / Backspace semantics** | Định nghĩa lại xóa trong lúc compose theo **đơn vị người dùng đang thấy** (ưu tiên ký tự/grapheme hiển thị), không còn theo lịch sử key event | Tài liệu + test + regression mới thay thế expectation cũ |
| M5.2 | **Đa âm tiết** | Policy: space / commit / boundary | Hành vi nhất quán với spec |

---

### Milestone M6 — Adapter Fcitx5 (production-grade) 🔄 CODE COMPLETE | testing/QA hoãn

**Mục tiêu:** lỗi “cursor / gạch chân / client drawing” được xử lý **theo capability** và có fallback có tài liệu.

**Trạng thái hiện tại (2026-05-12):** M6 hiện đã có **slice đầu tiên của `M6.1/M6.2`**, nhưng định hướng sản phẩm đã được siết lại sau test tay: **`panel` không còn là target UX chính**. Hướng đi mong muốn từ đây là tập trung 100% vào `auto/client` và làm cho đường `clientPreedit` ổn trong các app thực tế; `panel` nhiều nhất chỉ còn giá trị như **đường chẩn đoán tạm thời** để cô lập bug hoặc có workaround ngắn hạn trong giai đoạn debug. Adapter hiện đã bắt đầu đọc `InputContext::capabilityFlags()` và chọn kênh hiển thị theo capability; config runtime cũng có `fcitx5_preedit_mode=auto|client|panel` để benchmark có kiểm soát. Sau test tay đầu tiên, `client/auto` trên VSCode/Electron đã lộ bug `Enter` double-commit; nhánh này hiện đã có fix adapter-side theo hướng: nếu route thực tế là `clientPreedit`, adapter chỉ commit phần preedit và `forward` phím `Enter` gốc thay vì gửi `\n` hai lần. Phần này đã có smoke regression ở layer strategy và full `ctest` đang xanh, nhưng vẫn cần retest tay ở app thật để xác nhận đã chặn đúng bug VSCode/Electron. Một số app (đặc biệt Electron) vẫn có thể render preedit lệch dấu; phần đó được ưu tiên ghi nhận trong app-matrix như limitation client hơn là bug core. M6 **chưa đóng** và nhiệm vụ ưu tiên hiện tại vẫn là tiếp tục dùng matrix (ưu tiên `auto/client`) để sửa cho đường `client/auto` đạt mức dùng được thực tế. Slice **active-preedit anchoring / click-away** đã có **bản v1** trong adapter (xem bullet kỹ thuật ngay dưới scope); vẫn cần benchmark lại trên app thật sau khi cài plugin mới.

**Scope bổ sung kế tiếp trong M6 (2026-05-12):** thêm một slice riêng cho **active-preedit anchoring khi caret di chuyển**.

- **Bản chất bug:** đường adapter hiện tại vẫn là “preedit ngoài tài liệu, commit thật khi flush/reset”. Nếu app cập nhật caret trước rồi mới phát `reset`, text đang compose sẽ bị chèn vào vị trí caret mới (kiểu lỗi Bamboo) hoặc bị mất nếu chỉ clear state (kiểu lỗi Unikey).
- **Mục tiêu UX:** khi user đang gõ dở một từ rồi click sang chỗ khác, từ đó phải được chốt/giữ đúng ở vị trí cũ; không được biến mất, và cũng không được “copy” sang vị trí mới.
- **Ràng buộc kỹ thuật:** vá chỉ `commitString()` tại `reset()` không đủ khi surrounding đổi hoặc caret nhảy ngược; bản v1 neo bằng snapshot `SurroundingText` + synthetic `Left`/`Right` trong điều kiện hẹp; các trường hợp khác vẫn fallback `commitString` thuần và có thể cần slice bổ sung sau benchmark.
- **Quan hệ với `M6.3`:** slice này dùng chung primitive `surrounding text`, nhưng **khác mục tiêu** với `M6.3`. `M6.3` là rewrite trên **từ đã commit**, còn slice mới này là xử lý **preedit còn sống** khi caret nhảy chỗ.
- **Trạng thái quyết định hiện tại:** đã có **slice v1** trong adapter: khi `reset/deactivate` cần flush preedit, nếu client vẫn báo `SurroundingText` hợp lệ và buffer surrounding **không đổi** so với snapshot lúc còn preedit, đồng thời caret chỉ nhảy dọc theo buffer (tiến hoặc lùi, tối đa 256 bước UCS-4) và không có selection, adapter sẽ gửi synthetic mũi tên quanh `commitString` để neo commit về anchor cũ (`Left`→commit→`Right` khi caret nhảy về phía sau; `Right`→commit→`Left` khi nhảy về phía trước). **Giới hạn thực tế:** một số client (ví dụ LibreOffice Writer, VSCode/Electron) khi click có thể **đổi luôn chuỗi surrounding** trước khi IME nhận `reset`, khiến điều kiện “buffer không đổi” thất bại và vẫn fallback `commitString` thuần tại caret mới — hành vi “chữ đi theo con trỏ” vẫn có thể xuất hiện cho tới khi có slice bổ sung (so khớp nới lỏng, `deleteSurroundingText`, hoặc primitive khác tùy audit từng app).

**Scope mới cho M6.3 (2026-05-12):** thêm một phase riêng cho **sửa lại từ đã commit theo caret** thay vì buộc người dùng xóa và gõ lại từ đầu.

- **Gate triển khai:** trước khi mở rộng `M6.3` ra production, phải có **implementation plan riêng** — hiện đã có bản chốt tối thiểu tại [`docs/m6_3a_implementation_plan.md`](docs/m6_3a_implementation_plan.md) (2026-05-13); code prototype **M6.3a** (**C1** — *commit rồi vẫn sửa được*) đã bám theo plan này và sẽ được siết tiếp sau benchmark app.
- **M6.3a (C1) — Caret ở cuối từ đã commit:** khi con trỏ đứng ngay sau một từ tiếng Việt vừa commit, adapter có thể đọc `surrounding text` bên trái con trỏ, nhận diện biên từ gần nhất, parse lại từ đó trong core, rồi dùng `replace-surrounding`/API tương đương để thay đúng từ cũ bằng từ mới sau khi user gõ tone/transform key. Đây là scope ưu tiên vì sát nhu cầu thực tế và rủi ro thấp hơn.
- **M6.3b — Caret nằm giữa từ:** chỉ mở sau khi `M6.3a` ổn. Nhánh này cần giữ đúng vị trí caret sau rewrite, xử lý boundary hai phía, Unicode offset, selection, và fallback theo app nếu surrounding-text không đáng tin.
- **Nguyên tắc bật tính năng:** chỉ enable ở app/capability đã audit rõ; nếu thiếu `surrounding text` hoặc `replace-surrounding` ổn định thì phải fallback về hành vi hiện tại, không đoán mò.
- **Ràng buộc regression:** không được phá `M5` (undo, smart-boundary, raw escape, repeat disambiguation), không rewrite bừa lên text không parse được thành target tiếng Việt hợp lệ, và phải có app-matrix riêng trước khi coi là production-ready.

**Draft chi tiết cho M6.3a (caret-at-end)**

1. **Điều kiện trigger tối thiểu**
   - Chỉ xét khi đang ở `InputMode::Vietnamese`.
   - Không có `preedit` đang mở, không có selection, và caret đứng **ngay sau** token bên trái (không chen khoảng trắng hay dấu tách từ ở giữa).
   - Key mới gõ phải thuộc nhóm **special key** của method hiện tại:
     - Telex: tone / transform / remove-diacritics / raw-escape family.
     - VNI: `0..9` và raw-escape family.
   - Token bên trái phải trích ra được từ `surrounding text` theo boundary bảo thủ (ví dụ dừng ở whitespace hoặc punctuation rõ ràng).

2. **Luồng xử lý bảo thủ**
   - Adapter đọc token bên trái caret.
   - Core nhận token này như một “rewrite candidate”, dựng lại state tạm rồi áp key mới giống như đang sửa tiếp một từ chưa commit.
   - Chỉ khi kết quả rewrite **khác token cũ** và vẫn parse được hợp lệ thì mới gọi `replace-surrounding`.
   - Nếu bất kỳ bước nào thiếu bằng chứng, adapter fallback hoàn toàn về hành vi hiện tại: key mới đi theo đường nhập bình thường, không rewrite gì.

3. **Non-goals của M6.3a**
   - Không sửa khi caret đang nằm **giữa** từ.
   - Không sửa qua nhiều token hoặc qua dấu câu / khoảng trắng.
   - Không xử lý selection, multi-caret, hay app không báo surrounding-text đáng tin.
   - Không cố “đoán ý người dùng” khi token bên trái không ra ứng viên tiếng Việt đủ chắc.

4. **Representative spec examples**
   - `ban|` + Telex `s` -> rewrite thành `bán|`.
   - `ban|` + Telex `z` -> rewrite thành `ban|` nếu trước đó từ đang có dấu; nếu token đã không còn gì để xóa thì fallback theo policy literal hiện hành.
   - `thuoc|` + VNI `6`, rồi `1` -> rewrite thành `thuốc|`.
   - `ban |` + Telex `s` -> **không rewrite** `ban`; `s` đi như input mới vì caret không còn dính cuối token cũ.
   - `abc|` + special key nhưng token không parse được thành ứng viên rewrite an toàn -> **không rewrite**.

5. **Deliverable kỹ thuật mong muốn**
   - Adapter helper để lấy token bên trái caret + offset replace.
   - Core helper kiểu “rewrite committed token with one more key” để không phải dựng hack riêng trong adapter.
   - Contract rõ cho success / no-op / unsafe-fallback.
   - Test matrix riêng cho capability: app nào đọc được surrounding-text, app nào replace-surrounding giữ caret đúng.

6. **Exit criteria đề xuất cho M6.3a**
   - Có regression tự động cho rewrite-success và rewrite-refuse.
   - Có ít nhất một app benchmark GTK và một app benchmark Electron/Chromium được test tay.
   - Giữ đúng caret ở cuối token sau rewrite.
   - Không phá đường nhập hiện tại khi capability không có hoặc trả dữ liệu lỗi.

7. **Kết luận audit primitive Fcitx5 (2026-05-12)**
   - Fcitx5 **có đủ primitive nền** để thử `M6.3a`: `InputContext::capabilityFlags()`, `CapabilityFlag::SurroundingText`, cache `InputContext::surroundingText()`, và lệnh `deleteSurroundingText(offset, size)`.
   - `surroundingText()` dùng **offset theo ký tự UCS4**, trong khi preedit cursor của `fcitx::Text` hiện vẫn là **byte offset**. Vì vậy nếu đi vào rewrite path, adapter phải tách rõ hai hệ quy chiếu này, không tái dùng logic `preedit.size()` hiện tại.
   - Hiện không có “replace surrounding” convenience API ngay trong adapter hiện tại; đường rewrite thực tế nhiều khả năng sẽ là **delete vùng cũ rồi commit chuỗi mới**, có thể tận dụng `commitStringWithCursor()` nếu client báo hỗ trợ `CapabilityFlag::CommitStringWithCursor`.
   - Với kết quả này, **`caret-at-end` là scope khả thi thực dụng**, còn `caret-inside-word` vẫn nên để sau vì sẽ phụ thuộc mạnh hơn vào việc giữ caret chính xác sau rewrite và độ tin cậy của client capability.
   - Adapter FarolKey hiện đã **đọc `SurroundingText` tối thiểu** cho slice click-away v1 (snapshot + điều kiện nudge). **Prototype M6.3a (C1) v1 (2026-05-13–14):** `Engine::tryRewriteCommittedSyllable` + `tryApplyCommittedSyllableRewrite` trong plugin Fcitx5 (`deleteSurroundingText` + `commitString`), có smoke `farolkey_m6_3a_rewrite_smoke_test`; **VSCode `.txt` + Google Docs** ghi *known limitation / hoãn* (xem `docs/fcitx5_app_matrix.md`). **✅ M6.3b (CommitStringWithCursor, 2026-05-14):** khi client báo `CapabilityFlag::CommitStringWithCursor`, dùng `ic->commitStringWithCursor(token, token.size())` thay `commitString` trong C1 — cursor neo tường minh sau token, giảm cursor-drift VSCode/Electron; fallback về `commitString` khi không có capability; regression thêm vào `farolkey_m6_3a_rewrite_smoke_test`; `ctest` 9/9 xanh. Ma trận app (M6.4) hoãn sang testing phase.

| ID | Hạng mục | Deliverable | Exit criteria |
|----|-----------|-------------|----------------|
| M6.1 | **Client-preedit ổn định** | Ưu tiên `clientPreedit` như đường UX chính; `panel` tối đa chỉ dùng để debug/cô lập bug | Ma trận: ít nhất 4 app benchmark có note, và `auto/client` không còn blocker lớn ở app mục tiêu |
| M6.2 | **Capability flags cho đường chính** | Đọc `CapabilityFlag` phục vụ route `auto/client`; không mở rộng `panel` thành mode sản phẩm | Không crash / không blank preedit, có rule rõ khi nào `auto` đi được |
| M6.3 | **Surrounding-text rewrite theo caret** ✅ | M6.3a (C1 caret-at-end) + M6.3b (CommitStringWithCursor): code complete 2026-05-14. Caret-inside-word hoãn. | ✅ Code done; testing/QA hoãn |
| M6.4 | **Danh mục workaround** ⏸ | File `docs/fcitx5_app_matrix.md` (skeleton có sẵn) | Hoãn sang testing phase |

---

### Milestone M7 — Ma trận tương thích ứng dụng (QA) ⏸ HOÃN — mở lại sau M8 code xong

| ID | Hạng mục | Deliverable | Exit criteria |
|----|-----------|-------------|----------------|
| M7.1 | Chrome / Firefox | Checklist: preedit, commit, shortcut | Signed QA |
| M7.2 | VSCode / Electron | Same |  |
| M7.3 | Terminal (GTK/VTE, …) | Same |  |
| M7.4 | LibreOffice | Same |  |
| M7.5 | Wayland vs X11 | Same suite trên cả hai | Ghi chênh lệch |

---

### Milestone M8 — Từ điển cục bộ & gõ tắt ✅ M8.1/M8.2/M8.3 CLOSED

Nền tảng cho từ điển và expansion **tĩnh**; **macro / gõ tắt do user định nghĩa** có UI riêng được lên lịch ở **M13** (có thể tái sử dụng / mở rộng schema **M8.1**).

**Kế hoạch triển khai M8 (2026-05-14):**

**M8.1 — User dictionary format & loader**
- File: `~/.config/farolkey/user_dict.json` (hoặc `.jsonl`)
- Schema: `{"trigger": "btv", "expansion": "Ban Tổ chức"}`
- Loader: `src/core/user_dict.cpp` + `include/farolkey/core/user_dict.h`
- Config: `RuntimeConfig` có field `user_dict_path`
- Gate: load tại startup, bỏ qua nếu file thiếu, log warning không crash

**M8.2 — Static expansion lookup trong engine**
- Lookup xảy ra tại `processKey()` khi boundary key (Space/Enter) được nhấn
- Nếu preedit buffer khớp đúng `trigger`, thay bằng `expansion` rồi commit
- Chỉ khớp khi toàn bộ preedit == trigger (không khớp substring)
- Không áp expansion khi đang ở `InputMode::English`

**M8.3 — Conflict policy**
- Nếu trigger đồng thời là syllable tiếng Việt hợp lệ → **tiếng Việt ưu tiên**
- Expansion chỉ kích hoạt cho trigger không parse được thành syllable tiếng Việt
- Hoặc: user có thể đặt flag `"force": true` để override
- Tài liệu trong `docs/user_dict.md`

**Thứ tự code:** M8.1 → M8.2 → M8.3

| ID | Hạng mục | Deliverable | Exit criteria |
|----|-----------|-------------|----------------|
| M8.1 | User dictionary format | `user_dict.h/cpp`, schema JSON, loader an toàn | Load không crash khi file thiếu/lỗi; parse đúng schema |
| M8.2 | Static expansion | Lookup + commit khi Space/Enter sau trigger match | Smoke test: `btv→Ban Tổ chức`, `ko→không`, không ảnh hưởng Telex/VNI bình thường |
| M8.3 | Conflict policy | Tài liệu + code guard: VN syllable > dict; flag `force` | Không expansion nhầm `ban`, `toi`, `mai`, v.v. |

---

### Milestone M9 — Hiệu năng & ngân sách ✅ M9.1/M9.2 CLOSED

**M9.2 (2026-05-14):** Tối ưu `findStableComposeSplit` — thay thế `segmentWholeBufferWithPreference` (DP với nested `vector<vector<SyllableSpan>>` allocations) bằng reachability DP dùng `vector<bool>` + backward scan. Kết quả: long-word path **2.80 → 2.59 µs/key (−7%)**, Telex **1.37 → 1.24 µs/key (−9%)**. Worst-case 2.59 µs vẫn trong budget <10 µs; còn 0.59 µs tới target 2 µs nhưng hotspot còn lại (`baseSlice` O(n²)) không đáng refactor với buffer 5-7 chars. Tất cả 10/10 tests xanh.

| ID | Hạng mục | Deliverable | Exit criteria |
|----|-----------|-------------|----------------|
| M9.1 | Benchmark | Giữ `farolkey_keypress_benchmark`, thêm scenario corpus | Ngân sách µs/key đã ghi |
| M9.2 | Profiling | Tối ưu `findStableComposeSplit` (vector<bool> DP) | Long-word −7%, corpus pass, no regression |

---

### Milestone M10 — Đóng gói & triển khai doanh nghiệp ✅ M10.1/M10.3 CLOSED | M10.2 CODE DONE (chờ infra)

**M10.2 (2026-05-14):** `scripts/setup_apt_repo.sh` (build pool, Packages, Release, GPG sign, hướng dẫn client) + `docs/apt_repo_guide.md` (GPG key, nginx config, troubleshooting). Script đầy đủ nhưng chưa pilot vì cần máy chủ ký gói thực.

| ID | Hạng mục | Deliverable | Exit criteria |
|----|-----------|-------------|----------------|
| M10.1 | `.deb` | Package reproducible | Lintian tối thiểu acceptable |
| M10.2 | APT / repo nội bộ | `scripts/setup_apt_repo.sh` + `docs/apt_repo_guide.md` | Pilot ≥ X máy (pending infra) |
| M10.3 | Rollback / troubleshoot | Runbook | |

---

### Milestone M11 — Tài liệu người dùng & vòng phản hồi ✅ M11.1 CLOSED | M11.2 còn mở

**M11.1 (2026-05-14):** `docs/user_guide.md` — hướng dẫn cài .deb, kích hoạt Fcitx5, gõ thử Telex/VNI, tùy chỉnh config, từ điển cá nhân, C1, FAQ, báo lỗi.

| ID | Hạng mục | Deliverable | Exit criteria |
|----|-----------|-------------|----------------|
| M11.1 | User guide | `docs/user_guide.md`: cài đặt, toggle, user_dict, FAQ | ✅ CLOSED 2026-05-14 |
| M11.2 | Bug intake | Template issue + severity | |

---

### Milestone M12 — Lịch sử clipboard (kiểu Windows + V) 🔄 Phase 1 DONE

**Mục tiêu:** lịch sử clipboard toàn hệ thống (text + rich + hình khi khả thể), UI chọn bằng chuột, ghim mục, và cấu hình độ sâu lịch sử — **tách biệt** luận lý gõ tiếng Việt trong `farolkey_core`; triển khai dưới dạng **addon / module Fcitx5** (hoặc repo con) để không phá kiến trúc IME hiện tại.

**Vì sao không gộp vào M6–M7:** đây là tính năng **desktop shell / clipboard protocol** (X11 vs Wayland, MIME, ảnh, quyền riêng tư), không phải syllable/preedit; gộp chung sẽ kéo scope và rủi ro bảo trì.

**Ràng buộc thực tế:** hành vi clipboard khác nhau rõ giữa **Wayland** (portal / compositor) và **X11**; ảnh cần pipeline decode/preview và giới hạn bộ nhớ; lịch sử dài đòi hỏi policy **xoá / mã hóa / không log** nội dung nhạy cảm (đối chiếu `docs/logging_policy.md`).

| ID | Hạng mục | Deliverable | Exit criteria |
|----|-----------|-------------|----------------|
| M12.1 | Kiến trúc & phạm vi | `docs/m12_clipboard_design.md`: Python+GTK3 daemon, Fcitx5 systray trigger, privacy policy | ✅ DONE 2026-05-18 |
| M12.2 | Model dữ liệu | `ClipboardHistory`: queue 50, JSONL persist, pin, atomic save | ✅ DONE 2026-05-18 |
| M12.3 | UI/UX | `ClipboardPopup`: GTK3 search + ListBox + pin/delete + keyboard nav | ✅ DONE 2026-05-18 |
| M12.4 | Wayland/X11 | GNOME gsettings keybinding + X11 python-xlib + Unix socket IPC | ✅ DONE 2026-05-18 |
| M12.5 | Đóng gói | CMakeLists + install script; Fcitx5 systray; `farolkey-clipboard` in PATH | ✅ DONE 2026-05-18 |
| M12.6 | Windows-style redesign | No focus-out close, draggable, bottom-right pos, CSS, Clear all | ✅ DONE 2026-05-18 |
| M12.7 | Image support | PNG store + thumbnail + set_image on select | ✅ DONE 2026-05-18 |
| M12.8 | Auto-paste | `_try_auto_paste()`: ydotool/wtype (Wayland) + xdotool (X11); fallback: "✓ Copied" footer | ✅ DONE 2026-05-18 |
| M12.9 | Daemon autostart | `.desktop` → `~/.config/autostart/`; install script start daemon ngay | ✅ DONE 2026-05-18 |
| M12.10 | Real-time popup update | `map` signal capture + `set_popup_notify` callback → popup refresh khi clipboard thay đổi | ✅ DONE 2026-05-18 |

---

### Milestone M13 — Gõ tắt / macro do người dùng định nghĩa ✅ M13.1–M13.5 CLOSED

**Triển khai (2026-05-14):**
- **Schema:** Mở rộng `UserDictEntry` với `AbbrevMode` enum (`Vi`/`En`/`Both`); field `abbrev_mode` trong JSONL — backward-compatible với M8 (thiếu field → mặc định `Vi`). Cùng file `user_dict.json`, không tách.
- **Engine (M13.2/M13.3):** `commitWithSuffix` chỉ expand `Vi`/`Both` trong Vietnamese mode; `lookupEnglishAbbrev()` cho adapter dùng với `En`/`Both` entries qua surrounding-text path; `setPasswordField(bool)` block expansion khi password/sensitive field.
- **Adapter:** `activate()` detect `CapabilityFlag::Password | Sensitive` → `bridge.setPasswordField(true)`.
- **CLI tool `farolkey-abbrev` (M13.4/M13.5):** `list`, `add <t> <e> [--mode]`, `remove`, `export [file]`, `import <file> [--merge]`, `path`; atomic write (`.tmp`→rename) + backup (`.bak`); duplicate warning on `import --merge`.
- **Smoke tests:** 10 cases trong `farolkey_m13_abbrev_smoke_test`; 11/11 tests xanh.
- **En-mode expansion:** `En`/`Both` entries trong English mode dùng surrounding-text rewrite (adapter-side, cùng path M6.3a) — không thay đổi immediate-passthrough behavior của English mode.

| ID | Hạng mục | Deliverable | Exit criteria |
|----|-----------|-------------|----------------|
| M13.1 | Schema + ADR | `AbbrevMode` enum + `abbrev_mode` field; cùng file M8.1 | ✅ backward-compat, 10 schema tests |
| M13.2 | Engine lookup | Hash O(1) + `AbbrevMode` filter + `lookupEnglishAbbrev` | ✅ tests pass |
| M13.3 | Semantics | Vi: Space/Enter/Tab; En: adapter surrounding-text; password guard | ✅ |
| M13.4 | CLI `farolkey-abbrev` | list/add/remove/export/import + path | ✅ binary `farolkey-abbrev` |
| M13.5 | Import/export | Atomic write, `.bak`, duplicate warning | ✅ smoke test |

---

### Milestone M15 — Smart Templates (Parametric Macro / “Mẫu gõ động”) ⏳ PLANNED

**Ý tưởng cốt lõi:** User định nghĩa template có **tham số động** — giống SCSS mixin. Khi gõ trigger khớp pattern (có số/text trước keyword), engine thay thế bằng expansion có tính toán qua **Jinja2**.

**Ví dụ điển hình:**
```
{n}++    →  {% for i in range(1, n|int+1) %}{{ i }}\n{% endfor %}
           Gõ: 100++ → “1\n2\n3\n...\n100”

for{n}   →  for (let i = 1; i <= {{ n }}; i++) {\n    \n}
           Gõ: for5 → vòng for với i từ 1 đến 5

hello{name} → Kính gửi {{ name|title }},\n\nThân gửi,
           Gõ: helloderrick → email template

date     → {{ today('%d/%m/%Y') }}
           Gõ: date → “18/05/2026”
```

**Kiến trúc chốt (sau clarification với user):**

| Quyết định | Lựa chọn | Lý do |
|-----------|---------|-------|
| Template engine | **Jinja2** (Python) | Mạnh, linh hoạt, không hardcode |
| Chạy ở đâu | **Python subprocess** `farolkey-template expand` | C++ engine spawn, nhận kết quả qua stdout |
| Commit kết quả | **wl-copy + auto-paste** (M12.8) | Hỗ trợ multi-line, works với mọi app |
| Trigger syntax | **`{varname}`** trong pattern | Dễ đọc, giao diện rõ ràng |
| Mode | **Per-template** `mode: en/vi/both` | Linh hoạt như M13 abbrev_mode |
| Storage | Cùng `user_dict.json` | Không thêm file mới |

**Built-in Jinja2 context (tự động inject vào mọi template):**
```python
today(fmt='%d/%m/%Y')   # ngày hôm nay
now(fmt='%H:%M:%S')     # giờ hiện tại
range(n, start=1, sep='\n')  # numbered list
repeat(text, n, sep='')      # lặp chuỗi n lần
# + toàn bộ Jinja2 built-in filters: upper, lower, title, int, float, trim, ...
```

**Slices:**

| ID | Hạng mục | Deliverable | Exit criteria |
|----|-----------|-------------|----------------|
| M15.1 | Schema | `templates.json` riêng (JSONL): `pattern`, `expansion` (Jinja2), `mode` | ✅ DONE 2026-05-18 |
| M15.2 | CLI `farolkey-template` | `list/add/remove/expand/test/path`; `_decode_escapes` cho CLI args | ✅ DONE 2026-05-18 |
| M15.3 | Pattern matcher | `_pattern_to_regex`: `{var}` → named capture group; `find_match()` filter by mode | ✅ DONE 2026-05-18 |
| M15.4 | Jinja2 expansion | `SandboxedEnvironment` + globals: `numbered`, `today`, `now`, `repeat` + all Jinja2 filters | ✅ DONE 2026-05-18 |
| M15.5 | Engine integration (C++) | `tryParametricExpand()` via `popen`; hook vào `commitWithSuffix` (VI) + `flushEnBuffer` (EN); `enableSmartTemplates` config; `smartTemplateExpansion` flag trong `ProcessResult` | ✅ DONE 2026-05-18 |
| M15.6 | UI (Dict Manager tab) | `Gtk.Notebook` 2 tab: “📖 Abbreviations” + “⚡ Smart Templates”. Table + Add/Edit/Delete + quick test. `TemplateDialog` với live Jinja2 preview inline (không subprocess). | ✅ DONE 2026-05-18 |

**Scope KHÔNG có trong M15:**
- Arbitrary code execution / Python `eval` trong template (security boundary: chỉ Jinja2 sandbox)
- Conditional logic `{if/else}` trong pattern (chỉ trong expansion qua Jinja2)
- Template marketplace / cloud sync

---

### Milestone M16 — Screenshot Tool ✅ CLOSED 2026-05-19

**Mô tả:** Chụp màn hình tích hợp vào FarolKey, tương tự Win+Shift+S — tự vào clipboard, hỗ trợ GNOME/wlroots Wayland và X11.

**Flow:** Hotkey → capture fullscreen ngay → CaptureReviewOverlay (ảnh chụp làm background + toolbar) → [Chọn vùng crop | Dùng ảnh này | Huỷ | Cài đặt] → clipboard + save dialog.

| Sub | Tên | Nội dung | Trạng thái |
|---|---|---|---|
| M16.1 | Core capture | GNOME: `gnome-screenshot`; wlroots: `grim`; X11: `maim`. `crop_pixbuf()` via `save_to_bufferv` | ✅ |
| M16.2 | CaptureReviewOverlay | Multi-monitor: 1 fullscreen overlay per monitor (`fullscreen_on_monitor`), shared `_SelState` absolute coords, cross-monitor drag, confirm on release | ✅ |
| M16.3 | Clipboard integration | `wl-copy --type image/png` / `xclip`; ping clipboard daemon socket | ✅ |
| M16.4 | Save dialog | "Lưu không?" + checkbox "Không hỏi lại" + auto-save toggle | ✅ |
| M16.5 | Notification | `notify-send "Screenshot copied"` | ✅ |
| M16.6 | Hotkey daemon | GNOME: `gsettings` custom keybinding (no daemon needed); non-GNOME: `pynput` listener | ✅ |
| M16.7 | Settings window | Phím tắt (tự áp dụng), nơi lưu, toggles, tool status. "super = phím Windows" hint | ✅ |
| M16.8 | Systray entry | `screenshotAction_` trong fcitx5 engine; label realtime từ screenshot.conf; double-fork + 800ms delay | ✅ |
| M16.9 | Packaging | CMakeLists install targets; install.sh deps; autostart desktop; postinst gsettings setup | ✅ |

---

### Milestone M17 — Smart Tone Normalization (Đặt dấu thông minh) ✅ DONE (M17.1+M17.2+M17.3+M17.4 ✓)

**Bối cảnh / vấn đề:**
Khi user gõ tone key *trước* khi nucleus đầy đủ (ví dụ: `h→o→j→a→w→c` để gõ `hoặc`), tone mark được đặt vào 'o' tại thời điểm gõ `j`, nhưng sau khi nucleus mở rộng thành 'oa' và 'a' biến thành 'ă', tone vẫn nằm ở 'o' → commit ra `hộăc` (sai). Vietnamese orthography yêu cầu tone nằm trên **âm chính** của âm tiết — chỉ xác định được khi nucleus + coda đã đầy đủ.

**Phạm vi:** Cả Telex lẫn VNI. Scope lớn — chia tối đa thành sub-task nhỏ nhất có thể.

**Hai loại lỗi cần fix:**
1. **Sai vị trí tone** — tone nằm trên glide/âm đệm thay vì âm chính (vd. `hộăc` → `hoặc`)
2. **Diacritic sai trên glide** — biến âm 'o'→'ơ' khi 'o' đóng vai glide (vd. `hơặc` → `hoặc`)

---

**M17.1 — `correctToneBearingIndex` (pure logic, standalone)**

Hàm trả về **index** trong `[span.medial_end, span.nucleus_end)` — vị trí ĐÚNG để đặt tone dựa trên cấu trúc âm tiết đã parse. Không side effect, không thay đổi buffer.

| Nucleus pattern (base) | hasCoda | Tone nên ở | Ví dụ |
|---|---|---|---|
| 1 ký tự | bất kỳ | ký tự đó | `an`, `ăn` |
| `ia` / `ya` | false (mở) | `i`/`y` | `mía`, `thuỷ` |
| `ua` | false | `u` | `lúa`, `của` |
| `ưa` | false | `ư` | `mứa`, `hứa` |
| oaGlide (`o`+`a`/`ă`/`e`) | bất kỳ | char sau `o` | `hoặc`, `xoáy`, `khỏe` |
| uGlide (`u`+`ê`/`e`/`ô`+coda/`a`+coda) | bất kỳ | char sau `u` | `tuệ`, `khuất` |
| `iê` / `yê` | true | `ê` | `tiếng`, `yêu` |
| `uô` | true | `ô` | `muốn`, `uống` |
| `ươ` | bất kỳ | `ơ` | `mướn`, `người` |
| `uôi` | - | `ô` | `muối` |
| `ươi` / `ươu` | - | `ơ` | `rượu`, `mười` |
| `iêu` / `yêu` | - | `ê` | `tiếu`, `yếu` |
| `uyê` | - | `ê` | `quyền` |
| Còn lại | - | char cuối nucleus | fallback an toàn |

**Deliverable:** function trong `vi_syllable.cpp/.h` + unit test table đầy đủ trong `vi_syllable_smoke_test.cpp`.

**Exit criteria:** coverage toàn bộ pattern trong bảng trên; không regression.

---

**M17.2 — `normalizeSyllableTonePlacement` (buffer mutation)**

Gọi `correctToneBearingIndex`, rồi:
1. Tìm char nào trong nucleus đang mang tone → strip tone về base
2. Nếu char đang đóng vai glide (vd. `o` trong `oaGlide`) nhưng có diacritic sai (vd. `ơ`) → reset về base (`o`)
3. Apply tone vào đúng vị trí

```
hộăc  →  tone ở 'ộ' (sai), đúng là 'ă'  → hoặc ✓
hơặc  →  'ơ' glide sai diacritic → 'o'; tone ở 'ặ' (đã đúng) → hoặc ✓
hoặc  →  không thay đổi ✓
tiếng →  không thay đổi ✓
lúa   →  không thay đổi ✓
```

**Deliverable:** function trong `vi_syllable.cpp/.h` + test table dạng `(input, expected_output)`.

**Exit criteria:** test pass cho cả 2 loại lỗi; không regression corpus hiện tại.

---

**M17.3 — Wire vào commit path (Telex + VNI)**

Trong `commitWithSuffix` ([engine.cpp:469](src/core/engine.cpp#L469)), sau `normalizeTelexBuffer` (Telex):

```cpp
// Apply cho cả Telex lẫn VNI — hàm không phụ thuộc input method
if (auto spans = vi_syllable::segmentWholeBuffer(decoded)) {
    for (const auto& span : *spans) {
        vi_syllable::normalizeSyllableTonePlacement(decoded, span);
    }
}
```

**Regression test bắt buộc:**
- Telex: `hojawc` → `hoặc`
- VNI: `ho5a8c` → `hoặc`
- Không thay đổi: `tiếng`, `muốn`, `lúa`, `người`, `được`, `tương`, `thuở`, `quyền`
- Thêm ≥ 10 corpus entries mới vào `telex.jsonl` + `vni.jsonl`

**Exit criteria:** 11/11+ tests xanh; corpus count tăng; không có regression nào từ corpus cũ.

---

**M17.4 — Real-time normalization trong lúc compose** ✅ DONE (2026-05-26)

Lambda `applyPreeditNormalize` trong `processVietnameseKey` — gọi **chỉ trong push path** (Path B) sau `preeditBuffer_ = encodeUtf8(decoded)`, trước `maybeAutoCommitStablePrefix`.

**Thiết kế cuối cùng (push-path-only):**
- Transform path (Path A) không cần gọi: với oaGlide, coda luôn được thêm qua push path; open syllable `selectToneOffset(“oa”,false)=0` → ngang trên 'o' là đúng → redundant.
- Idempotent với M17.3: nếu preedit đã normalize thì commit-time cũng trả về false.
- Corpus: 11/11 pass (100%). Root cause của 2 failures trước: stale binary `cbakey_corpus_test` (May 18) chưa rebuild sau khi đổi cmake option name `CBAKEY_BUILD_TESTS`→`FAROLKEY_BUILD_TESTS`. Fix: `cmake -DFAROLKEY_BUILD_TESTS=ON` + rebuild → new `farolkey_corpus_test` → ALL PASS.

**Files:** `src/core/engine.cpp`

---

**M17.5 — Edge case & full corpus pass**

Thêm test cho các pattern ít gặp: `quyền`, `ngoài`, `toán`, `thuở`, `cưới`, `uống`. Chạy full corpus audit.

**Exit criteria:** 0 regression; corpus count tăng thêm ≥ 5 entries covering new patterns.

---

**Thứ tự ưu tiên M17:**

| Sub-task | Phụ thuộc | Độ khó | Rủi ro |
|---|---|---|---|
| M17.1 correctToneBearingIndex | — | Trung bình | Thấp |
| M17.2 normalizeSyllableTonePlacement | M17.1 | Trung bình | Thấp |
| M17.3 Wire commit path | M17.2 | Thấp | **Trung bình** (regression) |
| M17.5 Edge cases | M17.3 | Thấp | Thấp |
| M17.4 Real-time | M17.3 stable | Cao | **Cao** |

**Release target:** M17 hoàn chỉnh (M17.1+M17.2+M17.3+M17.4) = v0.1.3. M17.5 có thể làm trong v0.1.3 hoặc sau.

| ID | Hạng mục | Deliverable | Exit criteria |
|----|-----------|-------------|----------------|
| M17.1 | `correctToneBearingIndex` | Pure function + unit test table | Coverage toàn bộ nucleus pattern; no regression |
| M17.2 | `normalizeSyllableTonePlacement` | Buffer mutation: di chuyển tone + strip glide diacritic | Test `hộăc→hoặc`, `hơặc→hoặc` và ≥ 10 cases khác |
| M17.3 | Wire commit path | Hook vào `commitWithSuffix`; corpus entries mới | Telex `hojawc→hoặc`; VNI `ho5a8c→hoặc`; 0 regression |
| M17.4 | Real-time (preedit) | Normalize trong `pushChar` path khi vowel/coda push | Tone tự di chuyển live; không unexpected jump |
| M17.5 | Edge case coverage | Full corpus pass; ≥ 5 pattern mới | 0 regression; audit xanh |

---

### Milestone M14 — Input Method mở rộng ✅ M14.1–M14.7, M14.9–M14.12 DONE | M14.8 PLANNED

**Bối cảnh:** FarolKey hiện hỗ trợ Telex + VNI + VIQR + VIQR* + Simple Telex + Simple Telex 2 + Microsoft Vietnamese + Tốc ký. Còn M14.8 (Tự nhiên — complex) và M14.12 (Free Layout — planned).

**Quyết định thiết kế (chốt 2026-06-01):**

| Quyết định | Lựa chọn | Lý do |
|-----------|---------|-------|
| Passthrough mode | **Bỏ** | Chưa có use case rõ ràng |
| VIQR escape policy | **Double-key**: `..`→`.` `''`→`'` `` `` ``→`` ` `` `??`→`?` `~~`→`~` `^^`→`^` `((`→`(` `++`→`+` `ddd`→`dd` | Nhất quán với repeat-key Telex |

---

**VIQR transform table:**

```
Diacritics: a^→â  e^→ê  o^→ô  a(→ă  o+→ơ  u+→ư  dd→đ
Tones:      ' →sắc  ` →huyền  ? →hỏi  ~ →ngã  . →nặng
Escape:     ''→'  ``→`  ??→?  ~~→~  ..→.  ^^→^  ((→(  ++→+  ddd→dd
```

---

**Kiến trúc:**

```
Settings General tab:
  [Input method  ▼ Telex  ]
   Telex / VNI / VIQR / VIQR* / Simple Telex / Simple Telex 2 / Tự nhiên

Core (input): InputMethod enum → dispatch to transform function
Config:       inputMethod=telex|vni|viqr|viqr_star|simple_telex|simple_telex2|natural
```

---

**Thứ tự và phụ thuộc:**

```
M14.1 → M14.2 → M14.3 → M14.4 → M14.5  ──── DONE ────
M14.3 → M14.6 (VIQR*)               ✅ DONE
M14.5 → M14.7 (Simple Telex)        ✅ DONE
M14.5 → M14.9 (Simple Telex 2)      ✅ DONE
M14.5 → M14.8 (Tự nhiên — PLANNED)
M14.5 → M14.10 (Microsoft Vietnamese)   ✅ DONE
M14.5 → M14.11 (Tốc ký)                 ✅ DONE
M14.5 → M14.12 (Free Layout)            ✅ DONE
```

---

| ID | Hạng mục | Deliverable | Phụ thuộc | Exit criteria |
|----|----------|-------------|-----------|---------------|
| M14.1 | `InputMethod` enum + dispatch | Thêm `Viqr` vào enum; nhánh dispatch trong `processVietnameseKey` gọi stub | — | ✅ Telex/VNI corpus 0 regression |
| M14.2 | VIQR spec document | `docs/viqr_spec.md`: bảng transform, escape, edge cases đầy đủ | M14.1 | ✅ Spec review nội bộ trước khi code M14.3 |
| M14.3 | VIQR transform core | `vi_syllable.cpp` thêm `applyViqrTransform()`; `engine.cpp` thêm `applyToneViqr()`, `isViqrRepeatableKey()`, `viqrBypass` guard; ≥50 unit tests | M14.2 | ✅ 1/1 viqr_smoke_test pass; 0 regression |
| M14.4 | Corpus VIQR + wire corpus driver | `corpus/final/viqr.jsonl` 102 cases; `corpus_driver.cpp` thêm "viqr" config | M14.3 | ✅ 102/102 corpus pass; full suite 11/12 |
| M14.5 | Settings UI — VIQR | `farolkey-settings`: dropdown Input method thêm VIQR (3 options); `set_active` dùng `.index()` thay hardcode | M14.4 | ✅ Syntax OK; dropdown hiển thị Telex/VNI/VIQR |
| M14.6 | VIQR* (VIQR Star) | Biến thể VIQR: dùng `*` thay `+` cho horn (ơ, ư); `+` là literal; `**`→`*` escape; enum `ViqrStar`; systray + Settings dropdown; `corpus/final/viqr_star.jsonl` 35 cases | M14.3 | ✅ 35/35 corpus pass; 9 smoke tests pass; 0 Telex/VNI/VIQR regression |
| M14.7 | Simple Telex | Telex chỉ xử lý tone (`s/f/r/x/j`), không transform (`w/a/e/o/d/z` → literal); enum `SimpleTelex`; systray + Settings dropdown; `corpus/final/simple_telex.jsonl` 33 cases | M14.5 | ✅ 33/33 corpus pass; 5 smoke tests pass; 0 regression |
| M14.8 | Tự nhiên (Natural input) | Gõ không dấu/tone, engine tra từ điển chọn từ phù hợp nhất qua word-level disambiguation; `NaturalDispatch` + lookup; fallback commit-as-typed khi không tìm được | M14.5, M8 (dict) | ≥70% top-1 accuracy trên 100 common words; không crash khi dict thiếu |
| M14.9 | Simple Telex 2 (stelex2) | Full Telex + `w` standalone → ư khi không có vowel trong buffer (vne_telex_w fallback từ ibus-unikey); enum `SimpleTelex2`; systray + Settings dropdown; `corpus/final/simple_telex2.jsonl` 32 cases | M14.5 | ✅ 32/32 corpus pass; 5 smoke tests pass; 0 regression |
| M14.10 | Microsoft Vietnamese | Số trực tiếp: `1`→ă `2`→â `3`→ê `4`→ô `7`→ư `8`→ơ `0`→đ; tone: `s`→sắc `j`→nặng `5`→huyền `6`→hỏi `9`→ngã; `s` disambiguation (consonant vs tone theo context); enum `Microsoft`; systray + Settings dropdown; `corpus/final/microsoft.jsonl` 37 cases | M14.5 | ✅ 37/37 corpus pass; 6 smoke tests pass; 0 regression |
| M14.11 | Tốc ký (VNI) | Base VNI + phụ âm rút gọn. Đầu từ: f→ph, j→gi, z→d, d→đ, w→ng, q→qu; k/c literal. Cuối từ: g→ng, h→nh, k→ch. Enum `TocKy`; display "Tốc ký (VNI)". `corpus/final/toc_ky.jsonl` 37 cases | M14.5 | ✅ 37/37 corpus; 7 smoke tests; 0 regression |
| M14.12 | Free Layout | Input method 100% do user tự định nghĩa. **2 lớp cấu hình:** (1) Shortcut table: key→chuỗi Unicode bất kỳ; (2) Tone/Diacritic map: 10 actions cố định (sắc/huyền/hỏi/ngã/nặng/mũ/breve/móc/d-gạch/xóa-dấu), mỗi action user gán 1 phím tùy ý. Context-based: có nguyên âm→áp tone/diacritic; không có→shortcut. Config: `~/.config/farolkey/free_layout.json`. UI: Tab "Free Layout" trong Settings + dialog "Custom Tone Keys". Enum `FreeLayout`; display "Free Layout". `corpus/final/free_layout.jsonl` | M14.5 | ≥20 corpus pass; smoke tests pass; UI Add/Edit/Delete/Save; tone dialog gán key; hot-reload; 0 regression |

---

### Milestone M18 — Auto-capitalize ✅ DONE

**Bối cảnh:** Chữ cái đầu câu / đầu dòng tự viết hoa — tương tự bàn phím smartphone. Toggle trong config, **mặc định OFF** để tránh false positive với user không kỳ vọng tính năng này.

**Thiết kế thực tế (adapter-level, không cần engine flag):**
- Uppercase `ev.key` TRƯỚC khi pass vào `br.handleKey()` — core engine xử lý bình thường
- Trigger check qua `surroundingText` — không cần flag riêng, không cần `toUpperFirstChar` riêng vì engine đã có `applyTelexTransform` + `applyToneAt` xử lý uppercase (`'D'+'d'→'Đ'`, v.v.)
- Guard: preedit phải empty (đầu từ mới), key phải `a–z` (lowercase), không giữ Shift, không password field

| ID | Hạng mục | Deliverable | Status |
|----|----------|-------------|--------|
| M18.1 | Config toggle | `autoCapitalize: bool` trong `FarolKeyConfig`; default `false` | ✅ DONE |
| M18.2 | Uppercase helper (adapter) | `shouldAutoCapitalize(ic)` đọc surrounding text — field-start, newline, sau `.`/`?`/`!` | ✅ DONE |
| M18.3 | Apply trong `keyEvent()` | Uppercase `ev.key` khi trigger + guard password field, shift, ctrl | ✅ DONE |

**Changelog M18 (2026-05-28):**
- `include/farolkey/adapter/fcitx5/farolkey_fcitx5_config.h` — thêm `autoCapitalize` Option (default false)
- `src/adapter/fcitx5/farolkey_fcitx5_engine.cpp`:
  - `shouldAutoCapitalizeOnActivate(ic)` — dùng surroundingText, chỉ gọi ở `activate()` (fresh)
  - `commitTriggersCapitalize(text)` + `isWhitespaceOnlyCommit(text)` — helpers tracking
  - `capitalizeNext_` per-IC map — internal state thay thế surroundingText async
  - `activate()`: set flag từ surroundingText (fresh tại focus-in)
  - `keyEvent()`: check + consume flag trước `handleKey()`; reset flag khi non-alpha non-space at word start
  - Sau `ic->commitString()`: update flag dựa trên nội dung commit
  - `flushAndCleanup()`: erase flag khi IC destroyed
- Build clean ✅

**Changelog M18 (2026-06-01) — Terminal detection hardening:**
- `isTerminalContext(ic)`: case-insensitive matching (`std::tolower`); thêm Wayland app-ID fragments (`gnome.terminal`, `kde.konsole`, `elementary.terminal`, `mitchellh.ghostty`); thêm ghostty, lxterminal, mate-terminal, qterminal; thêm `#include <cctype>`
- Thêm `isElectronEditorApp(ic)` helper mới: exact match `"code"`, `"code-insiders"`; substring `"vscode"`, `"code-oss"`, `"codium"` — dùng cùng pattern lowercase
- `shouldAutoCapitalizeOnActivate`: return false cho Electron apps (documented limitation)
- Enter block + `isPwd` check: thêm `isElectronEditorApp(ic)` guard
- **Known limitation (accepted 2026-06-01):** VSCode editor cũng mất auto-capitalize — Electron/Chromium báo cùng capability flags cho editor IC và terminal IC (`preedit=1, surround=0, terminal=0`); không có discriminator đáng tin cậy ở fcitx5 level

---

### Milestone M19 — Config Live-reload 🔄 IN PROGRESS

**Bối cảnh:** Sau khi cài .deb mới hoặc sửa config file trực tiếp, user phải chạy tay `fcitx5-remote -r` hoặc logout/login. M19 tự động hóa quá trình reload.

**3 gap cần giải quyết:**
1. `install.sh` chưa tự restart fcitx5 sau khi cài xong
2. Developer sửa `~/.config/fcitx5/conf/farolkey.conf` trực tiếp → không có hiệu lực ngay
3. Không có lệnh convenience để reload nhanh

| ID | Hạng mục | Deliverable | Exit criteria | Status |
|----|----------|-------------|---------------|--------|
| M19.1 | `install.sh` auto-restart | Sau `dpkg -i` thành công → tự gọi `fcitx5-remote -r` (nếu fcitx5 đang chạy) | Cài .deb xong → config mới có hiệu lực ngay; không crash nếu fcitx5 chưa chạy | ✅ DONE |
| M19.2 | `farolkey-reload` helper | Script `farolkey-reload` trong PATH (wrapper `fcitx5-remote -r`); tài liệu trong user guide | Script chạy được từ terminal; mục trong `docs/user_guide.md` | ✅ DONE |
| M19.3 | Daemon auto-restart after install | `install.sh` + `postinst` kill old clipboard/screenshot daemons và relaunch ngay; detect first install vs update để hiển thị "Next steps" đúng | Update → không cần logout; first install → vẫn cần logout 1 lần cho env vars | ✅ DONE |
| M19.4 | inotify config watch *(optional, phức tạp)* | Engine watch config file mtime; reload khi thay đổi mà không crash | Config edit trực tiếp → take effect < 2s; rollback / giữ state nếu file lỗi | ⏳ PLANNED |

**Changelog M19 (2026-05-28):**
- M19.1: `deploy/deb/postinst` — đọc DBUS_SESSION_BUS_ADDRESS từ `/proc/$FCITX5_PID/environ`, gọi `fcitx5-remote -r` as real user
- M19.1: `install.sh` — sau Step 3 install, nếu `pgrep fcitx5` thì tự `fcitx5-remote -r`; "Next steps" ẩn logout instruction nếu đã reload thành công
- M19.2: `src/common/farolkey-reload` — bash script mới, installed vào `/usr/bin/`
- M19.2: `CMakeLists.txt` — thêm `install(PROGRAMS farolkey-reload ...)`
- M19.2: `docs/user_guide.md` — section 4 + FAQ cập nhật dùng `farolkey-reload`
- M19.3: `install.sh` — kill `farolkey-clipboard` + `farolkey-screenshot-daemon` cũ, relaunch với `nohup ... & disown`; detect `FIRST_INSTALL` từ `~/.profile` để hiển thị "Next steps" khác nhau
- M19.3: `deploy/deb/postinst` — helper `_session_env()` đọc env từ `/proc/PID/environ`; kill + relaunch 2 daemons as real user với đầy đủ `DBUS/DISPLAY/WAYLAND_DISPLAY/XDG_RUNTIME_DIR`

---

### Milestone M20 — Dedicated Settings GUI ✅ DONE (2026-05-28 – 2026-06-01)

**Bối cảnh:** fcitx5-configtool là UI generic, không thân thiện với user thường. M20 tạo settings window riêng cho FarolKey — tab rõ ràng, tích hợp các tool đã có (dict manager M13, template manager M15.6, log export M16).

**Architecture:** GTK3, standalone binary `farolkey-settings`; launch từ systray action và từ CLI; single-instance lock (tương tự `farolkey-clipboard`).

**Tabs:**
1. **General** — Input method, toggle key, preedit mode, underline, auto-capitalize, committed rewrite
2. **Dictionary & Templates** — merge dict manager + template manager từ M15.6
3. **Clipboard History** — popup position, close behavior, system tools
4. **Screenshot** — hotkey, capture mode, instant capture, save path, system tools
5. **About** — version, service status, restart daemons, export log, GitHub Issues

| ID | Hạng mục | Status |
|----|----------|--------|
| M20.1 | Window shell + launcher | ✅ Single-instance lock (`fcntl`), systray "Settings (vX.Y.Z)" action |
| M20.2 | General tab | ✅ Tất cả option `FarolKeyConfig`; save → `farolkey-reload` |
| M20.3 | Dictionary & Templates tab | ✅ Merge dict + template manager |
| M20.4 | About & Diagnostics tab | ✅ Version, service status, restart, export log, GitHub Issues |
| M20.5 | M19 live-reload | ✅ Save → `farolkey-reload` |

**Bug fixes (2026-05-28):**
- `AttributeError: set_im_module` — method không tồn tại; xóa 9 occurrences + `_ORIG_IM_MODULE` + focus callbacks
- `Gtk.show_uri_on_screen` → `subprocess.Popen(['xdg-open', url])`
- Switch widgets stretch full-width → `row()` helper thêm `expand=False` default
- Version "unknown" → hằng số `_SCRIPT_VERSION = '0.1.4'` fallback
- Icon piano → `input-keyboard` system icon

**Improvements (2026-06-01):**
- `_on_general_reset()` hiển thị diff table trước khi reset (tên setting / giá trị hiện tại đỏ → mặc định xanh)
- `Configurable=False` trong cả hai `.conf` để tắt generic fcitx5-configtool
- Toàn bộ UI chuyển sang **English** (fix inconsistency mixed VI/EN)

---

### Milestone M21 — Đa ngôn ngữ UI (Localization / i18n) ✅ DONE (2026-06-01)

**Bối cảnh:** Sau khi M20 chuẩn hóa UI sang English, M21 thêm tính năng chọn ngôn ngữ (English / Tiếng Việt) cho toàn bộ UI. Mặc định English.

**Vấn đề giải quyết:** UI hiện tại English-only sau M20; user Việt muốn UI tiếng Việt có thể toggle trong Settings.

**Gate triển khai:** Bắt đầu khi có user feedback cần VI UI; không rush vì audience chủ yếu tech-savvy đọc được English.

---

**Quyết định thiết kế (chốt 2026-06-01):**

| Quyết định | Lựa chọn | Lý do |
|-----------|---------|-------|
| i18n mechanism | **gettext + .po/.mo** | Chuẩn Linux, có tooling tốt (poedit), compile ra .mo binary |
| Apply language | **Restart app** | Đơn giản, không cần rebuild GTK widgets; M21.6 bỏ qua |
| C++ scope | **Systray menu labels** | Chỉ dịch systray labels; notification giữ English |

---

**Architecture:**

```
locales/
  en/LC_MESSAGES/farolkey.po    ← source strings (English, template)
  en/LC_MESSAGES/farolkey.mo    ← compiled (generated by msgfmt)
  vi/LC_MESSAGES/farolkey.po    ← Vietnamese translation
  vi/LC_MESSAGES/farolkey.mo    ← compiled
```

**Python scripts cần i18n** (chỉ GUI, không cần cho CLI tools):
- `src/gui/farolkey-settings` — M20 main GUI (tab General, Dictionary, Clipboard, Screenshot, About)
- `src/gui/farolkey-clipboard` — popup UI labels (nút Pin/Delete, search placeholder, footer)
- `src/gui/farolkey-screenshot-settings` — settings window

**Python scripts KHÔNG cần i18n** (CLI, không có UI):
- `farolkey-abbrev`, `farolkey-template`, `farolkey-reload`

**Shared i18n loader** (`src/gui/_farolkey_i18n.py`):
```python
import gettext, os
from pathlib import Path

def setup_i18n(lang: str = 'en') -> callable:
    locales_dir = Path(os.environ.get('FAROLKEY_LOCALES',
        Path(__file__).parent.parent / 'share/farolkey/locales'))
    try:
        t = gettext.translation('farolkey', localedir=locales_dir, languages=[lang])
        return t.gettext
    except FileNotFoundError:
        return str  # fallback: passthrough

def read_ui_language() -> str:
    conf = Path.home() / '.config/farolkey/farolkey.conf'
    # parse [UI] section, default 'en'
    ...
```

Mỗi script gọi `_ = setup_i18n(read_ui_language())` ở đầu, sau đó wrap strings bằng `_('...')`.

**Config format** (`~/.config/farolkey/farolkey.conf`):
```ini
[UI]
language=en
```

**CMake build step:**
```cmake
find_program(MSGFMT_EXECUTABLE msgfmt REQUIRED)
foreach(LANG en vi)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/locales/${LANG}/LC_MESSAGES/farolkey.mo
        COMMAND ${MSGFMT_EXECUTABLE} -o <output> locales/${LANG}/LC_MESSAGES/farolkey.po
        DEPENDS locales/${LANG}/LC_MESSAGES/farolkey.po
    )
endforeach()
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/locales
    DESTINATION share/farolkey)
```

**C++ systray — `UIStrings` struct:**
```cpp
struct UIStrings {
    const char* modeVietnamese;   // "Vietnamese" / "Tiếng Việt"
    const char* modeEnglish;      // "English" / "Tiếng Anh"
    const char* settings;         // "Settings (v...)" / "Cài đặt (v...)"
    const char* clipboardHistory; // "Clipboard History" / "Lịch sử Clipboard"
    const char* screenshot;       // "Screenshot" / "Chụp màn hình"
    const char* about;            // "About" / "Giới thiệu"
};
static const UIStrings kStringsEn = {"Vietnamese", "English", ...};
static const UIStrings kStringsVi = {"Tiếng Việt", "Tiếng Anh", ...};
```
Load ngôn ngữ từ `~/.config/farolkey/farolkey.conf` khi engine init; có hiệu lực sau `fcitx5 -r`.

---

**Thứ tự triển khai:**

```
M21.1 (inventory) → M21.2 (infra + xgettext) → M21.3 (vi.po translation)
                                               → M21.4 (language selector UI)
M21.5 (C++ systray) — độc lập, có thể làm song song
```

| ID | Hạng mục | Deliverable | Phụ thuộc | Độ phức tạp |
|----|----------|-------------|-----------|-------------|
| M21.1 | String inventory | `docs/i18n_strings.md`: liệt kê tất cả UI strings (Python + C++ systray) | — | Thấp | ✅ |
| M21.2 | Python i18n infrastructure | `farolkey_i18n.py`; `en/vi .po` template; CMake `compile_po.py` build step; tất cả GUI scripts dùng `_()` | M21.1 | Trung bình | ✅ |
| M21.3 | Vietnamese translation | Hoàn chỉnh `vi/LC_MESSAGES/farolkey.po` — 117 strings | M21.2 | Thấp (effort dịch) | ✅ |
| M21.4 | Language selector | Settings General tab: dropdown "Language"; save `[UI]\nlanguage=`; info dialog "Restart required"; đọc language khi khởi động | M21.2 | Trung bình | ✅ |
| M21.5 | C++ systray i18n | `UIStrings` struct EN/VI; đọc `language` từ `farolkey.conf` khi engine init; systray labels dùng struct | — | Thấp–Trung bình | ✅ |
| M21.6 | ~~Live reload~~ | **Bỏ qua** — restart-based đã được chọn | — | — | — |

---

**Exit criteria M21:**
- `farolkey-settings`, `farolkey-clipboard`, `farolkey-screenshot-settings` hiển thị đúng theo language đã chọn sau restart
- Systray menu labels đổi theo language sau `fcitx5 -r`
- Fallback về English khi .mo file thiếu hoặc config không có
- Không regression trên các tính năng hiện có

---

### Milestone M22 — Output Charset (TCVN3 / CP1258 / VISCII) ⏳ PLANNED

**Bối cảnh:** Tách từ M14 (2026-06-03). Output charset layer cho phép FarolKey commit text ở encoding legacy (TCVN3, CP1258, VISCII) thay vì UTF-8, phục vụ các app XIM/legacy không xử lý được Unicode.

**Quyết định thiết kế:**

| Quyết định | Lựa chọn | Lý do |
|-----------|---------|-------|
| Output charset mechanism | **Raw bytes** qua `commitString` | Apps legacy (XIM) nhận bytes trực tiếp |
| Charset hỗ trợ | TCVN3 (ABC), CP1258 (Windows-1258), VISCII | Cả 3 phổ biến nhất ở Việt Nam |
| UI Settings | **Dropdown độc lập** “Output charset” — không ràng buộc với input method | Input và output là orthogonal |
| Default | Unicode NFC (no-op) | Không ảnh hưởng user hiện tại |

**⚠ Known risk:** GTK/Qt hiện đại assume `commitString` là UTF-8. Raw bytes TCVN3/CP1258/VISCII chỉ hoạt động đúng với XIM protocol (legacy). Ghi rõ trong UI warning và docs.

---

**Kiến trúc:**

```
Settings General tab:
  [Output charset  ▼ Unicode NFC  ]
   Unicode NFC / TCVN3 (ABC) / CP1258 (Windows-1258) / VISCII
  ⚠ TCVN3/CP1258/VISCII chỉ dùng cho app XIM/legacy

Adapter (output): trước ic->commitString() → charset::convert(text, outputCharset_)
Config:           outputCharset=unicode|tcvn3|cp1258|viscii
```

---

**Thứ tự và phụ thuộc:**

```
M22.1 → M22.2 (TCVN3)  ──┐
M22.1 → M22.3 (CP1258) ──┼──── M22.5 (UI)
M22.1 → M22.4 (VISCII) ──┘
```

---

| ID | Hạng mục | Deliverable | Phụ thuộc | Exit criteria |
|----|----------|-------------|-----------|---------------|
| M22.1 | Output charset infrastructure | Enum `OutputCharset`; config `outputCharset`; hook trước `commitString`; no-op khi Unicode | — | No-op verified; 0 regression |
| M22.2 | TCVN3 (ABC) table | `src/core/charset/tcvn3_table.cpp` — 134 entries NFC→byte | M22.1 | Round-trip ≥100 ký tự |
| M22.3 | CP1258 (Windows-1258) table | `src/core/charset/cp1258_table.cpp` | M22.1 | Round-trip ≥100 ký tự |
| M22.4 | VISCII table | `src/core/charset/viscii_table.cpp` | M22.1 | Round-trip ≥100 ký tự |
| M22.5 | Settings UI | Dropdown “Output charset” trong `farolkey-settings`; warning label non-Unicode; save/reload config key `outputCharset` | M22.2, M22.3, M22.4 | Save/reload đúng; warning hiển thị khi chọn non-Unicode; không conflict với input method dropdown |

---

## 7. Ánh xạ công việc đã làm (prototype) — không coi là “IME hoàn chỉnh”

Các hạng mục dưới đây là **nền móng đã có trong repo**, nhưng **chưa đạt** exit criteria của **M1–M7**:

- Skeleton CMake, `farolkey_core`, smoke tests.
- Plugin Fcitx5 load được; script cài local.
- Luật Telex/VNI **một phần**; đặt dấu **heuristic** (chưa syllable model đầy đủ).
- Một số hành vi preedit/commit/separator/cursor trong core + plugin.

**Việc cần làm tiếp:** coi các phần trên là đầu vào cho **M0–M1**, không tick “parity” cho đến khi corpus + syllable model xong.

---

## 8. Risks (cập nhật)

- **Semantic gap:** thiếu mô hình âm tiết sẽ luôn sinh bug “chỉ sửa được từng case”.
- **UI parity:** Electron/GTK khác nhau; có thể cần nhánh theo capability hoặc accept limitation có tài liệu.
- **Maintenance:** corpus lớn đòi hỏi quy trình review khi thêm rule.
- **Scope creep:** giữ wishlist (VIQR, macro động, cloud…) ngoài roadmap cho đến khi M1–M7 ổn; **M12 (clipboard)** và **M13 (gõ tắt / macro user)** là wishlist có cấu trúc — tách khỏi exit “IME core” trừ khi đã có gate rõ trong từng milestone.

---

## 9. Change Log

- 2026-05-08: Initial scope locked and planning files created.
- 2026-05-08: Added C++ project skeleton (core/config/tests), drafted Fcitx5 adapter boundary notes, and added Phase 0 technical checklist.
- 2026-05-08: Build/test execution is currently blocked on missing `cmake` in environment.
- 2026-05-08: Added config file loader/validator, logging module, Unicode strategy doc, logging policy doc, and config format doc for Phase 0 completion.
- 2026-05-08: Implemented initial Telex transformations in core engine (`aa/aw/ee/oo/ow/uw/dd`, `s/f/r/x/j`) and added Vietnamese smoke tests.
- 2026-05-08: Added initial VNI transformations in core engine (`1..5` tones, `6/7/8/9` transforms), expanded tests for Telex/VNI, and cleaned `ProcessResult` initialization warnings.
- 2026-05-08: Implemented preedit history-based backspace/undo, added undo test coverage for Telex/VNI, and added keypress latency benchmark target (`farolkey_keypress_benchmark`).
- 2026-05-08: Implemented Phase 2 bridge adapter (`farolkey_fcitx5_adapter`) with preedit/commit lifecycle wiring, EN/VN toggle path, and config-file bridge creation; added `farolkey_fcitx5_bridge_smoke_test`.
- 2026-05-08: Added real Fcitx5 plugin module target (`farolkey_fcitx5plugin` -> `farolkey.so`) with `FCITX_ADDON_FACTORY` entrypoint, key translation, preedit/commit integration, and successful build against `Fcitx5Core/Fcitx5Utils/Fcitx5Config/Fcitx5Module`.
- 2026-05-08: Added local deployment assets (`deploy/fcitx5`) and install/uninstall scripts for user-scope Fcitx5 testing; verified local install paths and plugin copy.
- 2026-05-08: Aligned plugin naming with Fcitx5 convention (`libfarolkey.so`, `Library=libfarolkey`) and reinstalled local addon assets.
- 2026-05-08: Hardened local install script to generate addon config with absolute `Library=` path and copy plugin to multiarch local lib path for loader compatibility.
- 2026-05-11: Reprioritized roadmap after live testing: inserted Input Behavior Hardening phase before user features and expanded compatibility objectives.
- 2026-05-11: Phase 3 shipped: `KeyAux` for non-character keys; commit includes trailing space/punct/newline/tab; cursor keys commit preedit + `forwardOriginalKey`; DeleteForward mirrors backspace on buffer; deactivate commits pending composition; reset commits pending composition; tests updated (`KeyAux` naming avoids system `ExtendedKey` macro clash).
- 2026-05-11: UX + VNI fixes: client preedit `NoFlag` + cursor at end of UTF-8 buffer; tone vowel selection by nucleus pairs (`oa`/`ao`/`ie`/`iê`/…); VNI `678` keeps tone index when changing vowel row (fixes digit leak e.g. `hiện`, `hiề`); tests for `chào`, `hiện`, `hiề`.
- **2026-05-11: Viết lại toàn bộ `progress.md` thành roadmap dài hạn (M0–M11), exit criteria, parity checklist; điều chỉnh kỳ vọng “prototype vs production”.**
- 2026-05-11: Chốt quy ước M0 (corpus-only): B2–B3, C1–C2; thêm checkpoint [`corpus/README.md`](corpus/README.md); cập nhật bảng M0 trong `progress.md` trỏ `corpus/` + GoogleTest.
- 2026-05-11: M0.2 triển khai: `farolkey_corpus_test` (FetchContent GTest + nlohmann/json), `tests/corpus_driver.{h,cpp}`, mẫu `corpus/*.jsonl`; stub [`docs/behavior_reference.md`](docs/behavior_reference.md).
- 2026-05-11: Mở rộng corpus: `tools/corpus_bfs_main.cpp` (`farolkey_corpus_bfs`), `scripts/expand_corpus.py` + `requirements-corpus.txt`, `corpus/sources/` (word list + README), gợi ý BFS `ascii_hint`, cache `.bfs_cache.json`, quét JSONL đệ quy trong `farolkey_corpus_test`.
- 2026-05-11: Canonicalized M0 corpus via `scripts/finalize_corpus_set.py`; added `corpus/final/` and updated `farolkey_corpus_test` to prefer canonical corpus when present. Current local counts: Telex 539, VNI 500, engine_meta 3.
- 2026-05-11: Prepared M1 draft in `docs/vi_syllable_model.md`; locked initial decisions: parse only the last syllable, model `qu/gi` in IME-practical style, represent `medial` explicitly, implement tone-placement path first, use a new syllable module, and keep permissive parser + heuristic fallback during transition.
- 2026-05-11: Started M1 implementation slice 1: added `vi_syllable` module + smoke test, routed `applyTone()` / `applyToneVni()` through the new syllable-based tone selector with legacy heuristic fallback, and added engine smoke coverage for `qu` / `gi` tone placement.
- 2026-05-11: Continued M1 with slice 2: routed Telex/VNI vowel transforms through `vi_syllable` first (legacy fallback retained), added transform smoke coverage for `ươ`, `uô`, `iê`, `uya`, and kept full test suite green.
- 2026-05-11: Hardened M1 regression coverage: converted tone-bearing selection in `vi_syllable` to explicit pattern rules and added curated M1 corpus cases/tags (`tone_placement`, `edge.qu`, `edge.gi`, `nucleus.double`, `nucleus.triple`, `telex.transform`, `vni.transform`); regenerated `corpus/final/` (Telex 545, VNI 506, engine_meta 3).
- 2026-05-11: Expanded M1 hard-case coverage for `quốc`, `thuyền`, `nghiêng`, `nguyễn`; fixed `uyê/uye` tone-bearing to target `ê/e`, added corresponding engine/module tests and curated corpus cases, regenerated `corpus/final/` (Telex 549, VNI 510, engine_meta 3).
- 2026-05-11: Added `farolkey_m1_fallback_audit` to measure where `vi_syllable` still falls back to legacy logic on corpus cases; refined tone-pattern rules (`ie/ye`, `uyê/uye`) until the current canonical corpus reports **0 tone fallbacks** and **0 transform fallbacks** in the audit while the full test suite stays green.
- 2026-05-11: Continued edge-case hunt before legacy removal: added curated cases for `giỏi`, `nước`, `hướng`, `già`, `thuở` (VNI), corrected `uơ` tone-bearing and re-ran audit. Current canonical corpus (`corpus/final/`: Telex 553, VNI 515, engine_meta 3) reports **0 tone fallbacks** and **0 transform fallbacks** on `farolkey_m1_fallback_audit`, with the full test suite still green.
- 2026-05-11: Began controlled legacy removal in `src/core/engine.cpp`: tone selection and Telex/VNI transform paths now rely solely on `vi_syllable` on the main execution path; removed the in-file heuristic fallback helpers after re-running full `ctest` and confirming `farolkey_m1_fallback_audit` still reports **0 tone fallbacks / 0 transform fallbacks** on `corpus/final/`.
- 2026-05-11: Continued post-fallback edge-case hardening: expanded `vi_syllable` tone rules with `oai` / `oay`, added smoke + curated corpus coverage for `ngoài`, `xoáy`, `thuế`, `tuổi`, `cười`, regenerated `corpus/final/` (Telex 558, VNI 520, engine_meta 3), and kept `farolkey_m1_fallback_audit` at **0 tone fallbacks / 0 transform fallbacks**.
- 2026-05-11: Hardened transformed-vowel tone placement further by adding explicit `âu` / `ây` / `êu` / `ưu` rules and curated regression for `gấu`, `cấy`, `nếu`, `cứu`; regenerated `corpus/final/` (Telex 562, VNI 524, engine_meta 3) and kept full tests + `farolkey_m1_fallback_audit` green at **0 tone fallbacks / 0 transform fallbacks**.
- 2026-05-11: Added another curated triple-vowel hardening batch for `hiếu`, `yếu`, `muối`, `tưới`; confirmed the existing `iêu` / `yêu` / `uôi` / `ươi` rules hold under smoke + canonical corpus, regenerated `corpus/final/` (Telex 566, VNI 528, engine_meta 3), and kept full tests + `farolkey_m1_fallback_audit` green at **0 tone fallbacks / 0 transform fallbacks**.
- 2026-05-11: Found and fixed a real open-syllable tone-placement gap: adjusted `ia` and `ua` handling in `vi_syllable` (e.g. `mía`, `của`), added curated regression for `mía`, `của`, `hứa`, `thuỷ`, regenerated `corpus/final/` (Telex 570, VNI 532, engine_meta 3), and kept full tests + `farolkey_m1_fallback_audit` green at **0 tone fallbacks / 0 transform fallbacks**.
- 2026-05-11: Found and fixed two more missing triple-vowel rules in `vi_syllable`: `uây` and `ươu`; added curated regression for `khuấy` and `rượu`, regenerated `corpus/final/` (Telex 572, VNI 534, engine_meta 3), and kept full tests + `farolkey_m1_fallback_audit` green at **0 tone fallbacks / 0 transform fallbacks**.
- 2026-05-11: Added a clean regression-only batch for already-supported `ôi` / `ơi` / `ui` / `ưi` patterns via `tối`, `bởi`, `túi`, `gửi`; regenerated `corpus/final/` (Telex 576, VNI 538, engine_meta 3) and kept full tests + `farolkey_m1_fallback_audit` green at **0 tone fallbacks / 0 transform fallbacks**.
- 2026-05-11: Modernized open-syllable `oa` / `oe` tone placement by distinguishing no-coda vs coda cases (`xóa`, `khỏe` vs `toàn`), added curated regression accordingly, normalized legacy generated corpus spellings (`hoà/hoá` -> `hòa/hóa`), regenerated `corpus/final/` (Telex 579, VNI 541, engine_meta 3), and kept full tests + `farolkey_m1_fallback_audit` green at **0 tone fallbacks / 0 transform fallbacks**.
- 2026-05-11: Added another clean regression-only batch for common open diphthongs `ai` / `ay` / `au` / `eo` via `gái`, `máy`, `sáu`, `khéo`; regenerated `corpus/final/` (Telex 583, VNI 545, engine_meta 3) and kept full tests + `farolkey_m1_fallback_audit` green at **0 tone fallbacks / 0 transform fallbacks**.
- 2026-05-11: Fixed the remaining Telex `uơ` gap by changing `uo + w` into a staged `uơ` transform plus literal-followup normalization to `ươ...` when the syllable closes or extends; added Telex regression for `thuở`, regenerated `corpus/final/` (Telex 584, VNI 545, engine_meta 3), and updated `farolkey_m1_fallback_audit` to report the new literal-normalization path explicitly (`literal_supported`) without false fallback samples.
- 2026-05-11: Deepened curated regression for weak open-diphthong patterns by adding `báo`, `cãi`, `hãy`, `cháu`, `béo`; regenerated `corpus/final/` (Telex 589, VNI 550, engine_meta 3), kept full tests + `farolkey_m1_fallback_audit` green (`literal_supported: 6`), and improved the internal pattern audit from **6 strong / 29 weak / 8 missing** to **11 strong / 24 weak / 8 missing**.
- 2026-05-11: Deepened weak-pattern coverage further with second curated examples for `ia`, `oe`, `uê`, `uô`, `ưa`, `ươu` (`kìa`, `xòe`, `huế`, `buồn`, `bữa`, `bướu`); regenerated `corpus/final/` (Telex 595, VNI 556, engine_meta 3), kept full tests + `farolkey_m1_fallback_audit` green (`literal_supported: 7`), and improved the internal pattern audit from **11 strong / 24 weak / 8 missing** to **17 strong / 18 weak / 8 missing**.
- 2026-05-12: Cleared the remaining user-visible weak pattern buckets by adding a final curated hardening batch for `uy`, `âu`, `ây`, `êu`, `oi`, `ôi`, `ơi`, `ui`, `ưi`, `ua`, `oai`, `oay`, `ưu`, `iêu`, `yêu`, `uya`, `uây`, `uơ` (`huỷ`, `lẩu`, `bẫy`, `đều`, `nói`, `cối`, `mời`, `bụi`, `ngửi`, `múa`, `hoài`, `ngoáy`, `hữu`, `thiếu`, `yểu`, `khuya`, `huây`, `huơ`); regenerated `corpus/final/` (Telex 613, VNI 574, engine_meta 3), kept full tests + `farolkey_m1_fallback_audit` green (`literal_supported: 7`), and improved the internal pattern audit from **17 strong / 18 weak / 8 missing** to **35 strong / 0 weak / 8 missing**. The remaining `missing` entries are intermediate/non-user-visible patterns (`eu`, `ie`, `ya`, `ye`, `yê`, `uye`, `ieu`, `yeu`), not newly observed parser gaps.
- 2026-05-12: Started M2 with a first Telex semantics slice: repeated transform/tone keys now disambiguate into literal text in the common Telex style (`herr` -> `her`, `buss` -> `bus`, `xooong` -> `xoong`, `aww` -> `aw`, `ddd` -> `dd`), while different successive tone keys still let the last one win (`asf` -> `à`). Added engine/corpus regression for these cases, regenerated `corpus/final/` (Telex 619, VNI 574, engine_meta 3), and updated the audit tool so these repeat-escape paths count as supported engine behavior (`literal_supported: 10`) instead of false fallback.
- 2026-05-12: Continued M2 with Telex `z` remove-diacritics semantics (`remove diacritics / xóa dấu`): `z` now strips both tone and transformed vowel marks (`awz` -> `a`, `aasz` -> `a`, `ddz` -> `d`, `uowz` -> `uo`, `thieeuz` -> `thieu`), while staying literal when nothing can be removed (`az` -> `az`). Added focused `vi_syllable` + engine regression, curated Telex corpus coverage, regenerated `corpus/final/` (Telex 625, VNI 574, engine_meta 3), and kept full tests + `farolkey_m1_fallback_audit` green (`literal_supported: 15`).
- 2026-05-12: Continued M2.3 by locking Telex interaction-order semantics for common transform rows in both directions (`asa`, `ese`, `oso`, `asw`, `usw`, `awsf`, `aasf`) and by fixing the missing raw-`uo` tone-bearing step so `thuosoc` -> `thuốc` and `huoswng` -> `hướng` now work with tone-before-transform ordering too. Added focused `vi_syllable` + engine regression, curated Telex corpus coverage, regenerated `corpus/final/` (Telex 634, VNI 574, engine_meta 3), and kept full tests + `farolkey_m1_fallback_audit` green (`literal_supported: 16`).
- 2026-05-12: Continued M2 with explicit Telex raw-escape via backslash for special keys: `\` now suppresses immediate Telex interpretation on the following special key (`a\w` -> `aw`, `a\s` -> `as`, `a\z` -> `az`, `\\` -> `\`). Added engine/corpus regression, taught the audit tool to treat tagged `telex.raw_escape` cases as supported literal behavior, regenerated `corpus/final/` (Telex 638, VNI 574, engine_meta 3), and kept full tests + `farolkey_m1_fallback_audit` green (`literal_supported: 20`).
- 2026-05-12: Ran a broader Telex matrix audit for `aa/ee/oo/ow/uw/dd` and locked the remaining already-supported direct-family behaviors into regression coverage: repeat-escape (`aaa`, `eee`, `ooo`, `oww`, `uww`), direct remove-diacritic (`eez`, `ooz`, `owz`, `uwz`), and raw-escape (`a\a`, `e\e`, `o\o`, `o\w`, `u\w`, `d\d`, plus fallback-to-literal `a\b`). Regenerated `corpus/final/` (Telex 654, VNI 574, engine_meta 3), kept full tests green, and updated the audit tool so tagged `telex.raw_escape` cases no longer produce false fallback samples (`literal_supported: 37`).
- 2026-05-12: Closed M2 as complete after the final Telex closeout audit, then started M3 with the first VNI literal-digit slice: generalized backslash raw-escape to both Vietnamese input methods so VNI now keeps escaped digits literal (`a\1`, `a\6`, `u\7`, `d\9`, `a\16`), added smoke + curated corpus coverage, regenerated `corpus/final/` (Telex 654, VNI 579, engine_meta 3), and updated `farolkey_m1_fallback_audit` so tagged `vni.raw_escape` cases count as supported literal behavior (`literal_supported: 42`) with no fallback samples.
- 2026-05-12: Continued M3 with a broader VNI matrix audit around `tone-vs-digit` and `6789 + tone` ordering. The audit surfaced a real order gap on common `uoi/ưo` paths (`tuoi36`, `muoi16`, `cuoi177`, `cuoi717`, `hu7o17ng`, `ru7o57u`), which was fixed by teaching `selectToneVowelIndex()` the intermediate/raw nuclei `uoi`, `uơi`, and `ưo`. Added focused `vi_syllable` + engine regression and curated VNI corpus coverage, regenerated `corpus/final/` (Telex 654, VNI 584, engine_meta 3), and kept full tests + `farolkey_m1_fallback_audit` green (`literal_supported: 42`). The same audit also narrowed the remaining open cluster to deeper raw `uou/ươu` permutations (for example `ruou757`, `ruou577`, `ruo7u75`) for a later M3 slice.
- 2026-05-12: Continued M3 with the next VNI order slice for the deeper raw `uou/ươu` family around `rượu`. Added targeted VNI transform special-casing for intermediate nuclei `uou`, `uơu`, and `ưou`, plus tone-placement support for those same intermediate states, so permutations with enough information to complete `ươu` now converge correctly (`ruou775`, `ruou757`, `ruou577`, `ruo7u75`, `ruo7u57`, `ru7ou75`, `ru7ou57` -> `rượu`). Added focused `vi_syllable` + engine regression and curated VNI corpus coverage, regenerated `corpus/final/` (Telex 654, VNI 591, engine_meta 3), and kept full tests + `farolkey_m1_fallback_audit` green (`literal_supported: 42`). Remaining partial outputs like `ruou75`, `ruo7u5`, `ru7ou5` now look consistent with “missing one more `7` transform” rather than with a fresh tone-placement bug.
- 2026-05-12: Continued breadth audit on other VNI families and fixed the next raw full-word ordering gap cluster around `ue/uu`: added raw tone-placement support for `ue` and `uu`, plus a narrow VNI `7` special-case so raw `uu` converges to `ưu` instead of `uư`. This closes user-visible order cases such as `thue16` -> `thuế`, `hue16` -> `huế`, `huu74` / `huu47` -> `hữu`, and `cuu71` / `cuu17` -> `cứu`. Added focused `vi_syllable` + engine regression and curated VNI corpus coverage, regenerated `corpus/final/` (Telex 654, VNI 596, engine_meta 3), and kept full tests + `farolkey_m1_fallback_audit` green (`literal_supported: 42`).
- 2026-05-12: Completed the current breadth-order audit pass for VNI by fixing the final systematic alternate-order failure exposed by the corpus-derived matrix: raw `uay` tone-before-transform (`khuay16` -> `khuấy`). Added raw tone-placement support for `uay`, focused `vi_syllable` + engine regression, and curated VNI corpus coverage; regenerated `corpus/final/` (Telex 654, VNI 597, engine_meta 3); reran the systematic alternate-order audit across corpus-derived VNI variants and reduced it to **0 failures**, while full tests + `farolkey_m1_fallback_audit` stayed green (`literal_supported: 42`).
- 2026-05-12: Added VNI `0` remove-diacritics semantics: when the current preedit contains Vietnamese diacritics, `0` now strips both tone and transformed vowel/consonant marks (`a10` -> `a`, `a60` -> `a`, `a160` -> `a`, `d90` -> `d`, `u750` -> `u`, `thue610` -> `thue`, `thue160` -> `thue`, `ruou7750` -> `ruou`), while staying literal when nothing can be erased (`a0` -> `a0`). Reused the diacritic-removal path in `vi_syllable`, added focused `vi_syllable` + engine regression and curated VNI corpus coverage, regenerated `corpus/final/` (Telex 654, VNI 607, engine_meta 3), and updated `farolkey_m1_fallback_audit` so tagged `vni.remove_diacritics` cases count as supported literal behavior (`literal_supported: 51`) with no fallback samples.
- 2026-05-12: Added a first VNI repeated-digit/literal-disambiguation slice for tone keys `1..5`: repeating the same tone key now emits the digit literally (`a11` -> `a1`, `a22` -> `a2`, `a33` -> `a3`, `a44` -> `a4`, `a55` -> `a5`, `he33` -> `he3`), while different tone keys still follow the standard “last one wins” rule (`a12` -> `à`). Added focused engine regression and curated VNI corpus coverage, regenerated `corpus/final/` (Telex 654, VNI 613, engine_meta 3), and updated `farolkey_m1_fallback_audit` so tagged `vni.repeat_escape` tone cases are counted as supported literal behavior. Current repeated `6..9` behavior remains as-is (`a66` -> `â6`, `o77` -> `ơ7`, `d99` -> `đ9`) pending a separate spec decision.
- 2026-05-12: Audited and then locked the current repeated `6/7/8/9` VNI behavior as the temporary spec choice instead of changing it preemptively. Given the lack of strong external evidence for a rollback-to-literal convention on transform keys, and because backslash already provides an explicit literal path, the current transformed-plus-literal outputs are now covered by regression (`a66` -> `â6`, `e66` -> `ê6`, `o66` -> `ô6`, `a88` -> `ă8`, `o77` -> `ơ7`, `u77` -> `ư7`, `d99` -> `đ9`). Added focused engine regression and curated VNI corpus coverage, regenerated `corpus/final/` (Telex 654, VNI 620, engine_meta 3), and kept full tests + `farolkey_m1_fallback_audit` green (`literal_supported: 51`).
- 2026-05-13: Thêm **Milestone M12 — Lịch sử clipboard (kiểu Windows + V)** vào roadmap: xác định đây là tính năng **addon Fcitx5 / tách khỏi core IME**, có exit criteria theo slice (kiến trúc, model dữ liệu, UI chuột + preview ảnh best-effort, Wayland/X11, đóng gói), và **gate** không mở code trước khi M6–M7 ổn trừ khi đổi ưu tiên có chủ đích.
- 2026-05-13: Thêm **Milestone M13 — Gõ tắt / macro do người dùng định nghĩa** (trigger → chuỗi dài, UI riêng, import/export, policy xung đột với Telex/VNI); liên kết **M8** (schema/nền) và **gate** triển khai sau M8.1 + M6 ổn định.
- 2026-05-13: **M6.3a prototype (C1 — commit rồi vẫn sửa được):** thêm `docs/m6_3a_implementation_plan.md`, `Engine::tryRewriteCommittedSyllable`, adapter `committed_rewrite_fcitx5.cpp` (`deleteSurroundingText` + `commitString`), nối vào `farolkey_fcitx5_engine.cpp` khi không có preedit; smoke `farolkey_m6_3a_rewrite_smoke_test`; config `fcitx5_committed_rewrite`; Telex trigger v1 chỉ `s f r x j z w`.
- 2026-05-13: **C1 / VSCode:** chuẩn hóa `SurroundingText::cursor` cho Electron (byte UTF-8 vs scalar) trong `surrounding_cursor_normalize.*`; thêm `scripts/restart_fcitx5.sh` + hướng dẫn cài đặt chờ process cũ thoát trước khi `fcitx5 -d` để tránh tải plugin `.so` không nhất quán.
- 2026-05-13: Core fix cho đặt dấu **“hoặc”** khi gõ liền: `selectToneVowelIndex()` dùng `tonePatternBucket()` để nucleus **o+ă** khớp rule `oa` (trước đó pattern `o`+U+0103 không khớp → Telex `j` / VNI `5` rơi literal). Adapter: neo click-away **hai chiều** (caret tiến/lùi) khi surrounding **byte-identical**; ghi nhận Writer/VSCode vẫn có thể làm surrounding **đổi nội dung** khi click nên neo v1 vẫn có thể thất bại (kỳ vọng slice / matrix tiếp theo).
- 2026-05-14: **C1 / tone `iu`:** đổi `selectToneOffset` pattern **`iu`** sang **`{0,0}`** — dấu VNI/Telex đặt lên **nguyên âm đầu `i`** (tránh kiểu “chiụ” khi user muốn dấu trên `i`). **C1 backlog:** ghi nhận VSCode `.txt` + Google Docs là *known limitation / hoãn* trong `docs/m6_3a_implementation_plan.md` và bảng C1 trong `docs/fcitx5_app_matrix.md`; coi C1 v1 đủ để chuyển focus sang phase M6 tiếp theo (matrix đầy đủ, sau đó M6.3b khi có audit).
- 2026-05-14: **Core: sửa lỗi tone placement "ua+coda"** (`kToneRules` `{U"ua", 0, 0}` → `{U"ua", 0, 1}`): với âm tiết có coda như "xuân/tuần/chuẩn", dấu nay đặt đúng lên nguyên âm thứ hai (`â/a`) thay vì `u`. Lỗi cũ tạo ra buffer `xủân` → rule reject "chủân" kích hoạt nhầm → `findLastSyllable` chỉ thấy "ân" → `z` chỉ strip được một phần → hỏng nghiêm trong Google Docs và app non-preedit. Thêm regression `selectToneVowelIndex(tuan)==2`, engine Telex/VNI end-to-end `chuaanr→chuẩn`, `tuaanf→tuần`, `xua6n3→xuẩn`; 9/9 test xanh.
- 2026-05-14: **M6.3b: CommitStringWithCursor trong C1 adapter** — khi client báo `CapabilityFlag::CommitStringWithCursor`, dùng `ic->commitStringWithCursor(newToken, newToken.size())` thay cho `ic->commitString(newToken)` trong luồng rewrite C1; cursor neo đúng sau chuỗi commit, giảm cursor-drift cho VSCode `.txt` + Electron. Cập nhật `docs/m6_3a_implementation_plan.md`; `ctest` xanh.
- 2026-05-14: **Đóng phase / lên kế hoạch:** cập nhật `progress.md` đánh dấu M0–M5 ✅ CLOSED, M6 🔄 CODE COMPLETE, M7 ⏸ hoãn, M8 🎯 NEXT; chi tiết kế hoạch triển khai M8 (M8.1/M8.2/M8.3) thêm vào roadmap.
- 2026-05-14: **M8.1 User Dictionary loader:** `include/farolkey/core/user_dict.h` + `src/core/user_dict.cpp` — parser JSONL không dependency ngoài, hỗ trợ `\uXXXX` escape, load XDG default path, bỏ qua file thiếu; `RuntimeConfig.userDictPath` mới.
- 2026-05-14: **M8.3:** `docs/user_dict.md` — schema, conflict policy, giới hạn, ví dụ; M8 CODE CLOSED.
- 2026-05-14: **M9.1 Benchmark (Release):** thêm 4 scenario (Telex/VNI/long-word/dict); baseline Telex 1.37 µs/key, VNI 1.01 µs/key, worst-case 2.80 µs/key (long multisyllable auto-commit); budget <10 µs/key OK; target <2 µs/key chưa đạt ở long-word path → ghi nhận cho M9.2 profiling.
- 2026-05-14: **M10.1 CPack DEB:** `cmake install()` targets cho plugin + configs; `scripts/build_deb.sh`; `deploy/deb/postinst|prerm`; `.deb` 76 KB build thành công (`farolkey_0.1.0_amd64.deb`), `dpkg-deb` verify nội dung đúng.
- 2026-05-14: **M10.3 RUNBOOK:** `deploy/RUNBOOK.md` — cài .deb, cài local, gỡ, config, troubleshoot, rollback, checklist post-install.
- 2026-05-14: **M8.2 Static expansion trong Engine:** `commitWithSuffix` kiểm tra dict trước commit trên Space/Enter/Tab; dict luôn thắng khi `enableUserDictionary=true`; smoke test `farolkey_m8_user_dict_smoke_test` 8 case; `ctest` 10/10 xanh.
- 2026-05-14: **M10.1 — sửa phụ thuộc `.deb` (Ubuntu 24.04):** `CPACK_DEBIAN_PACKAGE_DEPENDS` bổ sung `libfcitx5core7 | libfcitx5core6` (trước đó chỉ 8|9|10 → `dpkg -i` báo lỗi khi máy chỉ có `libfcitx5core7`). `deploy/RUNBOOK.md`: mục troubleshooting, gợi ý `apt install ./…deb` và `apt-get install -f` sau cài lỗi.
- 2026-05-14: **Tiến độ roadmap:** chuẩn hóa ước lượng **~72%** (M0–M11, không M12/M13; M4/M6/M9/M10 theo trọng số từng slice; M6.4/M7 QA không vào tử số); thêm đoạn giải thích trong `progress.md` §1; `2026_5_14.md` đồng bộ cùng định nghĩa.
- 2026-05-14: **Tiến độ & kế hoạch (code-only):** `progress.md` thêm **§10 Ưu tiên triển khai code tiếp theo** (M9.2, M11.1, M10.2, M4.3 tùy chọn; loại trừ M6.4/M7 QA); `2026_5_14.md` thêm mục 13 (fix Depends) và cập nhật **dự định tiếp theo** không gồm QA.
- 2026-05-14: **M11.1 User guide:** `docs/user_guide.md` — cài .deb, kích hoạt Fcitx5, gõ thử Telex/VNI, tùy chỉnh config, từ điển cá nhân, C1, FAQ, báo lỗi.
- 2026-05-14: **M9.2 Profiling + tối ưu:** Tối ưu `findStableComposeSplit` (thay `segmentWholeBufferWithPreference` bằng `vector<bool>` reachability DP + backward scan với semantics đúng); long-word **2.80 → 2.59 µs/key (−7%)**, Telex **1.37 → 1.24 µs/key (−9%)**; 10/10 tests xanh.
- 2026-05-14: **M10.2 APT repo (code done):** `scripts/setup_apt_repo.sh` (pool, Packages, Release, GPG sign) + `docs/apt_repo_guide.md` (setup, nginx, client config, troubleshooting); chờ infra để pilot.
- 2026-05-14: **M11.2:** `.github/ISSUE_TEMPLATE/bug_report.md` + `feature_request.md` với severity S1–S4. M11 CLOSED.
- 2026-05-14: **M13 CLOSED:** `AbbrevMode` enum (`Vi`/`En`/`Both`) backward-compat với M8; engine `setPasswordField`/`lookupEnglishAbbrev`; adapter Password/Sensitive detection; CLI `farolkey-abbrev` (list/add/remove/export/import, atomic write, .bak); 10 smoke tests, 11/11 xanh.
- 2026-05-14: **Bugfix — Uppercase Đ (VNI + Telex):** `applyVniTransform` key='9' và `applyTelexTransform` key='d' đã kiểm tra `U'D'` (uppercase) và convert sang `U'Đ'`; regression tests trong `vi_syllable_smoke_test` + `engine_smoke_test`; 11/11 xanh.
- 2026-05-14: **Bugfix — Click-away relaxed anchor:** `compose_anchor.h` thêm `planCaretNudgeRelaxed` — tìm context xung quanh snap cursor trong new surrounding text (window 24 codepoints mỗi bên, match unique); `planCaretNudgeForAnchoredCommit` thử exact match trước rồi mới relaxed; `compose_anchor_fcitx5.cpp` thêm `ic->updateSurroundingText()` trước khi đọc. Cải thiện cross-line click-away khi text window shift.
- 2026-05-14: **Click-away v2 — 3 strategies + rescue + drop-on-fail:** Viết lại `compose_anchor.h` với 3 chiến lược (exact → centered-48cp → left-only-64cp); thêm `planCaretNudgeLeftOnly` dùng text BÊN TRÁI cursor (ổn định hơn, phù hợp VSCode/LibreOffice với text window nhỏ); rescue: sau khi commit sai chỗ, `deleteSurroundingText` xóa text khỏi vị trí mới (best-effort X11/Wayland); config `fcitx5_click_away_drop_on_fail=true` để không commit khi anchor fail (text mất thay vì xuất hiện sai chỗ). 11/11 tests xanh.
- 2026-05-15: **Bugfix — Double Enter + Custom hotkey:** Return→`aux=KeyAux::Enter`; adapter check configured toggleKey trước engine. 11/11 xanh.
- 2026-05-15: **Bugfix — BackSpace EN + toggle hotkey:** (1) `processEnglishKey` control chars → `consumed=false` (forward). (2) Sau `handleKey`, sync `globalMode_` nếu engine toggle nội bộ. 11/11 xanh.
- 2026-05-15: **Bugfix — EN mode reset khi click:** Root cause: `deactivate()` xóa Bridge → IC mới tạo Bridge mới với mode mặc định Vietnamese. Fix: thêm `globalMode_` ở engine level; `bridgeFor()` apply `globalMode_` khi tạo Bridge mới; toggle cập nhật `globalMode_` + tất cả Bridge đang active. 11/11 xanh.
- 2026-05-15: **Dynamic VI/EN tray icon:** SVG badges `mode_vi.svg`(green) + `mode_en.svg`(gray) generate vào `~/.cache/farolkey/`; `subModeIconImpl` trả path đúng theo mode; icon đổi realtime. 11/11 xanh.
- 2026-05-15: **M14 UI delay fix + subModeLabel:** `updateUserInterface(StatusArea)` sau mọi state change; `subModeLabelImpl` trả "VI"/"EN" cho Fcitx5 panel. 11/11 xanh.
- 2026-05-15: **M14 UI fix:** (1) `methodMenuAction_` hiển thị active method: "Input Method: Telex"/"Input Method: VNI"; `setChecked()` trên Telex/VNI items. (2) Xóa `underlineAction_` khỏi statusArea (config-only). (3) Toàn bộ text → English. 11/11 xanh.
- 2026-05-15: **M14 UI/UX CLOSED:** Config schema (`FCITX_CONFIGURATION FarolKeyConfig`: method/toggleKey/enableUserDictionary/showPreeditUnderline/committedRewrite); Systray actions: "VI"/"EN" status text (click=toggle), Telex/VNI submenu, Gõ tắt toggle, Gạch chân toggle, Mẫu gõ nhanh placeholder (greyed); Engine migrated to `InputMethodEngineV2` + `getConfig`/`setConfig`; legacy config auto-imported; 11/11 xanh.
- 2026-05-15: **Bugfix — toggle 'ươ'↔'uô' v2 (hoàn chỉnh):** VNI: '7'+'ươ'→'uo'(revert), '6'+'ươ'→'uô'(swap), '7'+'uô'→'ươ'; Telex: 'w'+'ươ'→'uo', 'o'+'ươ'→'uô', 'w'+'uô'→'ươ'. Workflow: VNI 'được'+'6'→'đuộc'+'1'→'đuốc'; 'đuốc'+'7'→'đước'+'5'→'được'. 11/11 xanh.
- 2026-05-15: **Bugfix — toggle 'ươ'↔'uô' v1:** (1) Telex 'ươ'+'w' → revert 'uo' (strip cả diacritics+tone): `rightmostNucleusCharWithBase` không tìm được 'ơ'/'ư' vì base ≠ 'o'/'u' ASCII → thêm special case size==2 tonePattern=="ươ". (2) VNI 'uô'+'7' → 'ươ' (BOTH chars): rightmost chỉ tìm 'u' → 'đưốc' sai; thêm special case size==2 tonePattern=="uô" transform cả hai. Tương tự Telex 'uô'+'w'→'ươ'. 11/11 xanh.
- 2026-05-14: **Bugfix — âm đôi 'uo' VNI: 'đuợc'→'được', 'vuợt'→'vượt':** Root cause: VNI '7' transform chỉ đổi 'o'→'ơ' (cho 'uơ'), nhưng không có bước 2 đổi 'u'→'ư' như Telex (vốn dùng `normalizeTelexBuffer` trong push branch). Fix: thêm `normalizeVniUoTransform` — chỉ normalize khi nucleus đúng là "uơ" (size==2) + có coda, tránh phá "cưới" (nucleus "uơi") và "thuở" (open syllable). Gọi sau VNI transform trong engine. 11/11 tests xanh.
- 2026-05-14: **X11 click-away pre-interception (`X11ClickInterceptor`):** `xcb_grab_button` (GrabModeSync) trên root window khi preedit active → button press freeze → FarolKey nhận event TRƯỚC app → commit preedit tại P₀ (cursor cũ) → `xcb_allow_events(REPLAY_POINTER)` → click forward cho app → cursor đến Q. File: `include/farolkey/adapter/fcitx5/x11_click_interceptor.h` + `src/adapter/fcitx5/x11_click_interceptor.cpp`; CMake: auto-detect XCB, disable gracefully nếu `libxcb1-dev` chưa cài; stub no-op trên Wayland. Cần `sudo apt install libxcb1-dev` để enable. 11/11 xanh.

- 2026-05-15: **Dictionary Manager GUI (Phase 1):** Tạo `src/gui/farolkey-dict-gui` (Python + GTK3) — table Trigger/Expansion/Mode, Enable switch (đọc/ghi `farolkey.conf`), Add/Edit/Delete với EntryDialog popup, auto-save atomic (`.tmp` + `os.replace`), backup `.bak`. Systray `dictAction_` đổi từ toggle → launcher `fork()`+`execlp("farolkey-dict-gui")`. Config `enableUserDictionary` xóa khỏi systray (chỉ còn trong Dictionary Manager). CMakeLists: `install(PROGRAMS src/gui/farolkey-dict-gui DESTINATION bin)`. `install_local_fcitx5.sh`: copy → `~/.local/bin/farolkey-dict-gui`. Build OK, `farolkey-dict-gui` sẵn trong PATH.

- 2026-05-18: **Bugfix — Double commit toàn hệ thống ("koko"):** Root cause: `fcitx5-gtk` (GTK IM module trong process của mỗi GTK3 app) tự commit preedit khi `focus-out` *trước* khi báo daemon → daemon gọi `deactivate()` → engine cũng commit → 2 lần. Fix: xóa `ic->commitString()` khỏi `flushAndCleanup()`, `reset()`, và X11ClickInterceptor callback — chỉ clear state. Ảnh hưởng mọi GTK3 app (Files, Dictionary Manager…). ✓
- 2026-05-18: **Bugfix — Enable User Dictionary toggle:** (1) `parseBool` xử lý PascalCase `True`/`False`. (2) `loadConfigFile` nhận cả `enable_user_dictionary=` và `EnableUserDictionary=`. (3) Xóa `enableUserDictionary` khỏi `FarolKeyConfig` → không còn trong fcitx5-configtool. (4) Thêm `enableUserDict_` member + hot-reload (mtime check mỗi key event). (5) `saveConfig()` append `enable_user_dictionary=...` sau `safeSaveAsIni` để preserve. Dictionary Manager là sole controller. ✓
- 2026-05-18: **Bugfix — Dict Manager GUI: double commit expansion field + key matching:** `GtkSettings.gtk-im-module` switching per focus-in (trigger→simple, expansion→fcitx5). `_is_enable_key()` match cả hai format. ✓
- 2026-05-18: **Dictionary Manager Phase 2:** Engine: `defaultDictPath()` + `lastDictMtime_`, `maybeHotReloadConfig()` watch cả `farolkey.conf` và `user_dict.json` → auto-reload bridges khi dict thay đổi. GUI: Search/filter via `Gtk.TreeModelFilter` + `Gtk.SearchEntry`; Import/Export CSV (`csv.DictWriter/DictReader`); buttons `⬆ Import CSV` / `⬇ Export CSV`. ✓
- 2026-05-18: **Bugfix — `abbrev_mode` key name (linter renamed):** Linter đổi nhầm `abbrev_mode` → `mode` ở 7 vị trí trong `src/gui/farolkey-dict-gui`; engine đọc `abbrev_mode` từ JSON → tất cả entry default về `vi` dù đã set `en`/`both`. Fix: khôi phục đúng `abbrev_mode` toàn bộ; `CSV_HEADER` dùng `abbrev_mode`. ✓
- 2026-05-18: **Bugfix — EN mode expansion:** Rewrite `processEnglishKey()` trong `src/core/engine.cpp`: accumulate chars vào `preeditBuffer_`, lookup dict (`AbbrevMode::En`/`Both`) khi Space/separator, Backspace pop_back buffer. `setInputMode()` clear buffer khi switch mode. `engine_smoke_test.cpp` cập nhật: EN key `'a'` → preedit `"a"`, commit rỗng; Space → commit `"a "`. ✓
- 2026-05-18: **M12 Phase 1+2 — Clipboard History (Windows-style):** `docs/m12_clipboard_design.md`. `src/clipboard/farolkey-clipboard` rewrite: (1) **Bug fix**: xóa `focus-out-event`→destroy — popup không đóng khi kéo. (2) **Popup redesign**: `set_decorated(False)` + custom titlebar, `begin_move_drag()`, đóng chỉ khi select/Esc/X. Vị trí bottom-right (X11) / center (Wayland). CSS styling. "Clear all" + confirm dialog. (3) **Image support**: `wait_for_image()` → save PNG → thumbnail 72px height → `set_image()` on select. Auto-delete PNG khi evict. (4) **Hotkey**: GNOME `gsettings` custom keybinding `<Control><Super>v` → `farolkey-clipboard --show` (verified active). X11 fallback: `python-xlib` XGrabKey trong daemon thread. IPC: Unix socket `SHOW` command, bring-to-front nếu popup đã mở. Fcitx5: `clipboardAction_` systray. Build OK. ✓
- 2026-05-18: **M12.8–M12.10 — Clipboard History hoàn chỉnh:** (M12.8) `_try_auto_paste()`: ydotool/wtype Wayland + xdotool X11, fallback "✓ Copied" footer 1.2s; popup destroy + thread paste để focus trả về app trước. (M12.9) `deploy/autostart/farolkey-clipboard.desktop` → `~/.config/autostart/`, install script start daemon ngay. (M12.10) Real-time update: `map` signal → `_on_popup_mapped()` capture sau khi surface ready; `set_popup_notify()` callback trên monitor → `_refresh_popup()` mỗi khi clipboard thay đổi; PopupServer singleton bằng socket prevent spam. ✓
- 2026-05-18: **Fix test suite (11/11 xanh):** (1) `engine_smoke_test.cpp`: 7 test case dùng `typeSequence(e, "seq ")` với trailing space dư (double space) → xóa trailing space; `thuowf` typo → `thuowr` (hỏi ≠ huyền); `dduoc75` VNI sai → `d9uoc75`; VNI Bug2 `dd9` → `d9`; assertion `"D9"===""` sai → `"\xC4\x90 "`. (2) `vi_syllable_smoke_test.cpp`: M9.2 rewrite `findStableComposeSplit` (vector<bool> DP) có semantics khác `segmentWholeBufferWithPreference(buf, minimizeSyllableCount=true)` cũ → restore về gọi function cũ. (3) `corpus_driver.cpp`: thêm `cfg.enableUserDictionary = false` để corpus test không bị ảnh hưởng bởi user_dict.json cá nhân. (4) `engine_meta.jsonl:1`: EN mode giờ buffer preedit → update expect `{"preedit":"a","commit":""}`. ✓
- 2026-05-19: **M16 Screenshot Tool:** `src/screenshot/farolkey-screenshot` — capture fullscreen (grim/maim), `CaptureReviewOverlay` GTK3 fullscreen với toolbar + rubber-band crop, `SaveDialog`, `notify-send`. `farolkey-screenshot-daemon` (pynput hotkey), `farolkey-screenshot-settings` (GTK3 settings window, system tools status). Single-instance lock (`fcntl.flock`). Multi-monitor letterbox. `set_keep_above(True)` cho settings window. Auto-paste sau khi chọn clipboard item. CMakeLists + install.sh deps. ✓
- 2026-05-20: **Debug logging system:** C++: `src/common/logger.cpp` + `include/farolkey/common/logger.h` — file logger với rotation 5 MB → `~/.cache/farolkey/farolkey.log`; thread-safe (mutex); timestamp ms; levels Debug/Info/Warn/Error; fallback stderr. Log calls trong engine: init, activate/deactivate, reset, mode toggle (VI↔EN), commit (byte count only — không log nội dung). Python: `src/common/farolkey_log.py` — `RotatingFileHandler` → `~/.cache/farolkey/tools.log`; `get_logger(component)`. Log calls trong `farolkey-clipboard` (daemon start/stop, popup open/close, item selected N chars, X11/Wayland focus) và `farolkey-screenshot` (capture start/ok, mode, region, cancel, save path). **Privacy rule:** tuyệt đối không log văn bản người dùng gõ, clipboard content, hay dict expansion. ✓
- 2026-05-20: **Export log UI trong fcitx5-configtool:** `fcitx::ExternalOption exportLog` trong `FarolKeyConfig` → nút "Export log bundle (for bug reports)" xuất hiện tự động trong UI auto-generated của fcitx5-configtool. Script `src/common/farolkey-export-log` (GTK3): chọn log nào xuất (IME engine / tools / cả hai), chọn thư mục; 1 log → copy file trực tiếp, 2 log → tạo `.zip` gồm cả 2 + `system_info.txt` (OS, session type, fcitx5 version, tool availability). `export_log_bundle()` trong `farolkey_log.py`. Nút Export log cũng có trong clipboard settings và screenshot settings. ✓
- 2026-05-20: **README: mục Debug Log** — thêm phần "🔒 Debug Log — Hỗ trợ điều tra lỗi" với bảng rõ có/không có trong log, hướng dẫn xuất log 4 bước, đường dẫn file log. Nhấn mạnh bằng blockquote lớn: FarolKey KHÔNG lưu nội dung người dùng gõ. ✓
- 2026-05-20: **Bugfix — EN mode terminal (immediate commit):** Root cause: `processEnglishKey` buffer chữ vào preedit → terminal không nhận cho đến khi space. Fix: EN mode commit từng chữ ngay lập tức (`commit=char, preedit=""`); `preeditBuffer_` giữ vai trò internal word-tracker cho dict lookup. Khi space/boundary: nếu match EN dict/template → `deleteSurroundingBefore=N` + commit expansion (adapter dùng `deleteSurroundingText` nếu client hỗ trợ surrounding text). Backspace: `consumed=false` để app xử lý visually. Thêm `deleteSurroundingBefore` field vào `ProcessResult`. Adapter xử lý deletion trước commit. Engine smoke test + corpus cập nhật. 11/11 xanh. ✓
- 2026-05-20: **Bugfix VNI — 'người', 'tương', 'tươi' (uo+7 normalize):** Root cause: `normalizeVniUoTransform` chỉ normalize "uơ"→"ươ" khi `hasCoda=true` (size==2), bỏ sót 2 case: (1) "tương" — khi '7' gõ chưa có coda, sau khi 'ng' push vào không có normalize; (2) "người"/"tươi" — nucleus "uơi" (size==3) bị loại bởi size check. Fix: thêm tham số `fromPushPath=false` vào `normalizeVniUoTransform`; khi `fromPushPath=true` cũng xử lý "uơi" (size==3) → "ươi". Engine push path gọi `normalizeVniUoTransform(decoded, true)` như Telex gọi `normalizeTelexBuffer`. "thuở" (thuo73) và "cưới" (cuoi717) kiểm tra không bị phá. 4 corpus regression thêm vào `corpus/final/vni.jsonl`. 11/11 tests xanh. ✓
- 2026-05-20: **Bugfix VNI — 'người', 'tương', 'cưới' (i-before-7 case):** `fromPushPath` chỉ fix khi 'i' gõ SAU '7'; khi 'i' gõ TRƯỚC '7' (cuoi71, nguoi72) vẫn sai vì transform path dùng `fromPushPath=false`. Fix cuối: bỏ điều kiện `fromPushPath` cho "uơi" size==3 — normalize cả 2 path. Cập nhật corpus entry `cuoi717`→`cuoi71`. 11/11 xanh. ✓
- 2026-05-20: **Bugfix — EN mode terminal:** `processEnglishKey` commit từng char ngay (không preedit). `deleteSurroundingBefore` trong `ProcessResult` để adapter xóa raw chars trước khi commit expansion. Backspace `consumed=false`. Corpus + smoke test cập nhật. ✓
- 2026-05-20: **Bugfix — Click-away mất chữ trong browser (Google Sheets, Docs…):** Root cause: browser dùng `frontend='fcitx4'` (fcitx4 compatibility protocol) không có auto-commit; `reset()` gọi `commitPanelPreeditIfNeeded` lấy+xóa composition mà không commit (Client mode) → `deactivate` thấy bridge rỗng → mất chữ. Fix: (1) Thêm `commitForFocusLoss()` phân biệt frontend: "xim"/"waylandim"/"fcitx4" → explicit commit; "dbus" (GTK/fcitx5-gtk) → không commit để tránh koko. (2) `reset()` và `flushAndCleanup()` dùng `commitForFocusLoss`. Xác nhận hoạt động trên Firefox/Google Sheets. ✓
- 2026-05-20: **Release v0.1.1:** Build `farolkey_0.1.1_amd64.deb` (184KB). Bump version CMakeLists + install.sh. ✓
- 2026-05-25: **Bugfix Clipboard — Clear All đóng popup trên Wayland:** `_on_clear_all` thiếu `_settings_open = True` guard trước `dlg.run()` → focus-out destroy popup trước khi dialog kịp hiện. Fix: thêm guard cùng pattern với Settings dialog. ✓
- 2026-05-25: **Bugfix Clipboard — Arrow key không cuộn listbox (Wayland + X11):** Thêm `_scroll_to_row(row)` dùng `vadjustment` để giữ row được chọn trong viewport. Lưu `ScrolledWindow` vào `self._scroll`. Gọi qua `GLib.idle_add` để defer đến sau layout pass — cần thiết trên X11 vì `get_allocation()` trả về giá trị cũ nếu gọi đồng bộ ngay sau `select_row()`. ✓
- 2026-05-25: **Bugfix Clipboard — Fallback notification không hiện trên Wayland:** xdotool exit code 0 trên Wayland nhưng không thực sự paste được vào native Wayland window → `_try_auto_paste` trả `True` sớm, bỏ qua `_notify_paste_fallback`. Fix: trên Wayland chỉ trust ydotool/wtype; nếu cả hai fail thì xdotool vẫn chạy best-effort nhưng luôn gọi `_notify_paste_fallback()` bất kể kết quả. ✓
- 2026-05-25: **Bugfix Clipboard — ydotool gõ "2442" thay vì Ctrl+V:** ydotool 1.x không còn nhận format `29:1 47:1 47:0 29:0` (evdev keycode:value của v0.x) — lấy ký tự đầu mỗi argument → gõ "2442". Fix: đổi sang `['ydotool', 'key', '--key-delay', '50', 'ctrl+v']` (XKB keysym name, format chuẩn 1.x). ✓
- 2026-05-25: **Release v0.1.2:** Bump version CMakeLists + install.sh → `0.1.2`. ✓
- 2026-05-25: **Version display trong systray:** Thêm `versionAction_` (`fcitx::SimpleAction`) hiển thị nhãn `"v0.1.2"` trong systray (click = no-op); `FAROLKEY_VERSION` truyền từ `PROJECT_VERSION` qua `target_compile_definitions`. ✓
- 2026-05-25: **Fix build_deb.sh wrong version log:** Xóa `farolkey*.deb` cũ trước CPack; đọc version từ `CMakeLists.txt` và construct tên file chính xác thay vì dùng glob. ✓
- 2026-05-25: **Pre-release fix — `addon/farolkey.conf` Version:** `Version=0.1.0` → `Version=0.1.2` (hardcoded cũ). ✓
- 2026-05-25: **Pre-release fix — `farolkey-abbrev` trong .deb:** Di chuyển `add_executable(farolkey_abbrev)` ra khỏi `FAROLKEY_BUILD_CORPUS_TOOLS` block (OFF trong release) → luôn build; thêm `install(TARGETS farolkey_abbrev RUNTIME DESTINATION bin)`. ✓
- 2026-05-25: **Release notes v0.1.2:** `.rules/v0.1.2.md` — 4 bugfix Clipboard, hướng dẫn cài đặt. ✓
- 2026-05-25: **Build `farolkey_0.1.2_amd64.deb`:** 188758 bytes; xác nhận `farolkey-abbrev` + `farolkey.conf Version=0.1.2` trong package. ✓
- 2026-05-25: **Bugfix Clipboard — CSS provider accumulation (dark/light mode 3-state bug):** `add_provider_for_screen` đăng ký lên GdkScreen singleton; daemon restart không xóa provider cũ → sau 2–3 restart có nhiều provider xung đột, provider cũ có hardcoded color ghi đè theme variable. Fix: bỏ class-level guard, connect `destroy` signal → `remove_provider_for_screen`; CSS chỉ dùng `@theme_*` named colors. ✓
- 2026-05-25: **Screenshot — Multi-Monitor Overlay (M16 extension):** Thay thế `CaptureReviewOverlay(Gtk.Window)` (1 window letterbox) bằng kiến trúc mới: `_SelState` (shared absolute coords) + `MonitorOverlay` (1 fullscreen window per monitor via `fullscreen_on_monitor`) + `CaptureReviewOverlay` (plain class manager). Cross-monitor drag hoạt động nhờ implicit pointer grab (Wayland + X11). Visual: dim background + full-brightness subpixbuf overlay. Không dùng Cairo (tránh phụ thuộc `python3-gi-cairo`). Confirm on release (không cần Enter). ✓
- 2026-05-26: **Bugfix Telex — 'hoặc'→'hơạc' (oaGlide detection):** Root cause: `applyTelexTransform` key 'w' ưu tiên tìm 'o'/'u' → horn trước 'a' → breve; 'o' trong âm đệm "oa" bị biến thành 'ơ' thay vì 'a'→'ă'. Fix: thêm oaGlide check — khi `tonePatternBucket(nucleus[0])=='o'` và `tonePatternBucket(nucleus[1])=='a'` → target `rightmostNucleusCharWithBase(U"aă")` với `newSetIdx=1` (breve). Regression test `hoa+'w'→hoă` thêm vào `vi_syllable_smoke_test.cpp`. ALL TESTS PASSED. ✓
- 2026-05-26: **Feature Screenshot — Review mode + instant_capture option:** Sau khi thả chuột, mặc định vào chế độ review thay vì xác nhận ngay. 8 resize handles (4 góc + 4 cạnh) + move toàn bộ vùng. Esc trong review → quay về selecting (không về toolbar). Click ngoài vùng trong review → bắt đầu drag mới. Enter xác nhận từ cả selecting và review. Config key `instant_capture` (default `false`): nếu `true` → xác nhận ngay khi thả chuột (hành vi cũ). Settings UI thêm checkbox "Chụp ngay khi thả chuột" + hint. ✓
- 2026-05-26: **M17.1+M17.2+M17.3 — Smart Tone Normalization (oaGlide, commit-time):** Implement `correctToneBearingIndex` (internal helper) + `normalizeSyllableTonePlacement` (public, oaGlide-only, conservative) + hook vào `commitWithSuffix` (cả Telex + VNI). Fix: h→o→j→a→w→c cho "họăc" → "hoặc"; "hơặc" → "hoặc" (strip glide diacritic); "hợac" → "hoạc". Conservative scope: chỉ normalize khi b0=bucket 'o', b1=bucket 'a' (phonologically unambiguous). Không normalize "uy" và các pattern ambiguous. Corpus pass: 0 regression (engine_smoke_test là pre-existing bug không liên quan). ALL TESTS PASSED (91% = 10/11, 1 pre-existing). ✓
- 2026-05-26: **M17.4 — Smart Tone Normalization (realtime/preedit):** Lambda `applyPreeditNormalize` trong `processVietnameseKey` (engine.cpp) — gọi trong push path (Path B) sau `preeditBuffer_ = encodeUtf8(decoded)`, trước `maybeAutoCommitStablePrefix`. Push-path-only: với oaGlide, coda luôn được thêm qua push path → normalization đúng thời điểm (vd: sau 'c' push trên "họăc" → "hoặc" trong preedit). Transform path redundant vì open syllable không cần normalize (offset=0 → tone trên 'o' là đúng). Corpus: 11/11 pass (100% — stale binary issue fixed bằng rebuild với `FAROLKEY_BUILD_TESTS=ON`). ✓
- 2026-05-26: **Release v0.1.3:** Bump version CMakeLists + `deploy/fcitx5/addon/farolkey.conf` + `install.sh` → `0.1.3`. Build `farolkey_0.1.3_amd64.deb`. Release notes `.rules/v0.1.3.md`. ✓
- 2026-05-26: **Bugfix — Auto-commit preedit khi click ra ngoài (Chrome X11 + dbus):** Root cause: `commitForFocusLoss` skip toàn bộ "dbus" → Chrome cũng bị skip dù không tự auto-commit. Điều tra: cả Chrome lẫn GTK đều có `ClientUnfocusCommit=yes` (flag do fcitx5-dbus frontend set cho tất cả dbus clients). Fix: trong X11 interceptor callback, bọc commit+clear bằng `if (!ic->capabilityFlags().test(ClientUnfocusCommit))` — clients có flag được để yên, FocusOut tự xử lý auto-commit cho cả Chrome lẫn GTK. Kết quả: Chrome X11 auto-commit ✓, GTK không koko ✓. Chrome Wayland (ibus) không fix được ở plugin level — documented trong release notes. Rebuild `farolkey_0.1.3_amd64.deb`. ✓
- 2026-05-27: **Bugfix M17 — Mở rộng `normalizeSyllableTonePlacement` cho yGlide + uGlide:** Root cause: guard cứng `if (b0 != 'o') return false` khiến mọi pattern không bắt đầu bằng 'o' đều bị bỏ qua — cả commit (M17.3) lẫn realtime (M17.4). Fix: thêm `yGlide` (b0='y', cho "quyền") và `uGlide` (b0='u', b1='o', cho "cưới"/"hướng"). Cả hai pattern mới chỉ cần Step 1 (di chuyển tone), không cần Step 2 (strip diacritic). "uy" explicitly excluded (b1='y' ≠ 'o') vì ambiguous. 6 unit test mới (Cases 7–12) + 6 corpus entries (3 Telex + 3 VNI). 10/11 tests pass (1 pre-existing). ✓
- 2026-05-27: **Bugfix M17.6 — uyGlide trong `normalizeSyllableTonePlacement`:** Root cause: nucleus "uy*" (vd "nguyền", "thuyền") không được normalize vì b0='u', b1='y' không match uGlide (yêu cầu b1='o'). Fix: thêm `uyGlide` (nucleusLen≥3 && b0='u' && b1='y') — bao phủ cả tone misplaced trên 'u' (ngùyên→nguyền) lẫn trên 'y' (nguỳên→nguyền). "uy" 2-char vẫn excluded (ambiguous: "úy" vs "uý"). 4 unit test mới (Cases 13–16) + 6 corpus entries (3 Telex + 3 VNI). 1323/1323 corpus PASS. ✓
- 2026-05-27: **Bugfix M17.7 — ieGlide trong `normalizeSyllableTonePlacement`:** Root cause: nucleus "ie*"/"iê*" (vd "tiếng", "miền", "nhiều") không normalize khi tone misplaced trên 'i'. Fix: thêm `ieGlide = (b0=='i' && b1=='e')` — không cần nucleusLen restriction vì "ia" (b1='a') đã excluded tự nhiên (tone on 'i' là đúng trong "mía"). 4 unit test mới (Cases 17–20) + 6 corpus entries (3 Telex + 3 VNI). 1329/1329 corpus PASS. ✓
- 2026-05-27: **Bugfix M17.8 — ueGlide trong `normalizeSyllableTonePlacement`:** Root cause: nucleus "ue"/"uê" (vd "thuế", "tuệ") không normalize khi tone misplaced trên 'u'. Fix: thêm `ueGlide = (b0=='u' && b1=='e')`. "ua"/"ưa" (b1='a') và "uo*" (b1='o') đã handle bởi pattern khác, không bị ảnh hưởng. 4 unit test mới (Cases 21–24) + 6 corpus entries (3 Telex + 3 VNI). 1335/1335 corpus PASS. ✓
- 2026-05-27: **Bugfix M17.10 — oeGlide + sửa kToneRules "oe":** Root cause: `{U"oe", 0, 1}` đặt tone trên 'o' cho open syllable — sai về mặt ngữ âm học (tất cả từ "oe" tiếng Việt đều có tone trên 'e': khoé, khoẻ, xoè, loé). Fix: đổi thành `{U"oe", 1, 1}` + thêm oeGlide = (b0='o', b1='e') + cập nhật 2 selectToneVowelIndex tests + sửa 2 corpus entries sai cũ. 4 unit test mới (Cases 31–34) + 6 corpus entries (3+3). 675 Telex / 662 VNI PASS. ✓
- 2026-05-27: **Bugfix M17.11 — uaGlide:** Root cause: nucleus "ua"/"ưa" không có glide pattern match (b0='u', b1='a' không khớp uGlide/ueGlide). Fix: thêm uaGlide = (b0='u', b1='a') — không cần hasCoda vì correctToneBearingIndex phân biệt open/closed qua selectToneOffset. Bao phủ: "túan"→"tuán" (closed), "muá"→"múa" (open), "ưa" cũng xử lý đúng (tonePatternBucket('ư')='u'). 4 unit test mới (Cases 35–38) + 6 corpus entries. ✓
- 2026-05-27: **Bugfix M17.9 — iMedial/uMedial trong `normalizeSyllableTonePlacement`:** Root cause: hàm chỉ xét nucleus region `[medial_end, nucleus_end)`, bỏ qua tone đặt trên medial `[onset_end, medial_end)`. Khi user gõ sắc trên 'i' (sau 'g') hoặc 'u' (sau 'q'), nucleusLen=1 → guard `< 2` return false ngay. Fix: thêm medial tone check **trước** guard — nếu char cuối medial có tone ≠ ngang, di chuyển sang `correctToneBearingIndex(span)`. An toàn: medial ('u'/'i') không bao giờ là tone-bearing trong tiếng Việt. Covers: "gíup"→"giúp", "qúa"→"quá", "qùyên"→"quyền". 6 unit test mới (Cases 25–30) + 6 corpus entries (3 Telex + 3 VNI). 669 Telex / 656 VNI corpus PASS. ✓
- 2026-05-27: **Bugfix FB1 — Enter/Tab cần nhấn 2 lần (X11 Panel mode):** Root cause: `adjustCommitForPresentation` chỉ strip `'\n'` và forward Enter cho Client preedit mode. Với Panel mode (XIM / app không set `CapabilityFlag::Preedit`), Enter được embed trong commit string dưới dạng text — app nhận `"\n"` thay vì real Enter key event, Teams không gửi message, terminal không trigger tab completion. Fix: (1) Bỏ điều kiện `presentation == Client` trong `preedit_strategy.cpp` → áp dụng Enter forwarding cho mọi presentation mode; (2) Thêm Tab handling tương tự (strip `'\t'`, forward Tab key). Update `fcitx5_preedit_strategy_smoke_test.cpp`: 2 assertion cũ + 4 assertion mới (Tab, no-suffix). 10/11 tests PASS (1 pre-existing engine_smoke_test). ✓
- 2026-05-27: **Bugfix FB2 — EN mode backspace double text:** Root cause: Toggle handler gọi `setInputMode(EN)` nhưng không gọi `pushPreedit(ic_, "", ...)` để clear display preedit. VI preedit cũ vẫn còn embedded trong GTK text widget. Khi user gõ char đầu tiên trong EN mode, `commitString` → GTK auto-commit stale preedit TRƯỚC rồi mới insert char → doubled text. Fix: (1) Thêm `pushPreedit(ic_, "", ...)` trong toggle handler ngay sau `setInputMode`; (2) Guard `pushPreedit` trong keyEvent bằng `if (hadPreedit || !br.preedit().empty())` — skip khi preedit empty→empty (loại bỏ spurious `updatePreedit()` signal trong EN mode, tránh GTK timing race). Files: `farolkey_fcitx5_engine.cpp`. ✓
- 2026-05-27: **Release v0.1.4:** Bump version CMakeLists + `deploy/fcitx5/addon/farolkey.conf` + `install.sh` → `0.1.4`. Build `farolkey_0.1.4_amd64.deb`. Release notes `.rules/v0.1.4.md`. ✓
- 2026-05-27: **Bugfix FB2b — EN mode double text trên Firefox/fcitx4 (cả X11 và Wayland):** Root cause: Khi native key event không bị consume (ví dụ backspace `consumed=false`), Firefox/fcitx4 gọi XIM `ResetIC` để sync state → trigger `reset()` handler → `commitForFocusLoss` → `takeCompositionForCommit()` return `preeditBuffer_` (EN word-tracking buffer "hell") → `commitString("hell")` → doc = "hell"+"hell" = "hellhell". Tương tự: sau một số event khác, `preeditBuffer_` = "ch" bị commit lại → "chch". Root cause cốt lõi: `takeCompositionForCommit()` trong EN mode return tracking buffer như thể đó là VI preedit cần commit. Fix: Trong `engine.cpp::takeCompositionForCommit()`, nếu `mode_ == English` thì clear buffer và return "" (không commit). EN mode chars đã được committed individually qua commitString, không cần commit lại khi reset. Files: `src/core/engine.cpp`. 10/11 tests PASS (1 pre-existing). ✓
- 2026-05-28: **Bug UX1 — "Show underline while composing" không tắt được gạch chân ở Chrome/Edge/Electron:** Root cause: Chrome/Blink luôn render default compositing underline cho mọi preedit text, bất kể format flag từ IME (`TextFormatFlag::NoFlag` không tương đương "explicit no underline" như `IBUS_ATTR_UNDERLINE_NONE` của IBus). GTK apps (Firefox) dùng Pango attrs nên tôn trọng NoFlag, còn Chrome/Electron dùng Blink renderer tự thêm underline. Fix: (1) Thêm `FarolKeyPreeditMode` enum (Auto/Client/Panel) vào `FarolKeyConfig` — user có thể chọn Panel mode để tránh underline ở Chromium apps; (2) Cập nhật option description: "note: Chrome, Edge, Electron always show underline in Auto/Client mode". Files: `include/farolkey/adapter/fcitx5/farolkey_fcitx5_config.h`, `src/adapter/fcitx5/farolkey_fcitx5_engine.cpp`. ✓
- 2026-06-01: **M20 ✅ DONE — Settings GUI hoàn chỉnh:** Vá lỗi runtime (`set_im_module`, `show_uri_on_screen`, Switch stretching, version unknown, icon piano→keyboard); `_on_general_reset()` hiển thị diff table; `Configurable=False` trong cả hai `.conf`; toàn bộ UI chuyển sang English (fix mixed VI/EN inconsistency).
- 2026-06-01: **M18 — Auto-capitalize cleanup:** Revert re-focus mechanism (`everActivated_`, `icDestroyHandle_`, `isFirstActivation` param); giữ chỉ terminal fix (`isTerminalContext` + program name heuristic). Label UI: `'Auto-capitalize sentences (not fully optimized)'`. Re-focus bug ghi nhận là known limitation.
- 2026-06-01: **M21 ✅ DONE — Đa ngôn ngữ UI (i18n):** `farolkey_i18n.py` shared helper; `locales/en|vi/LC_MESSAGES/farolkey.po` (117 strings); Language dropdown trong Settings General tab (save `language=` vào farolkey.conf, restart required); `UIStrings` struct EN/VI trong C++ engine + `readUiLanguage()`; `tools/compile_po.py` pure-Python .po→.mo compiler (fix pybabel broken trên Python 3.12/Ubuntu 24.04). User test OK ✓
- 2026-06-01: **Release v0.1.5:** Bump version CMakeLists + `deploy/fcitx5/addon/farolkey.conf` + `install.sh` + `src/gui/farolkey-settings` + `locales/*/farolkey.po` → `0.1.5`. Release notes `.rules/v0.1.5.md`. ✓
- 2026-06-04: **M14.10 — Microsoft Vietnamese input method:** Direct char push (1→ă, 2→â, 3→ê, 4→ô, 7→ư, 8→ơ, 0→đ) + tone keys (s/j/5/6/9). `s` disambiguation: consonant nếu không có nguyên âm. 37 corpus cases, 6/6 smoke tests. ✓
- 2026-06-04: **Sửa lỗi i18n:** Nút `? Guide` hardcode VI → đổi msgid sang EN convention; label "Screenshot (hotkey)" trên systray không đổi khi chuyển ngôn ngữ → thêm `screenshotPrefix` vào `UIStrings`. ✓
- 2026-06-04: **Input method reference dialog (? Guide):** `MethodReferenceDialog` trong Settings — bảng phím tắt cho từng method, dịch sang VI. ✓
- 2026-06-04: **M14.11 — Tốc ký (VNI) input method:** VNI base + phụ âm rút gọn đầu từ (f→ph, j→gi, z→d, d→đ, w→ng, q→qu) + cuối từ (g→ng, h→nh, k→ch). k/c đầu từ = literal. Sau điều chỉnh: đổi tên systray/dropdown thành "Tốc ký (VNI)". 37 corpus, 7/7 smoke. ✓
- 2026-06-04: **M14.6 — VIQR* input method:** VIQR variant dùng `*` thay `+` cho dấu móc (ơ, ư). ✓
- 2026-06-04: **M14.7/M14.9 — Simple Telex & Simple Telex 2:** Tone key only (không expand diacritic); STel2 thêm `w` standalone → ư. ✓
- 2026-06-08: **M14.12 — Free Layout (Kiểu gõ Tuỳ biến):** Input method 100% do user tự định nghĩa. Shortcut table (key→Unicode, chỉ khi không có nguyên âm) + Tone/Diacritic map (10 actions, user gán phím). Context-based conflict. Config hot-reload `~/.config/farolkey/free_layout.json`. UI 2 cột trong Settings. Tone dedup tự động (Wayland-safe via `changed` signal). i18n đầy đủ. Display: "Tuỳ biến" (dropdown/systray), "Kiểu gõ Tuỳ biến" (tab). 24 corpus, 8/8 smoke. ✓
- 2026-06-08: **Bugfix Telex — "luuw" → "lưu":** nucleus "uu" + key 'w', `rightmostNucleusCharWithBase` lấy 'u' cuối → "uư" (sai). Fix: special case biến đổi 'u' đầu tiên → "ưu" (nguyên âm đôi hợp lệ). 3 regression corpus entries. ✓
- 2026-06-08: **Release v0.1.6:** Bump version 7 file → `0.1.6`. Release notes `.rules/v0.1.6.md`. ✓
- 2026-05-28: **Điều tra Bug FB3 — Enter 2 lần trên máy một user cụ thể (chưa có root cause):** User báo Enter cần nhấn 2 lần để gửi tin trong Teams (Edge) dù đã cài v0.1.4. Developer + user khác test cùng config không tái hiện. Đã xác minh trực tiếp: fcitx5 đã restart, config mặc định, cùng OS spec + fcitx5 version qua install.sh. Điều tra frontend: Edge trên máy dev → `dbus + x11 + preedit=1 + autoCmt=1` (same path as Chrome, FB1 fix đã cover). Insight quan trọng: log chỉ từ máy dev, chưa có log từ máy user thực tế. Tạo `tools/farolkey_frontend_probe.py` — Python script dùng dbus-monitor để capture frontend + capability của bất kỳ app nào, không cần rebuild/install. Bước tiếp theo: gửi script cho user chạy để xác minh config thực tế trên máy đó.

## 10. Checklist release version mới

Khi bump version `X.Y.Z → X.Y.Z+1`, sửa **đúng thứ tự** sau:

### Bước 1 — Sửa file nguồn (7 file)

| File | Dòng cần sửa |
|------|-------------|
| `CMakeLists.txt` | `project(FarolKey VERSION X.Y.Z ...)` |
| `install.sh` | `VERSION="X.Y.Z"` |
| `deploy/fcitx5/addon/farolkey.conf` | `Version=X.Y.Z` |
| `src/gui/farolkey-settings` | `_SCRIPT_VERSION = 'X.Y.Z'` |
| `locales/en/LC_MESSAGES/farolkey.po` | `"Project-Id-Version: FarolKey X.Y.Z\n"` |
| `locales/vi/LC_MESSAGES/farolkey.po` | same |
| `README.md` | tên file `.deb` trong lệnh cài đặt |

### Bước 2 — Rebuild plugin C++ (bắt buộc)

`FAROLKEY_VERSION` là **compile-time constant** baked vào `libfarolkey.so`. Nếu không rebuild, systray sẽ hiển thị version cũ dù các file khác đã đúng.

```bash
cmake --build build --target farolkey_fcitx5plugin
cp build/libfarolkey.so ~/.local/lib/fcitx5/libfarolkey.so
fcitx5-remote -r
```

### Bước 3 — Build .deb và tạo release

```bash
cmake --build build_deb
cd build_deb && cpack -G DEB
```

Tạo file `.rules/vX.Y.Z.md` — nội dung cho GitHub Release.

### Bước 4 — Cập nhật docs

- `progress.md` → thêm dòng vào **§9 Change Log**: `- DATE: **Release vX.Y.Z:** ...`
- `daily/YYYY_M_D.md` → ghi nhận release

---

## 11. Pending tasks (chờ điều kiện bên ngoài)

| Task | Mô tả | Chờ gì |
|------|--------|--------|
| **Bug FB3 — probe trên máy user** | Gửi `tools/farolkey_frontend_probe.py` cho user bị lỗi Enter 2 lần, yêu cầu chạy và paste output. Mục tiêu: xác nhận `frontend`, `preedit`, `autoCmt` thực tế trên máy đó để tìm root cause (khác máy dev). | User rảnh để test |

---

## 11. Ưu tiên triển khai code tiếp theo (không gồm QA / matrix app)

1. **M9.2** — ✅ DONE: `findStableComposeSplit` tối ưu; worst-case 2.59 µs (trong budget).
2. **M11.1** — ✅ DONE: `docs/user_guide.md`.
3. **M10.2** — ✅ Code done (`scripts/setup_apt_repo.sh` + `docs/apt_repo_guide.md`); cần infra để pilot.
3. **M10.2** — APT/repo nội bộ (khi có máy chủ ký gói); không chặn dùng `.deb` trực tiếp.
4. **M4.3** (tùy chọn) — grapheme / surrogate nếu có đường nhập thực tế từ client.

**Không lên lịch ở đây:** M6.4 (app matrix), M7 (QA theo app), benchmark tay từng app.
