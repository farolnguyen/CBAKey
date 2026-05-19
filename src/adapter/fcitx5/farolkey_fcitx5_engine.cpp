#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <unistd.h>

#include <fcitx/action.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/event.h>
#include <fcitx/instance.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/inputpanel.h>
#include <fcitx/menu.h>
#include <fcitx/statusarea.h>
#include <fcitx/text.h>
#include <fcitx/userinterfacemanager.h>
#include <fcitx-config/iniparser.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/keysym.h>
#include <fcitx-utils/capabilityflags.h>
#include <fcitx-utils/textformatflags.h>

#include "farolkey/adapter/fcitx5/bridge.h"
#include "farolkey/adapter/fcitx5/farolkey_fcitx5_config.h"
#include "farolkey/adapter/fcitx5/committed_rewrite_fcitx5.h"
#include "farolkey/adapter/fcitx5/compose_anchor_fcitx5.h"
#include "farolkey/adapter/fcitx5/preedit_strategy.h"
#include "farolkey/adapter/fcitx5/x11_click_interceptor.h"
#include "farolkey/config/config.h"
#include "farolkey/core/types.h"

namespace {

// ── Dynamic SVG mode icons ────────────────────────────────────────────────────
//
// Two SVG files written to ~/.cache/farolkey/ at startup.
// subModeIconImpl() returns the appropriate path so Fcitx5 uses them as the
// tray icon — the icon changes when the user toggles EN/VI.

constexpr std::string_view kViIconSvg = R"svg(<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 48 48" width="48" height="48">
  <rect width="48" height="48" rx="10" fill="#27AE60"/>
  <text x="24" y="33" font-family="sans-serif" font-size="20" font-weight="bold"
        text-anchor="middle" fill="white" letter-spacing="1">VI</text>
</svg>)svg";

constexpr std::string_view kEnIconSvg = R"svg(<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 48 48" width="48" height="48">
  <rect width="48" height="48" rx="10" fill="#7F8C8D"/>
  <text x="24" y="33" font-family="sans-serif" font-size="20" font-weight="bold"
        text-anchor="middle" fill="white" letter-spacing="1">EN</text>
</svg>)svg";

// Write icon SVG to path (always overwrite to keep in sync with binary).
void writeIconFile(const std::string& path, std::string_view content) {
    std::ofstream f(path, std::ios::trunc);
    if (f) f << content;
}

// Resolve icon paths using a priority chain:
//   1. /usr/share/farolkey/icons/  — installed by .deb package (best for release)
//   2. ~/.local/share/farolkey/icons/ — user-local install (scripts/install_local_fcitx5.sh)
//   3. ~/.cache/farolkey/           — generated at runtime (fallback / dev mode)
//
// For paths 1 and 2 the files are installed by the package; for path 3 we
// generate them from the embedded SVG strings so the feature works out of the box
// even before the icon files are explicitly installed.
std::pair<std::string, std::string> initModeIcons() {
    // Search for pre-installed icons first.
    static const char* kSystemDirs[] = {
        "/usr/share/farolkey/icons",
        "/usr/local/share/farolkey/icons",
        nullptr
    };
    for (const char** d = kSystemDirs; *d; ++d) {
        const std::string vi = std::string(*d) + "/mode_vi.svg";
        const std::string en = std::string(*d) + "/mode_en.svg";
        if (std::filesystem::exists(vi) && std::filesystem::exists(en))
            return {vi, en};
    }

    // User-local install (XDG_DATA_HOME).
    const char* dataBase = std::getenv("XDG_DATA_HOME");
    const std::string localDataDir =
        (dataBase && dataBase[0]) ? std::string(dataBase) + "/farolkey/icons"
                                  : std::string(std::getenv("HOME") ? std::getenv("HOME") : ".")
                                        + "/.local/share/farolkey/icons";
    {
        const std::string vi = localDataDir + "/mode_vi.svg";
        const std::string en = localDataDir + "/mode_en.svg";
        if (std::filesystem::exists(vi) && std::filesystem::exists(en))
            return {vi, en};
    }

    // Fallback: generate icons into XDG_CACHE_HOME (~/.cache/farolkey/).
    // This ensures the feature works in dev mode and on fresh installs before
    // the icons are placed in a data directory.
    const char* cacheBase = std::getenv("XDG_CACHE_HOME");
    const std::string cacheDir =
        (cacheBase && cacheBase[0]) ? std::string(cacheBase) + "/farolkey"
                                    : std::string(std::getenv("HOME") ? std::getenv("HOME") : ".")
                                          + "/.cache/farolkey";

    std::filesystem::create_directories(cacheDir);
    const std::string viPath = cacheDir + "/mode_vi.svg";
    const std::string enPath = cacheDir + "/mode_en.svg";
    writeIconFile(viPath, kViIconSvg);
    writeIconFile(enPath, kEnIconSvg);
    return {viPath, enPath};
}

