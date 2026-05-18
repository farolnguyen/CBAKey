# Nguồn từ / word list sources

## Khuyến nghị (MIT): `wordfreq`

Thư viện **[wordfreq](https://github.com/rspeer/wordfreq)** (MIT) cung cấp tần suất theo ngôn ngữ, gồm tiếng Việt (`vi`).

```bash
pip install -r scripts/requirements-corpus.txt
```

Script `scripts/expand_corpus.py` gọi `wordfreq.top_n_list("vi", n=...)` khi không truyền `--words-file`.

## File từ tùy chỉnh

- Mỗi dòng **một từ** (một âm tiết hoặc một token), UTF-8, **NFC**.
- Tránh dòng trống, khoảng trắng trong từ (BFS hiện chỉ tối ưu cho **một âm tiết** / preedit đơn).
- Có thể trộn nguồn: xuất nhiều file rồi `sort -u`.

## Giấy phép

- **wordfreq:** MIT (xem repo upstream).
- Nếu bạn thêm file từ nguồn khác, ghi rõ **LICENSE** cạnh file trong thư mục này.
