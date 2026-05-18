# M12 — Clipboard History: Architecture & Design

## Decision

**Architecture:** Standalone Python + GTK3 daemon (`cbakey-clipboard`), NOT a Fcitx5 C++ addon.

**Why not a Fcitx5 addon:**
- Clipboard protocol (X11 XAtom / Wayland wl-data-device) is orthogonal to IME logic.
- Daemon model avoids coupling clipboard lifetime to Fcitx5 process restart.
- Consistent with Dictionary Manager pattern (Python + GTK3 → reuse toolkit).
- Easier to package separately or disable without affecting IME.

**Hotkey integration:** CBAKey Fcitx5 engine adds a systray action "Clipboard History"
that `fork()`+`execlp("cbakey-clipboard", "--show")` — same pattern as Dict Manager.
For the daemon autostart, added to Fcitx5 autostart or user `~/.config/autostart/`.

---

## Scope (V1)

| Slice | Feature | Included |
|-------|---------|---------|
| Data model | In-memory queue (max 50 items) + JSON persist | ✅ |
| Monitoring | `Gtk.Clipboard` `owner-change` signal | ✅ |
| UI | GTK3 popup: list + search + keyboard nav | ✅ |
| Paste | Set clipboard content → user Ctrl+V | ✅ |
| Dedup | Skip identical consecutive entries | ✅ |
| Privacy | Skip if clipboard mime contains `x-kde-passwordManagerHint` or source window name matches known password managers | ✅ |
| Images | Text-only V1; image support deferred | ❌ V2 |
| Rich text | Plain text only | ❌ V2 |
| Wayland hotkey | Fcitx5 systray action (no XGrabKey dependency) | ✅ |

---

## File Layout

```
src/clipboard/
  cbakey-clipboard        # Python daemon + UI (single file)
docs/m12_clipboard_design.md
~/.local/share/cbakey/
  clipboard_history.json  # persisted history (plain text entries)
```

---

## Data Model

```python
@dataclass
class ClipboardEntry:
    text: str                    # plain text content
    timestamp: float             # unix time
    pinned: bool = False

class ClipboardHistory:
    MAX_ITEMS = 50
    entries: list[ClipboardEntry]  # newest first
```

Persistence: JSONL (one JSON object per line), newest-first, max 50 lines.
File: `~/.local/share/cbakey/clipboard_history.json`

---

## Popup UI (GTK3)

- `Gtk.Window` type `POPUP` (no titlebar, no taskbar), centered on screen or near mouse.
- `Gtk.SearchEntry` at top — filters entries live.
- `Gtk.ListBox` — each row: truncated preview (first 80 chars, single line), timestamp hint.
- Keyboard: `↑↓` navigate, `Enter` select + close, `Esc` dismiss.
- Click row: select + close.
- On select: `Gtk.Clipboard.set_text(entry.text)` — user pastes with Ctrl+V.
- Pin button (📌) per row — pinned entries never auto-evict.

---

## Privacy Policy

Skip storing entry if:
- Clipboard text is empty or whitespace-only.
- Owner window has `_NET_WM_NAME` matching known password managers
  (KeePass, KeePassXC, 1Password, Bitwarden, gnome-keyring, secret-tool).
- Clipboard has `x-kde-passwordManagerHint` target atom.
- Text matches common password pattern heuristic (>= 8 chars, no spaces,
  mixed case + digit + special char) AND source is in password manager list above.

Privacy doc referenced: `docs/logging_policy.md`.

---

## Wayland / X11 Notes

- `Gtk.Clipboard.get(Gdk.SELECTION_CLIPBOARD)` + `owner-change` works on both.
- No XGrabKey dependency — hotkey comes from Fcitx5 systray action.
- Paste simulation deferred: V1 only sets clipboard; user presses Ctrl+V manually.
  V2 can add `xdotool` (X11) / `ydotool` (Wayland) for auto-paste.