// ── Utility ───────────────────────────────────────────────────────────────────

std::string defaultConfigPath() {
    const char* home = std::getenv("HOME");
    return home ? std::string(home) + "/.config/farolkey/farolkey.conf" : "farolkey.conf";
}

std::string defaultDictPath() {
    const char* xdg  = std::getenv("XDG_CONFIG_HOME");
    const char* home = std::getenv("HOME");
    const std::string base = (xdg && xdg[0]) ? std::string(xdg)
                                              : std::string(home ? home : ".") + "/.config";
    return base + "/farolkey/user_dict.json";
}

farolkey::config::RuntimeConfig toRuntimeConfig(
    const farolkey::adapter::fcitx5::FarolKeyConfig& cfg, bool enableUserDict) {
    farolkey::config::RuntimeConfig rc = farolkey::config::defaultConfig();
    rc.method = (cfg.method.value() == farolkey::adapter::fcitx5::FarolKeyMethod::Telex)
                    ? farolkey::core::InputMethod::Telex
                    : farolkey::core::InputMethod::Vni;
    rc.enableUserDictionary   = enableUserDict;
    rc.fcitx5CommittedRewrite = cfg.committedRewrite.value();
    return rc;
}

farolkey::adapter::fcitx5::PreeditCapabilitySnapshot snapshotCaps(fcitx::InputContext* ic) {
    return {ic->capabilityFlags().test(fcitx::CapabilityFlag::Preedit)};
}

void pushPreedit(fcitx::InputContext* ic,
                 const std::string& preedit,
                 farolkey::config::Fcitx5PreeditMode mode,
                 bool underline) {
    auto& panel = ic->inputPanel();
    panel.setClientPreedit(fcitx::Text());
    panel.setPreedit(fcitx::Text());
    if (!preedit.empty()) {
        const auto fmt = underline ? fcitx::TextFormatFlag::Underline
                                   : fcitx::TextFormatFlag::NoFlag;
        fcitx::Text text(preedit, fmt);
        text.setCursor(static_cast<int>(preedit.size()));
        if (farolkey::adapter::fcitx5::choosePreeditPresentation(mode, snapshotCaps(ic)) ==
            farolkey::adapter::fcitx5::PreeditPresentation::Client) {
            panel.setClientPreedit(text);
        } else {
            panel.setPreedit(text);
        }
    }
    ic->updatePreedit();
    ic->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
}

// ── Engine ────────────────────────────────────────────────────────────────────

class FarolKeyFcitx5Engine final : public fcitx::InputMethodEngineV2 {
public:
    explicit FarolKeyFcitx5Engine(fcitx::Instance* instance)
        : instance_(instance)
        , interceptor_(
              std::make_unique<farolkey::adapter::fcitx5::X11ClickInterceptor>(instance)) {
        auto [vi, en] = initModeIcons();
        viIconPath_ = std::move(vi);
        enIconPath_ = std::move(en);
        loadConfig();
        setupActions();
    }

    ~FarolKeyFcitx5Engine() override {
        auto& ui = instance_->userInterfaceManager();
        ui.unregisterAction(&modeAction_);
        ui.unregisterAction(&methodMenuAction_);
        ui.unregisterAction(&telexAction_);
        ui.unregisterAction(&vniAction_);
        ui.unregisterAction(&dictAction_);
        ui.unregisterAction(&clipboardAction_);
        ui.unregisterAction(&underlineAction_);
    }

    // ── Fcitx5 config framework ─────────────────────────────────────────────

    const fcitx::Configuration* getConfig() const override { return &config_; }

