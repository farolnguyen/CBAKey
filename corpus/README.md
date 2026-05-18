# M0 — Corpus & runner (checkpoint / spec)

**Tiếng Việt | English:** tài liệu song ngữ; phần tiếng Anh ngắn gần cuối mỗi mục quan trọng.

**Nguồn sự thật cho milestone M0:** mọi thay đổi định dạng corpus hoặc semantics runner nên cập nhật file này **và** dòng tương ứng trong `progress.md` (mục M0).

---

## 1. Mục tiêu M0 (tóm tắt)

| ID | Nội dung |
|----|-----------|
| M0.1 | Thư mục `corpus/` chứa JSON Lines (`*.jsonl`), có **schema version** |
| M0.2 | Runner (GoogleTest) đọc corpus, báo cáo **pass / fail / skip** rõ ràng — target CMake: `cbakey_corpus_test` (`ctest -R cbakey_corpus_test`, `WORKING_DIRECTORY` = project root để thấy `corpus/`) |
| M0.3 | `docs/behavior_reference.md` — behavior reference song ngữ (tách file; bổ sung dần) |
| M0.4 | Phân tách prototype vs production — theo exit criteria trong `progress.md` |

*English:* M0 defines **test-only** contracts; product UX (auto-commit, word boundary) is specified separately in behavior reference / later milestones.

---

## 2. Vị trí & tên file (đã chốt)

- **Thư mục:** `corpus/` (tại root repo).
- **Tách file:** ví dụ `telex.jsonl`, `vni.jsonl`, `engine_meta.jsonl` (toggle, English, forward keys, …).
- **Bộ canonical:** khi có `corpus/final/`, đây là bộ **ưu tiên dùng cho regression**; các file ở `corpus/generated/` và root `corpus/` vẫn là source/seed để regenerate và rà thủ công.
- **Encoding:** UTF-8; chuỗi kỳ vọng Unicode ở dạng **NFC**.

*English:* Data under `corpus/`; split by concern; UTF-8 + NFC expected strings.

---

## 3. Định dạng JSON Lines (đã chốt)

- Mỗi dòng không phân cách là **một test case** (một object JSON hoàn chỉnh).
- **Schema version:** mỗi dòng **bắt buộc** có field `corpus_schema_version` (số nguyên). Runner chỉ chấp nhận version đã implement; version lạ → **fail** với log rõ (hoặc **skip** nếu policy runner ghi rõ — mặc định nên **fail** để không nuốt lỗi typo).

*English:* One JSON object per line; `corpus_schema_version` required per line.

---

## 4. Quyết định B2 — Kết thúc case & commit trong **corpus** (chốt cho M0)

**Phạm vi:** chỉ quy ước **harness kiểm thử**, không quyết định “IME thật có auto-commit kiểu Windows hay không” (phần đó nằm ở behavior reference + milestone sản phẩm).

1. **Không có commit “ngầm” từ runner**  
   Runner **không** tự thêm space/enter cuối case. Mọi phím kết thúc composition phải xuất hiện **trong** `sequence` (hoặc DSL tương đương mà parser expand ra đủ sự kiện).

2. **So khớp kỳ vọng theo “snapshot sau sự kiện cuối” (mặc định)**  
   Sau khi áp dụng lần lượt toàn bộ `sequence`, so sánh với `expect` dựa trên **`ProcessResult` của lần `processKey` cuối cùng**:
   - `expect.preedit` ≡ `last.preedit`
   - `expect.commit` ≡ `last.commit`  
   (và các field optional ở mục 7).

3. **Case có nhiều lần commit trong một dòng**  
   Nếu cần kiểm tra từng bước (ví dụ `viet` + space + `nam` + space), case **phải** cung cấp thêm một trong hai:
   - **`expect_trace`**: mảng cùng độ dài với `sequence` — mỗi phần tử là object kỳ vọng cho **đúng** lần `processKey` tại index đó; hoặc
   - **`expect_accumulated_commits`**: chuỗi nối **theo thứ tự** tất cả `commit` không rỗng từng bước (khi không dùng trace đầy đủ).  
   Runner implementation: nếu có `expect_trace` thì ưu tiên trace; nếu chỉ có `expect_accumulated_commits` thì assert chuỗi nối; nếu chỉ có `expect` mặc định thì chỉ assert **bước cuối** (case đó chỉ hợp hợp khi commit xảy ra một lần ở cuối hoặc chỉ quan tâm trạng thái cuối).

