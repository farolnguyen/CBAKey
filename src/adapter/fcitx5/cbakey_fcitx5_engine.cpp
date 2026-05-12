#include <cstdlib>
#include <string>
#include <unordered_map>

#include <fcitx/addonfactory.h>
#include <fcitx/addoninstance.h>
#include <fcitx/addonmanager.h>
#include <fcitx/event.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/inputpanel.h>
#include <fcitx/text.h>
#include <fcitx-utils/key.h>
#include <fcitx-utils/keysym.h>
#include <fcitx-utils/capabilityflags.h>
#include <fcitx-utils/textformatflags.h>

#include "cbakey/adapter/fcitx5/bridge.h"
#include "cbakey/adapter/fcitx5/preedit_strategy.h"

namespace {

std::string defaultConfigPath() {
    const char* home = std::getenv("HOME");
    if (!home) {
        return "cbakey.conf";
    }
    return std::string(home) + "/.config/cbakey/cbakey.conf";
}

bool translateKeyEvent(const fcitx::Key& key, cbakey::core::KeyEvent& out) {
    const auto states = key.states();
    out.ctrl = states.test(fcitx::KeyState::Ctrl);
    out.alt = states.test(fcitx::KeyState::Alt);
    out.shift = states.test(fcitx::KeyState::Shift);
    out.aux = cbakey::core::KeyAux::None;
    out.key = '\0';

    switch (key.sym()) {
        case FcitxKey_Left:
            out.aux = cbakey::core::KeyAux::Left;
            return true;
        case FcitxKey_Right:
            out.aux = cbakey::core::KeyAux::Right;
            return true;
        case FcitxKey_Home:
            out.aux = cbakey::core::KeyAux::Home;
            return true;
        case FcitxKey_End:
            out.aux = cbakey::core::KeyAux::End;
            return true;
        case FcitxKey_Delete:
            out.aux = cbakey::core::KeyAux::DeleteForward;
            return true;
        case FcitxKey_Return:
        case FcitxKey_KP_Enter:
            out.aux = cbakey::core::KeyAux::Enter;
            return true;
        case FcitxKey_Tab:
        case FcitxKey_ISO_Left_Tab:
            out.aux = cbakey::core::KeyAux::Tab;
            return true;
        default:
            break;
    }

    if (key.sym() == FcitxKey_BackSpace) {
        out.key = '\b';
        return true;
    }
    if (key.sym() == FcitxKey_space) {
        out.key = ' ';
        return true;
    }

    const std::string utf8 = fcitx::Key::keySymToUTF8(key.sym());
    if (utf8.size() == 1) {
        out.key = utf8[0];
        return true;
    }
    return false;
}

cbakey::adapter::fcitx5::PreeditCapabilitySnapshot snapshotPreeditCapabilities(
    fcitx::InputContext* inputContext) {
    if (!inputContext) {
        return {};
    }
    return cbakey::adapter::fcitx5::PreeditCapabilitySnapshot{
        .supports_client_preedit =
            inputContext->capabilityFlags().test(fcitx::CapabilityFlag::Preedit),
    };
}

void pushPreeditToInputContext(fcitx::InputContext* inputContext,
                               const std::string& preedit,
                               cbakey::config::Fcitx5PreeditMode mode) {
    if (!inputContext) {
        return;
    }
    auto& panel = inputContext->inputPanel();
    panel.setClientPreedit(fcitx::Text());
    panel.setPreedit(fcitx::Text());
    if (!preedit.empty()) {
        const auto capabilities = snapshotPreeditCapabilities(inputContext);
        fcitx::Text text(preedit, fcitx::TextFormatFlag::NoFlag);
        text.setCursor(static_cast<int>(preedit.size()));
        if (cbakey::adapter::fcitx5::choosePreeditPresentation(mode, capabilities) ==
            cbakey::adapter::fcitx5::PreeditPresentation::Client) {
            panel.setClientPreedit(text);
        } else {
            panel.setPreedit(text);
        }
    }
    inputContext->updatePreedit();
    inputContext->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
}

class CBAKeyFcitx5Engine final : public fcitx::InputMethodEngine {
public:
    CBAKeyFcitx5Engine() = default;