    void setConfig(const fcitx::RawConfig& raw) override {
        config_.load(raw, /*partial=*/true);
        fcitx::safeSaveAsIni(config_, defaultConfigPath());
        applyConfigToAllBridges();
        refreshActionStates();
        // Notify all active ICs so the status area redraws immediately.
        refreshAllStatusAreas();
    }

    // ── IME entry list ──────────────────────────────────────────────────────

    // ── Dynamic mode icon + label ────────────────────────────────────────────
    //
    // subModeIconImpl  → file path to the SVG that Fcitx5 uses as the tray icon.
    //   VI mode: green  "VI" badge  (~/.cache/farolkey/mode_vi.svg)
    //   EN mode: gray   "EN" badge  (~/.cache/farolkey/mode_en.svg)
    //
    // subModeLabelImpl → short text label shown in Fcitx5's panel/tooltip.
    //
    // Both are re-queried by Fcitx5 after every updateUserInterface(StatusArea)
    // call, so the icon updates in real-time when the user toggles the mode.

    std::string subModeIconImpl(const fcitx::InputMethodEntry&,
                                fcitx::InputContext&) override {
        return (globalMode_ == farolkey::core::InputMode::Vietnamese)
                   ? viIconPath_ : enIconPath_;
    }

    std::string subModeLabelImpl(const fcitx::InputMethodEntry&,
                                 fcitx::InputContext&) override {
        return (globalMode_ == farolkey::core::InputMode::Vietnamese) ? "VI" : "EN";
    }

    // ── IME entry list ──────────────────────────────────────────────────────

    std::vector<fcitx::InputMethodEntry> listInputMethods() override {
        std::vector<fcitx::InputMethodEntry> entries;
        entries.emplace_back("farolkey", "FarolKey", "vi", "farolkey");
        entries.back().setLabel("VI").setConfigurable(true);
        return entries;
    }

    // ── Key event ───────────────────────────────────────────────────────────

    void keyEvent(const fcitx::InputMethodEntry& entry,
                  fcitx::KeyEvent& keyEvent) override {
        FCITX_UNUSED(entry);
        if (keyEvent.isRelease()) return;

        maybeHotReloadConfig();

        // ── Check configured toggle hotkey FIRST ────────────────────────────
        // The engine has a hardcoded Ctrl+Alt+Z check, but the user may have
        // configured a different toggle key.  Handle it here so the config key
        // always works, and consume the event before it reaches the engine.
        if (keyEvent.key().check(config_.toggleKey.value())) {
            auto* ic_ = keyEvent.inputContext();
            globalMode_ = (globalMode_ == farolkey::core::InputMode::Vietnamese)
                              ? farolkey::core::InputMode::English
                              : farolkey::core::InputMode::Vietnamese;
            for (auto& [_, br_] : bridges_) br_.setInputMode(globalMode_);
            refreshModeAction(ic_);
            refreshAllStatusAreas();
            keyEvent.filterAndAccept();
            return;
        }

        farolkey::core::KeyEvent ev;
        if (!translateKey(keyEvent.key(), ev)) return;

        auto* ic   = keyEvent.inputContext();
        auto& br   = bridgeFor(ic);
        const bool underline = config_.showPreeditUnderline.value();

        if (br.inputMode() == farolkey::core::InputMode::Vietnamese && br.preedit().empty()) {
            if (farolkey::adapter::fcitx5::tryApplyCommittedSyllableRewrite(ic, br.config(), ev)) {
                pushPreedit(ic, "", br.config().fcitx5PreeditMode, underline);
                keyEvent.filterAndAccept();
                return;
            }
        }

        const bool hadPreedit = !br.preedit().empty();
        const auto result = br.handleKey(ev);

        // Sync globalMode_ with the bridge's actual mode. The engine may have
        // toggled the mode internally (e.g. via the hardcoded Ctrl+Alt+Z hotkey)
        // without going through the adapter's action callback, so globalMode_
        // might be stale. Detecting the mismatch here keeps everything in sync.
        if (br.inputMode() != globalMode_) {
            globalMode_ = br.inputMode();
            for (auto& [otherIc, otherBr] : bridges_) {
                if (otherIc != ic) otherBr.setInputMode(globalMode_);
            }
            refreshModeAction(ic);
        }

        if (!result.consumed) return;

        const auto dispatch = farolkey::adapter::fcitx5::adjustCommitForPresentation(
            farolkey::adapter::fcitx5::choosePreeditPresentation(br.config().fcitx5PreeditMode,
                                                                snapshotCaps(ic)),
            ev.aux, result.commit);

        if (!dispatch.commit.empty() && ic)
            ic->commitString(dispatch.commit);

        pushPreedit(ic, br.preedit(), br.config().fcitx5PreeditMode, underline);

        if (br.preedit().empty()) {
            composeAnchors_.erase(ic);
            interceptor_->stopIntercepting();
        } else {
            farolkey::adapter::fcitx5::ComposeAnchorSnapshot snap;
            farolkey::adapter::fcitx5::refreshComposeAnchorForPreedit(ic, br.preedit(), &snap);
            composeAnchors_[ic] = snap;
            if (!hadPreedit && interceptor_->isAvailable() && !interceptor_->isIntercepting()) {
                interceptor_->startIntercepting([this, ic]() {
                    auto it = bridges_.find(ic);
                    if (it == bridges_.end()) return;
                    // Take (and discard) the composition to clear engine state.
                    // Do NOT call ic->commitString() here: fcitx5-gtk already
                    // commits the preedit to the GTK entry when focus-out fires
                    // (before telling the daemon).  If we also commit here we
                    // produce a double commit ("koko" instead of "ko").
                    it->second.takeCompositionForCommit();
                    pushPreedit(ic, "", it->second.config().fcitx5PreeditMode,
                                config_.showPreeditUnderline.value());
                    composeAnchors_.erase(ic);
                    interceptor_->stopIntercepting();
                });
            }
        }

        if ((result.forwardOriginalKey || dispatch.forward_original_key) && ic)
            ic->forwardKey(keyEvent.key(), keyEvent.isRelease(), keyEvent.time());
        keyEvent.filterAndAccept();
        refreshModeAction(ic);
    }