4. **Dấu cách / dấu câu trong `expect.commit`**  
   Phải ghi **đúng byte** kỳ vọng (kể cả space sau commit nếu engine trả về như smoke test hiện tại). Không “chuẩn hóa lại” trong runner trừ khi có test riêng cho NFC.

*English:* No implicit commit from runner; default assert last `ProcessResult`; multi-commit cases require `expect_trace` or `expect_accumulated_commits`.

---

## 5. Quyết định B3 — Đa âm tiết trong buffer (chốt cho M0)

1. **Corpus “bulk” ban đầu (generator / seed lớn):**  
   - Ưu tiên **một âm tiết + commit rõ** (space/enter/punct trong `sequence`), hoặc **nhiều tiếng qua ranh giới explicit** (space giữa các tiếng, mỗi đoạn có commit nếu cần).  
   → Tương ứng **B3b** trong thảo luận (ranh giới rõ trong input).

2. **Một composition chứa nhiều âm tiết liền (B3a, không có space giữa các tiếng trong buffer):**  
   - **Không bắt buộc** trong đợt corpus đầu tiên.  
   - Khi thêm: bắt buộc tag `multisyllable_buffer` (hoặc tag con chi tiết hơn sau này) và nên dùng **`expect_trace`** để tránh mơ hồ.

*English:* Default corpus = explicit boundaries; multi-syllable single-buffer cases are optional, tagged, trace-heavy.

---

## 6. Quyết định C1 — Độ chi tiết assert (chốt cho M0)

| Loại case | Cách assert |
|-----------|-------------|
| Mặc định | Chỉ `expect` khớp **kết quả bước cuối** (xem §4). |
| Regression UI/state tinh | Thêm `expect_trace` (per-key). |
| Nhiều commit | `expect_trace` hoặc `expect_accumulated_commits` (xem §4). |

Không bắt buộc mọi case đều có trace — tránh corpus phình vô ích.

*English:* Final-only by default; optional per-key trace; multi-commit rules as above.

---

## 7. Quyết định C2 — `consumed` / `forwardOriginalKey` (chốt cho M0)

- **Mặc định:** runner **không** assert `consumed` / `forwardOriginalKey` (chỉ `preedit` + `commit` nếu có trong `expect`).
- **Bật assert meta:** thêm field `expect.assert_meta: true` **hoặc** tag `needs_meta` / file `engine_meta.jsonl` chỉ chứa case cần meta. Khi bật, `expect` (hoặc từng bước trong `expect_trace`) **phải** có `consumed`, `forward_original_key` (tên field JSON snake_case khớp runner).

*English:* Meta assertions opt-in via `expect.assert_meta` or tags / dedicated jsonl.

---

## 8. Các quyết định khác (đã thống nhất trước đó)

| Mục | Chốt |
|-----|------|
| A1 | `corpus/` + `telex.jsonl`, `vni.jsonl`, … |
| A2 | JSON Lines + `corpus_schema_version` mỗi dòng |
| A3 | UTF-8, NFC |
| B1 | Syntax mở rộng từ đầu — biểu diễn `sequence` chi tiết (ký tự + aux + modifier); runner có parser/version riêng nếu sau này thêm DSL chuỗi |
| B4 | Có case English + toggle hotkey trong corpus (khuyến nghị file `engine_meta.jsonl` hoặc tag tương đương) |
| D1 | Tag càng chi tiết càng tốt; danh mục tag mở rộng theo nhu cầu sau khi có thêm case |
| D2 | Field **`id`** bắt buộc, ổn định để trace log CI (`id` + file + line) |
| E1 | Gen từ script và/hoặc nguồn free / licence chấp nhận được, không tốn phí |
| E2 | Cho **skip** / **fail**; log phải in rõ: `id`, file, lý do skip, diff fail |
| E3 | Ngân sách thời gian CI đặt sau khi corpus ổn định về kích thước |
| F1 | Có `docs/behavior_reference.md` |
| F1b | Song ngữ |
| G1 | **GoogleTest** cho runner corpus |
| G2 | Nếu có gen ngẫu nhiên: seed cố định trong script + ghi log; M0 không bắt buộc random |