    std::vector<fcitx::InputMethodEntry> listInputMethods() override {
        std::vector<fcitx::InputMethodEntry> entries;
        entries.emplace_back("cbakey", "CBAKey", "vi", "cbakey");
        entries.back().setLabel("VI").setConfigurable(true);
        return entries;
    }

    void keyEvent(const fcitx::InputMethodEntry& entry,
                  fcitx::KeyEvent& keyEvent) override {
        FCITX_UNUSED(entry);
        if (keyEvent.isRelease()) {
            return;
        }

        cbakey::core::KeyEvent engineEvent;
        if (!translateKeyEvent(keyEvent.key(), engineEvent)) {
            return;
        }

        auto* inputContext = keyEvent.inputContext();
        auto& bridge = bridgeFor(inputContext);
        const auto presentation = cbakey::adapter::fcitx5::choosePreeditPresentation(
            bridge.config().fcitx5PreeditMode, snapshotPreeditCapabilities(inputContext));
        const auto result = bridge.handleKey(engineEvent);
        if (!result.consumed) {
            return;
        }

        const auto dispatch =
            cbakey::adapter::fcitx5::adjustCommitForPresentation(presentation, engineEvent.aux, result.commit);
        if (!dispatch.commit.empty() && inputContext) {
            inputContext->commitString(dispatch.commit);
        }
        pushPreeditToInputContext(inputContext, bridge.preedit(), bridge.config().fcitx5PreeditMode);
        if ((result.forwardOriginalKey || dispatch.forward_original_key) && inputContext) {
            inputContext->forwardKey(keyEvent.key(), keyEvent.isRelease(), keyEvent.time());
        }
        keyEvent.filterAndAccept();
    }

    void activate(const fcitx::InputMethodEntry& entry,
                  fcitx::InputContextEvent& event) override {
        FCITX_UNUSED(entry);
        auto* inputContext = event.inputContext();
        auto& bridge = bridgeFor(inputContext);
        bridge.reset();
        pushPreeditToInputContext(inputContext, "", bridge.config().fcitx5PreeditMode);
    }

    void deactivate(const fcitx::InputMethodEntry& entry,
                    fcitx::InputContextEvent& event) override {
        FCITX_UNUSED(entry);
        auto* inputContext = event.inputContext();
        if (inputContext) {
            auto iter = bridges_.find(inputContext);
            if (iter != bridges_.end()) {
                const std::string pending = iter->second.takeCompositionForCommit();
                if (!pending.empty()) {
                    inputContext->commitString(pending);
                }
                bridges_.erase(iter);
            }
            pushPreeditToInputContext(inputContext, "", cbakey::config::Fcitx5PreeditMode::Auto);
        }
    }

    void reset(const fcitx::InputMethodEntry& entry,
               fcitx::InputContextEvent& event) override {
        FCITX_UNUSED(entry);
        auto* inputContext = event.inputContext();
        auto iter = bridges_.find(inputContext);
        if (iter != bridges_.end()) {
            const std::string pending = iter->second.takeCompositionForCommit();
            if (!pending.empty() && inputContext) {
                inputContext->commitString(pending);
            }
        }
        pushPreeditToInputContext(
            inputContext, "",
            iter != bridges_.end() ? iter->second.config().fcitx5PreeditMode
                                   : cbakey::config::Fcitx5PreeditMode::Auto);
    }

private:
    cbakey::adapter::fcitx5::Bridge& bridgeFor(fcitx::InputContext* inputContext) {
        auto iter = bridges_.find(inputContext);
        if (iter != bridges_.end()) {
            return iter->second;
        }
        const auto [created, _] = bridges_.emplace(
            inputContext, cbakey::adapter::fcitx5::createBridgeFromConfigFile(defaultConfigPath()));
        return created->second;
    }

    std::unordered_map<fcitx::InputContext*, cbakey::adapter::fcitx5::Bridge> bridges_;
};

class CBAKeyFcitx5Factory final : public fcitx::AddonFactory {
public:
    fcitx::AddonInstance* create(fcitx::AddonManager* manager) override {
        FCITX_UNUSED(manager);
        return new CBAKeyFcitx5Engine();
    }
};

}  // namespace

FCITX_ADDON_FACTORY(CBAKeyFcitx5Factory)