    // ── Lifecycle ───────────────────────────────────────────────────────────

    void activate(const fcitx::InputMethodEntry& entry,
                  fcitx::InputContextEvent& event) override {
        FCITX_UNUSED(entry);
        auto* ic  = event.inputContext();
        auto& br  = bridgeFor(ic);
        br.reset();
        interceptor_->stopIntercepting();
        composeAnchors_.erase(ic);

        const bool isPwd =
            ic->capabilityFlags().test(fcitx::CapabilityFlag::Password) ||
            ic->capabilityFlags().test(fcitx::CapabilityFlag::Sensitive);
        br.setPasswordField(isPwd);

        pushPreedit(ic, "", br.config().fcitx5PreeditMode,
                    config_.showPreeditUnderline.value());

        // Populate status area. underlineAction_ is intentionally omitted
        // (it lives only in the config panel, not in the systray menu).
        auto& sa = ic->statusArea();
        sa.addAction(fcitx::StatusGroup::InputMethod, &modeAction_);
        sa.addAction(fcitx::StatusGroup::InputMethod, &methodMenuAction_);
        sa.addAction(fcitx::StatusGroup::InputMethod, &dictAction_);
        sa.addAction(fcitx::StatusGroup::InputMethod, &clipboardAction_);
        sa.addAction(fcitx::StatusGroup::InputMethod, &screenshotAction_);

        refreshModeAction(ic);
    }

    void deactivate(const fcitx::InputMethodEntry& entry,
                    fcitx::InputContextEvent& event) override {
        FCITX_UNUSED(entry);
        auto* ic = event.inputContext();
        flushAndCleanup(ic);
        pushPreedit(ic, "", farolkey::config::Fcitx5PreeditMode::Auto,
                    config_.showPreeditUnderline.value());
    }

    void reset(const fcitx::InputMethodEntry& entry,
               fcitx::InputContextEvent& event) override {
        FCITX_UNUSED(entry);
        auto* ic = event.inputContext();
        auto  it = bridges_.find(ic);
        // Clear engine state without committing.  fcitx5-gtk commits the preedit
        // to the GTK entry on focus-out before calling reset/deactivate on the
        // daemon; committing here too produces a double commit ("koko" bug).
        if (it != bridges_.end()) it->second.takeCompositionForCommit();
        composeAnchors_.erase(ic);
        interceptor_->stopIntercepting();
        pushPreedit(ic, "",
                    it != bridges_.end() ? it->second.config().fcitx5PreeditMode
                                         : farolkey::config::Fcitx5PreeditMode::Auto,
                    config_.showPreeditUnderline.value());
    }

private:
    // ── Key translation ──────────────────────────────────────────────────────

