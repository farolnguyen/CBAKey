#include "cbakey/adapter/fcitx5/x11_click_interceptor.h"

#ifdef CBAKEY_ENABLE_X11

// Only include xcb/xcb.h (from libxcb1-dev). Intentionally NOT including
// xcb_public.h or xcb_ewmh.h to avoid the libxcb-ewmh-dev dependency.
#include <xcb/xcb.h>

#include <fcitx-utils/event.h>
#include <fcitx/instance.h>

#include <cstdlib>

namespace cbakey::adapter::fcitx5 {

struct X11ClickInterceptor::Impl {
    fcitx::EventLoop*  eventLoop = nullptr;
    xcb_connection_t*  conn      = nullptr;
    xcb_window_t       root      = XCB_NONE;

    X11ClickInterceptor::CommitFn commitFn;
    bool grabActive = false;

    // IO watcher on our XCB connection fd — kept alive while watching.
    std::unique_ptr<fcitx::EventSourceIO> ioWatch;

    bool init(fcitx::Instance* instance) {
        // Only activate on X11 sessions (DISPLAY set, WAYLAND_DISPLAY not set).
        const char* display  = std::getenv("DISPLAY");
        const char* wayland  = std::getenv("WAYLAND_DISPLAY");
        if (!display || !display[0] || (wayland && wayland[0])) {
            return false;
        }

        int screenNum = 0;
        conn = xcb_connect(display, &screenNum);
        if (!conn || xcb_connection_has_error(conn)) {
            if (conn) { xcb_disconnect(conn); conn = nullptr; }
            return false;
        }

        // Walk to the right screen and grab its root window.
        const xcb_setup_t* setup = xcb_get_setup(conn);
        xcb_screen_iterator_t it = xcb_setup_roots_iterator(setup);
        for (int i = 0; i < screenNum && it.rem > 0; ++i) xcb_screen_next(&it);
        if (!it.rem) {
            xcb_disconnect(conn);
            conn = nullptr;
            return false;
        }
        root = it.data->root;
        eventLoop = &instance->eventLoop();
        return true;
    }

    ~Impl() {
        stopGrab();
        if (conn) { xcb_disconnect(conn); conn = nullptr; }
    }

    void startGrab(X11ClickInterceptor::CommitFn fn) {
        if (grabActive || !conn || root == XCB_NONE) return;
        commitFn   = std::move(fn);
        grabActive = true;

        // Passive grab: ALL buttons, ANY modifier, on the root window.
        // GrabModeSync for pointer: pointer events freeze on button press
        // until we call xcb_allow_events — giving us a window to commit
        // preedit before the click reaches the target app.
        xcb_grab_button(
            conn,
            /*owner_events=*/0,
            root,
            XCB_EVENT_MASK_BUTTON_PRESS,
            XCB_GRAB_MODE_SYNC,   // freeze pointer on press
            XCB_GRAB_MODE_ASYNC,  // keyboard stays async
            XCB_NONE,
            XCB_NONE,
            XCB_BUTTON_INDEX_ANY,
            XCB_MOD_MASK_ANY);
        xcb_flush(conn);

        // Watch the XCB fd in Fcitx5's event loop.
        const int fd = xcb_get_file_descriptor(conn);
        ioWatch = eventLoop->addIOEvent(
            fd, fcitx::IOEventFlag::In,
            [this](fcitx::EventSourceIO*, int, fcitx::IOEventFlags) -> bool {
                drainEvents();
                return true;
            });
    }

    void stopGrab() {
        if (!grabActive || !conn) return;
        ioWatch.reset();
        xcb_ungrab_button(conn, XCB_BUTTON_INDEX_ANY, root, XCB_MOD_MASK_ANY);
        xcb_flush(conn);
        grabActive = false;
        commitFn   = nullptr;
    }

    void drainEvents() {
        xcb_generic_event_t* ev;
        while ((ev = xcb_poll_for_event(conn))) {
            const uint8_t type = ev->response_type & ~0x80u;
            if (type == XCB_BUTTON_PRESS && grabActive) {
                auto* btn = reinterpret_cast<xcb_button_press_event_t*>(ev);

                // Commit preedit BEFORE releasing the pointer freeze.
                // At this point the app hasn't received the click yet,
                // so the cursor is still at the original preedit position.
                if (commitFn) {
                    commitFn();
                    commitFn = nullptr;
                }

                // Unfreeze and replay the click so the app receives it normally.
                xcb_allow_events(conn, XCB_ALLOW_REPLAY_POINTER, btn->time);
                xcb_flush(conn);
                grabActive = false;
                ioWatch.reset();
            }
            std::free(ev);
        }
    }
};

// ── Public API ────────────────────────────────────────────────────────────────

X11ClickInterceptor::X11ClickInterceptor(fcitx::Instance* instance)
    : impl_(std::make_unique<Impl>()) {
    available_ = impl_->init(instance);
}

X11ClickInterceptor::~X11ClickInterceptor() {
    impl_->stopGrab();
}

void X11ClickInterceptor::startIntercepting(CommitFn fn) {
    if (!available_) return;
    impl_->startGrab(std::move(fn));
    intercepting_ = true;
}

void X11ClickInterceptor::stopIntercepting() {
    if (!available_) return;
    impl_->stopGrab();
    intercepting_ = false;
}

} // namespace cbakey::adapter::fcitx5

#else // CBAKEY_ENABLE_X11 not defined — no-op stub

namespace cbakey::adapter::fcitx5 {

struct X11ClickInterceptor::Impl {};

X11ClickInterceptor::X11ClickInterceptor(fcitx::Instance*)
    : impl_(std::make_unique<Impl>()) { available_ = false; }
X11ClickInterceptor::~X11ClickInterceptor() = default;
void X11ClickInterceptor::startIntercepting(CommitFn) {}
void X11ClickInterceptor::stopIntercepting() {}

} // namespace cbakey::adapter::fcitx5

#endif // CBAKEY_ENABLE_X11
