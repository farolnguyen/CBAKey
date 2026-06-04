#pragma once

#include <fcitx-config/configuration.h>
#include <fcitx-config/enum.h>
#include <fcitx-config/option.h>
#include <fcitx-utils/i18n.h>
#include <fcitx-utils/key.h>

namespace farolkey::adapter::fcitx5 {

// Input method enum — visible in fcitx5-configtool as a dropdown.
// FCITX_CONFIG_ENUM defines the enum class AND auto-generates string names.
FCITX_CONFIG_ENUM(FarolKeyMethod, Telex, VNI, VIQR, ViqrStar, SimpleTelex, SimpleTelex2, Microsoft, TocKy)

// Preedit display mode enum.
FCITX_CONFIG_ENUM(FarolKeyPreeditMode, Auto, Client, Panel)

// Config class — auto-renders as a settings panel in fcitx5-configtool when
// the user clicks the ⚙ gear icon next to FarolKey.
FCITX_CONFIGURATION(
    FarolKeyConfig,

    fcitx::Option<FarolKeyMethod> method{
        this, "Method", N_("Input Method"), FarolKeyMethod::Telex};

    fcitx::Option<fcitx::Key> toggleKey{
        this, "ToggleKey", N_("Switch EN/VI"),
        fcitx::Key("Control+Alt+z")};

    fcitx::Option<bool> showPreeditUnderline{
        this, "ShowPreeditUnderline", N_("Show underline while composing"),
        true};

    fcitx::Option<bool> autoCapitalize{
        this, "AutoCapitalize",
        N_("Auto-capitalize at sentence start (after '.', '?', '!', newline, or empty field)"),
        false};

    fcitx::Option<FarolKeyPreeditMode> preeditMode{
        this, "PreeditMode",
        N_("Preedit mode (note: Chrome, Edge, Electron always show underline in Auto/Client mode)"),
        FarolKeyPreeditMode::Auto};

    fcitx::Option<bool> committedRewrite{
        this, "CommittedRewrite",
        N_("Allow modifying committed text via surrounding-text (experimental)"),
        true};

    fcitx::ExternalOption exportLog{
        this, "ExportLog", N_("Export log bundle (for bug reports)"),
        "farolkey-export-log"};
)

}  // namespace farolkey::adapter::fcitx5