    bool translateKey(const fcitx::Key& key, farolkey::core::KeyEvent& out) {
        const auto states = key.states();
        out.ctrl  = states.test(fcitx::KeyState::Ctrl);
        out.alt   = states.test(fcitx::KeyState::Alt);
        out.shift = states.test(fcitx::KeyState::Shift);
        out.aux   = farolkey::core::KeyAux::None;
        out.key   = '\0';
        out.key_from_keypad = false;

        const fcitx::KeySym sym = key.sym();
        if (sym >= FcitxKey_KP_0 && sym <= FcitxKey_KP_9) out.key_from_keypad = true;

        switch (sym) {
            case FcitxKey_Left:      out.aux = farolkey::core::KeyAux::Left;          return true;
            case FcitxKey_Right:     out.aux = farolkey::core::KeyAux::Right;         return true;
            case FcitxKey_Up:        out.aux = farolkey::core::KeyAux::Up;            return true;
            case FcitxKey_Down:      out.aux = farolkey::core::KeyAux::Down;          return true;
            case FcitxKey_Home:      out.aux = farolkey::core::KeyAux::Home;          return true;
            case FcitxKey_End:       out.aux = farolkey::core::KeyAux::End;           return true;
            case FcitxKey_Page_Up:
            case FcitxKey_Page_Down: return false;  // not handled by the core engine
            case FcitxKey_BackSpace:
                out.key = '\b'; return true;
            case FcitxKey_Delete:
                out.aux = farolkey::core::KeyAux::DeleteForward; return true;
            case FcitxKey_Escape:    return false;  // not in KeyAux
            case FcitxKey_Tab:
                out.aux = farolkey::core::KeyAux::Tab;   return true;
            case FcitxKey_KP_Enter:
            case FcitxKey_Return:
                out.aux = farolkey::core::KeyAux::Enter; return true;
            default: break;
        }

        // For Ctrl/Alt combos: extract the character so the engine can detect
        // the toggle hotkey (isToggleHotkey checks ctrl+alt+key internally).
        const std::string utf8 = fcitx::Key::keySymToUTF8(sym);
        if (utf8.size() != 1) return false;
        out.key = static_cast<char>(static_cast<unsigned char>(utf8[0]));
        return out.key != '\0';
    }

    // ── Actions setup ────────────────────────────────────────────────────────