---

## 9. Gợi ý schema dòng JSON (v1 — tham chiếu triển khai)

Runner thực tế có thể điều chỉnh nhỏ miễn giữ `corpus_schema_version` và các quy tắc §4–§7.

```json
{
  "corpus_schema_version": 1,
  "id": "T-000001",
  "tags": ["method.telex", "tone", "commit.space"],
  "config": "default",
  "sequence": [
    { "key": "a" },
    { "key": "a" },
    { "key": "s" },
    { "key": " " }
  ],
  "expect": {
    "preedit": "",
    "commit": "ấ "
  }
}
```

**Phím đặc biệt (ví dụ):** dùng object với `aux` thay cho `key` khi cần:

```json
{ "aux": "Enter" }
```

**Toggle (ví dụ):** theo `KeyEvent` thực tế của engine (kiểm tra `engine_smoke_test.cpp`):

```json
{ "key": "z", "ctrl": true, "alt": true }
```

Giá trị `aux` khớp tên enum C++ `KeyAux` trong `include/cbakey/core/types.h` (`Enter`, `Tab`, `Left`, …).

**Trường tùy chọn trên case:**

- `skip`: `true` — runner ghi **SKIP** + `id` + lý do (nếu có `skip_reason`).
- `expect.assert_meta`: `true` — thêm assert `consumed`, `forward_original_key`.
- `expect_trace`: mảng kỳ vọng per-step.
- `expect_accumulated_commits`: chuỗi (khi không dùng trace đầy đủ cho multi-commit).

---

## 10. Tag (D1) — hướng dẫn nhỏ

- Dùng **phân cấp bằng dấu chấm** nếu cần: `tone.telex`, `edge.qu`, `method.vni`, `commit.enter`, `mode.english`, `meta.forward`.
- Một case có thể có nhiều tag; tag phục vụ lọc báo cáo và skip theo nhóm.

---

## 11. English summary (M0 contracts)

- **Corpus location:** `corpus/**/*.jsonl`, UTF-8, **NFC** expected text.
- **Line format:** one JSON object per line; **`corpus_schema_version`** + stable **`id`** required.
- **B2:** Runner never auto-appends commit keys; default assert **last** `ProcessResult`; multi-commit cases require **`expect_trace`** or **`expect_accumulated_commits`**.
- **B3:** Default bulk cases use explicit boundaries; single-buffer multi-syllable cases are optional, tagged (`multisyllable_buffer`), prefer traces.
- **C1:** Default final-only assertions; optional per-key traces.
- **C2:** `consumed` / `forwardOriginalKey` assertions are **opt-in** (`expect.assert_meta`, tags, or `engine_meta.jsonl`).
- **Tooling:** GoogleTest-based runner; skip/fail must log **id + file + reason**; CI time budget TBD from corpus size.
- **Product UX** (Windows-like auto word boundary, etc.) is **not** decided here; document in **`docs/behavior_reference.md`** as the product evolves.

---

## 12. Runner triển khai (CMake)

- **Target:** `cbakey_corpus_test` — link `cbakey_core`, `GTest::gtest_main`, `nlohmann_json::nlohmann_json`.
- **Phát hiện file:** nếu có `corpus/final/`, runner **ưu tiên quét `corpus/final/**/*.jsonl`**; nếu không, fallback sang **đệ quy** mọi `corpus/**/*.jsonl` (theo thứ tự đường dẫn), bỏ qua dòng trống.
- **Skip:** dòng có `"skip": true` → in `[SKIP] …` ra stderr (kèm `skip_reason` nếu có); không fail test.
- **Lần build đầu:** CMake `FetchContent` tải **googletest** + **nlohmann/json** (cần mạng).

---

## 13. Bộ canonical `corpus/final/`

- **Mục đích:** giữ một bộ corpus ổn định để regression, không lẫn sample/raw/generated và không phụ thuộc thứ tự chạy script.
- **Script chốt bộ:** `scripts/finalize_corpus_set.py`
- **Quy tắc:**
  - ưu tiên curated/sample trước, rồi append unique generated entries;
  - dedupe theo hành vi thực sự của case (`sequence`, `expect`, `expect_trace`, `expect_accumulated_commits`, `config`, ...), **không** theo `id`;
  - merge `tags` khi hai dòng trùng semantics;
  - re-id ổn định thành `FINAL-telex-*`, `FINAL-vni-*`, `FINAL-meta-*`.
