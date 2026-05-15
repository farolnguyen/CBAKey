#pragma once

#include <fcitx-config/configuration.h>
#include <fcitx-config/enum.h>
#include <fcitx-config/option.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/key.h>

namespace cbakey::adapter::fcitx5 {

// Input method enum — visible in fcitx5-configtool as a dropdown.
// FCITX_CONFIG_ENUM defines the enum class AND auto-generates string names.
FCITX_CONFIG_ENUM(CBAKeyMethod, Telex, VNI)

// Config class — auto-renders as a settings panel in fcitx5-configtool when
// the user clicks the ⚙ gear icon next to CBAKey.
FCITX_CONFIGURATION(
    CBAKeyConfig,

    fcitx::Option<CBAKeyMethod> method{
        this, "Method", N_("Input Method"), CBAKeyMethod::Telex};

    fcitx::Option<fcitx::Key> toggleKey{
        this, "ToggleKey", N_("Switch EN/VI"),
        fcitx::Key("Control+Alt+z")};

    fcitx::Option<bool> enableUserDictionary{
        this, "EnableUserDictionary", N_("Enable abbreviations / user dictionary"),
        true};

    fcitx::Option<bool> showPreeditUnderline{
        this, "ShowPreeditUnderline", N_("Show underline while composing"),
        true};

    fcitx::Option<bool> committedRewrite{
        this, "CommittedRewrite",
        N_("Allow modifying committed text via surrounding-text (experimental)"),
        true};
)

}  // namespace cbakey::adapter::fcitx5