    void setupActions() {
        auto& ui = instance_->userInterfaceManager();

        // VI / EN mode toggle shown in systray status bar.
        modeAction_.setShortText("VI");
        modeAction_.setLongText("Vietnamese (FarolKey)");
        modeAction_.connect<fcitx::SimpleAction::Activated>([this](fcitx::InputContext* ic) {
            globalMode_ = (globalMode_ == farolkey::core::InputMode::Vietnamese)
                              ? farolkey::core::InputMode::English
                              : farolkey::core::InputMode::Vietnamese;
            // Apply to all existing bridges so every open window switches mode.
            for (auto& [_, br] : bridges_) br.setInputMode(globalMode_);
            refreshModeAction(ic);
            refreshAllStatusAreas();
        });
        ui.registerAction("farolkey-mode", &modeAction_);

        // Telex / VNI submenu — active item shown with a bullet (●) prefix.
        telexAction_.setShortText("Telex");
        telexAction_.setChecked(config_.method.value() ==
                                farolkey::adapter::fcitx5::FarolKeyMethod::Telex);
        telexAction_.connect<fcitx::SimpleAction::Activated>([this](fcitx::InputContext* ic) {
            FCITX_UNUSED(ic);
            switchMethod(farolkey::adapter::fcitx5::FarolKeyMethod::Telex);
        });
        ui.registerAction("farolkey-telex", &telexAction_);

        vniAction_.setShortText("VNI");
        vniAction_.setChecked(config_.method.value() ==
                              farolkey::adapter::fcitx5::FarolKeyMethod::VNI);
        vniAction_.connect<fcitx::SimpleAction::Activated>([this](fcitx::InputContext* ic) {
            FCITX_UNUSED(ic);
            switchMethod(farolkey::adapter::fcitx5::FarolKeyMethod::VNI);
        });
        ui.registerAction("farolkey-vni", &vniAction_);

        methodMenu_.addAction(&telexAction_);
        methodMenu_.addAction(&vniAction_);
        // Show active method in the parent action text so it's visible without opening menu.
        refreshMethodMenuLabel();
        methodMenuAction_.setMenu(&methodMenu_);
        ui.registerAction("farolkey-method-menu", &methodMenuAction_);

        // Dictionary Manager — launches farolkey-dict-gui as a detached process.
        dictAction_.setShortText("Dictionary Manager");
        dictAction_.setLongText("Open Dictionary / Abbreviation Manager");
        dictAction_.connect<fcitx::SimpleAction::Activated>([](fcitx::InputContext* ic) {
            FCITX_UNUSED(ic);
            if (const pid_t pid = fork(); pid == 0) {
                execlp("farolkey-dict-gui", "farolkey-dict-gui", nullptr);
                _exit(1);
            }
        });
        ui.registerAction("farolkey-dict", &dictAction_);

        // Clipboard History — launches farolkey-clipboard --show as a detached process.
        clipboardAction_.setShortText("Clipboard History (Ctrl + Win + V)");
        clipboardAction_.setLongText("Show Clipboard History (farolkey-clipboard)");
        clipboardAction_.connect<fcitx::SimpleAction::Activated>([](fcitx::InputContext* ic) {
            FCITX_UNUSED(ic);
            if (const pid_t pid = fork(); pid == 0) {
                execlp("farolkey-clipboard", "farolkey-clipboard", "--show", nullptr);
                _exit(1);
            }
        });
        ui.registerAction("farolkey-clipboard", &clipboardAction_);

        // Screenshot — double-fork with a short delay so the systray popup fully
        // closes before the capture starts (avoids the menu appearing in the shot).
        refreshScreenshotLabel();
        screenshotAction_.connect<fcitx::SimpleAction::Activated>([](fcitx::InputContext* ic) {
            FCITX_UNUSED(ic);
            if (const pid_t pid = fork(); pid == 0) {
                // Grandchild is re-parented to init; first child exits immediately.
                if (fork() == 0) {
                    usleep(800'000); // 800 ms — systray + any popups dismiss before capture
                    execlp("farolkey-screenshot", "farolkey-screenshot", nullptr);
                    _exit(1);
                }
                _exit(0);
            }
        });
        ui.registerAction("farolkey-screenshot", &screenshotAction_);

        // Preedit underline — config only, NOT shown in systray menu.
        underlineAction_.setShortText("Underline while composing");
        underlineAction_.setChecked(config_.showPreeditUnderline.value());
        underlineAction_.connect<fcitx::SimpleAction::Activated>([this](fcitx::InputContext* ic) {
            FCITX_UNUSED(ic);
            *config_.showPreeditUnderline.mutableValue() =
                !config_.showPreeditUnderline.value();
            underlineAction_.setChecked(config_.showPreeditUnderline.value());
            saveConfig();
        });
        ui.registerAction("farolkey-underline", &underlineAction_);
        // underlineAction_ is registered but NOT added to statusArea (config-only).

    }

    // ── Config helpers ───────────────────────────────────────────────────────

    void loadConfig() {
        fcitx::RawConfig raw;
        fcitx::readAsIni(raw, defaultConfigPath());
        if (raw.hasSubItems()) {
            config_.load(raw);
        } else {
            const auto legacy = farolkey::config::loadConfigFile(defaultConfigPath());
            *config_.method.mutableValue() =
                (legacy.method == farolkey::core::InputMethod::Telex)
                    ? farolkey::adapter::fcitx5::FarolKeyMethod::Telex
                    : farolkey::adapter::fcitx5::FarolKeyMethod::VNI;
            *config_.committedRewrite.mutableValue() = legacy.fcitx5CommittedRewrite;
        }
        // enableUserDictionary is managed exclusively by Dictionary Manager, not
        // by configtool.  Read it separately via the legacy parser so changes
        // from farolkey-dict-gui are always picked up.
        const auto legacyCfg   = farolkey::config::loadConfigFile(defaultConfigPath());
        enableUserDict_        = legacyCfg.enableUserDictionary;
        enableSmartTemplates_  = legacyCfg.enableSmartTemplates;
        try { lastConfigMtime_ = std::filesystem::last_write_time(defaultConfigPath()); } catch (...) {}
        try { lastDictMtime_   = std::filesystem::last_write_time(defaultDictPath());   } catch (...) {}
    }

    // Called at the start of every keyEvent. Detects external writes to the
    // config file (farolkey.conf) or user dictionary (user_dict.json) and reloads
    // without restart — used by farolkey-dict-gui for live updates.
    void maybeHotReloadConfig() {
        bool configChanged = false;
        bool dictChanged   = false;
        try {
            const auto m = std::filesystem::last_write_time(defaultConfigPath());
            if (m != lastConfigMtime_) { lastConfigMtime_ = m; configChanged = true; }
        } catch (...) {}
        try {
            const auto m = std::filesystem::last_write_time(defaultDictPath());
            if (m != lastDictMtime_)   { lastDictMtime_ = m;   dictChanged   = true; }
        } catch (...) {}

        if (!configChanged && !dictChanged) return;
        if (configChanged) loadConfig();
        applyConfigToAllBridges();   // reloads user dict inside new Bridge/Engine
        if (configChanged) { refreshActionStates(); refreshAllStatusAreas(); }
    }

    void saveConfig() {
        fcitx::safeSaveAsIni(config_, defaultConfigPath());
        // enableUserDictionary is not in FarolKeyConfig so safeSaveAsIni drops it.
        // Append it so loadConfigFile() always finds it after a configtool save.
        std::ofstream f(defaultConfigPath(), std::ios::app);
        f << "enable_user_dictionary="  << (enableUserDict_       ? "true" : "false") << "\n";
        f << "enable_smart_templates="  << (enableSmartTemplates_ ? "true" : "false") << "\n";
    }

    // Show active method in the parent menu label: "Input Method: Telex" or "Input Method: VNI".
    void refreshMethodMenuLabel() {
        const bool isTelex =
            config_.method.value() == farolkey::adapter::fcitx5::FarolKeyMethod::Telex;
        methodMenuAction_.setShortText(isTelex ? "Input Method: Telex" : "Input Method: VNI");
    }

    // Read hotkey from ~/.config/farolkey/screenshot.conf and update action label.
    void refreshScreenshotLabel() {
        const char* xdgCfg = getenv("XDG_CONFIG_HOME");
        std::string cfgDir = xdgCfg ? std::string(xdgCfg)
                                     : std::string(getenv("HOME")) + "/.config";
        std::ifstream f(cfgDir + "/farolkey/screenshot.conf");
        std::string hotkey = "Windows+Shift+S";
        for (std::string line; std::getline(f, line); ) {
            if (line.rfind("hotkey=", 0) == 0) {
                hotkey = line.substr(7);
                // Capitalise first letter of each segment: "super+shift+s" → "Super+Shift+S"
                bool cap = true;
                for (char& c : hotkey) {
                    if (c == '+') { cap = true; }
                    else if (cap) { c = static_cast<char>(toupper(c)); cap = false; }
                }
                // "Super" → "Windows" for user-friendly display
                for (std::string::size_type pos = 0;
                     (pos = hotkey.find("Super", pos)) != std::string::npos; ) {
                    hotkey.replace(pos, 5, "Windows");
                    pos += 7;
                }
                break;
            }
        }
        screenshotAction_.setShortText("Screenshot (" + hotkey + ")");
        screenshotAction_.setLongText("Take a screenshot with FarolKey");
    }

    void refreshActionStates() {
        const bool isTelex =
            config_.method.value() == farolkey::adapter::fcitx5::FarolKeyMethod::Telex;
        telexAction_.setChecked(isTelex);
        vniAction_.setChecked(!isTelex);
        underlineAction_.setChecked(config_.showPreeditUnderline.value());
        refreshMethodMenuLabel();
        refreshScreenshotLabel();   // pick up hotkey changes from screenshot.conf
    }

    void applyConfigToAllBridges() {
        auto rc = toRuntimeConfig(config_, enableUserDict_);
        rc.enableSmartTemplates = enableSmartTemplates_;
        for (auto& [ic, br] : bridges_) br.reloadConfig(rc);
    }

    void switchMethod(farolkey::adapter::fcitx5::FarolKeyMethod method) {
        *config_.method.mutableValue() = method;
        refreshActionStates();
        applyConfigToAllBridges();
        saveConfig();
        refreshAllStatusAreas();
    }

    void refreshModeAction(fcitx::InputContext* ic) {
        const bool vi = (globalMode_ == farolkey::core::InputMode::Vietnamese);
        modeAction_.setShortText(vi ? "VI" : "EN");
        modeAction_.setLongText(vi ? "Vietnamese (FarolKey)" : "English (FarolKey)");
        ic->updateUserInterface(fcitx::UserInterfaceComponent::StatusArea);
    }

    // Notify ALL active input contexts to repaint their status areas.
    // Called after config changes (method switch, dict toggle) so every open
    // window's indicator updates immediately, not just the one that was clicked.
    void refreshAllStatusAreas() {
        for (auto& [ic, _br] : bridges_) {
            ic->updateUserInterface(fcitx::UserInterfaceComponent::StatusArea);
        }
    }

    // ── Flush & cleanup on deactivate ────────────────────────────────────────

    void flushAndCleanup(fcitx::InputContext* ic) {
        auto iter = bridges_.find(ic);
        if (iter != bridges_.end()) {
            // Clear engine state without committing.  fcitx5-gtk (the GTK IM
            // module in the app process) already commits preedit to the GTK entry
            // on focus-out before notifying the daemon.  Committing again here
            // produces a double commit ("koko" instead of "ko").
            iter->second.takeCompositionForCommit();
            bridges_.erase(iter);
        }
        composeAnchors_.erase(ic);
        interceptor_->stopIntercepting();
    }

    // ── Bridge factory ───────────────────────────────────────────────────────

    farolkey::adapter::fcitx5::Bridge& bridgeFor(fcitx::InputContext* ic) {
        auto it = bridges_.find(ic);
        if (it != bridges_.end()) return it->second;
        auto initRc = toRuntimeConfig(config_, enableUserDict_);
        initRc.enableSmartTemplates = enableSmartTemplates_;
        const auto [created, _] = bridges_.emplace(
            ic, farolkey::adapter::fcitx5::Bridge(std::move(initRc)));
        // Apply the engine-level global mode so EN mode survives context switches.
        created->second.setInputMode(globalMode_);
        return created->second;
    }

    // ── Members ──────────────────────────────────────────────────────────────

    fcitx::Instance*  instance_;
    farolkey::adapter::fcitx5::FarolKeyConfig config_;

    // Global input mode — shared across all input contexts so toggling EN/VI
    // in one window persists when focus moves to another window.
    farolkey::core::InputMode globalMode_ = farolkey::core::InputMode::Vietnamese;

    // Managed by Dictionary Manager (not configtool); read from farolkey.conf.
    bool enableUserDict_       = true;
    bool enableSmartTemplates_ = true;
    std::filesystem::file_time_type lastConfigMtime_{};
    std::filesystem::file_time_type lastDictMtime_{};

    std::unordered_map<fcitx::InputContext*, farolkey::adapter::fcitx5::Bridge>       bridges_;
    std::unordered_map<fcitx::InputContext*, farolkey::adapter::fcitx5::ComposeAnchorSnapshot>
                                                                                    composeAnchors_;
    std::unique_ptr<farolkey::adapter::fcitx5::X11ClickInterceptor>                  interceptor_;

    // Mode icon paths (SVG files written to ~/.cache/farolkey/ at startup)
    std::string viIconPath_;
    std::string enIconPath_;

    // Actions
    fcitx::SimpleAction modeAction_;
    fcitx::SimpleAction methodMenuAction_;
    fcitx::Menu         methodMenu_;
    fcitx::SimpleAction telexAction_;
    fcitx::SimpleAction vniAction_;
    fcitx::SimpleAction dictAction_;
    fcitx::SimpleAction clipboardAction_;
    fcitx::SimpleAction screenshotAction_;
    fcitx::SimpleAction underlineAction_;
};

// ── Factory ───────────────────────────────────────────────────────────────────

class FarolKeyFcitx5Factory final : public fcitx::AddonFactory {
public:
    fcitx::AddonInstance* create(fcitx::AddonManager* manager) override {
        return new FarolKeyFcitx5Engine(manager->instance());
    }
};

}  // namespace

FCITX_ADDON_FACTORY(FarolKeyFcitx5Factory)