- **Trạng thái local hiện tại (2026-05-11):**
  - `corpus/final/telex.jsonl`: **539**
  - `corpus/final/vni.jsonl`: **500**
  - `corpus/final/engine_meta.jsonl`: **3**

*English:* `corpus/final/` is the canonical regression set; regenerate it via `scripts/finalize_corpus_set.py` after updating source/generated corpora.

---

## 14. Mở rộng corpus tự động (BFS + script)

### Công cụ C++: `cbakey_corpus_bfs`

- Build: `-DCBAKEY_BUILD_CORPUS_TOOLS=ON` (mặc định bật cùng flow thường dùng).
- **Ý tưởng:** BFS trên `Engine` thật — tìm chuỗi phím ASCII (Telex `a–z`, VNI thêm `0–9`) đưa preedit đúng **từ đích** (một âm tiết), rồi commit bằng **space**; xuất một dòng JSONL giống schema §9.
- **Gợi ý thứ tự phím (quan trọng):** truyền thêm **ascii_hint** (ví dụ NFKD bỏ dấu thanh: `chào` → `chao`) để ưu tiên thử các chữ trong skeleton Latin → giảm thời gian BFS (Telex dài có thể ~1 phút/từ nếu không gợi ý).
- **Môi trường:** `CBAKEY_CORPUS_BFS_MAX_NODES` (mặc định `1200000`), `CBAKEY_CORPUS_BFS_MAX_DEPTH` (mặc định `26`).

```bash
build/cbakey_corpus_bfs --one telex 'chào' chao
build/cbakey_corpus_bfs --batch < words.tsv   # mỗi dòng: telex<TAB>từ[<TAB>hint>]
```

### Script Python: `scripts/expand_corpus.py`

- Gợi ý `ascii_hint` tự động (NFKD + bỏ `Mn`, map `đ→d`).
- **Cache** tránh chạy lại BFS: `corpus/.bfs_cache.json` (đã `.gitignore`).
- **Song song:** `--jobs N` (mặc định ~một nửa CPU).
- **Nguồn từ:** cài `wordfreq` (MIT) hoặc `--words-file` (xem `corpus/sources/README.md`).

```bash
pip install -r scripts/requirements-corpus.txt
cmake -S . -B build -DCBAKEY_BUILD_CORPUS_TOOLS=ON -DCBAKEY_BUILD_TESTS=ON
cmake --build build

python3 scripts/expand_corpus.py --method telex --limit 500 --jobs 8 \
  --out corpus/generated/telex_wordfreq.jsonl

python3 scripts/expand_corpus.py --method vni --limit 500 --jobs 8 \
  --out corpus/generated/vni_wordfreq.jsonl
```

Từ **không** tìm được trong giới hạn node/depth sẽ `[skip]` trên stderr; có thể tăng `--max-nodes` / `--timeout` hoặc bổ sung hint tay trong file batch.

### English (short)

- **`cbakey_corpus_bfs`:** offline BFS against the real `Engine`; optional **ASCII hint** (NFKD base letters) reorders the try-alphabet to shrink search time.
- **`expand_corpus.py`:** optional **wordfreq** word source, on-disk **cache**, parallel workers; writes JSONL for `cbakey_corpus_test`.

---

## 15. Change log (schema / spec)

| Date | Change |
|------|--------|
| 2026-05-11 | Initial M0 checkpoint: user decisions + locked B2–B3, C1–C2 for corpus-only semantics; example schema v1. |
| 2026-05-11 | Implemented `cbakey_corpus_test` + sample `telex.jsonl` / `vni.jsonl` / `engine_meta.jsonl`. |
| 2026-05-11 | Added `cbakey_corpus_bfs`, `scripts/expand_corpus.py`, recursive JSONL discovery, `corpus/generated/` sample. |
| 2026-05-11 | Added canonical `corpus/final/`, `scripts/finalize_corpus_set.py`, and runner preference for canonical corpus when available. |
