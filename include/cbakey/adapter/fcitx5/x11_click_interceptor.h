#pragma once

#include <functional>
#include <memory>
#include <string>

namespace fcitx {
class Instance;
class AddonInstance;
class HandlerTableEntryBase;
} // namespace fcitx

namespace cbakey::adapter::fcitx5 {

/// Intercepts X11 mouse button press events BEFORE they reach the target app,
/// allowing the IME to commit preedit at the original cursor position.
///
/// Flow on X11:
///   user click → XCB grab fires → commit preedit at P₀ (old position)
///             → xcb_allow_events → click forwarded to app → cursor moves to Q
///
/// On Wayland: automatically becomes a no-op (Fcitx5 XCB module not present).
/// On XWayland: also no-op (XWayland clicks don't go through our root grab).
class X11ClickInterceptor {
public:
    using CommitFn = std::function<void()>;

    explicit X11ClickInterceptor(fcitx::Instance* instance);
    ~X11ClickInterceptor();

    X11ClickInterceptor(const X11ClickInterceptor&) = delete;
    X11ClickInterceptor& operator=(const X11ClickInterceptor&) = delete;

    /// True if running on X11 and the XCB module is available.
    bool isAvailable() const { return available_; }

    /// Start intercepting button press events.
    /// \p fn is called with preedit still at P₀ (before the click reaches the app).
    /// Safe to call even when not available (becomes a no-op).
    void startIntercepting(CommitFn fn);

    /// Stop intercepting. Call when preedit ends normally (committed via Space/Enter).
    /// Safe to call even when not intercepting.
    void stopIntercepting();

    bool isIntercepting() const { return intercepting_; }

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    bool available_  = false;
    bool intercepting_ = false;
};

} // namespace cbakey::adapter::fcitx5
