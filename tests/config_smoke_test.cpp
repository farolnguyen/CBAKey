#include <cassert>
#include <fstream>

#include "farolkey/config/config.h"

int main() {
    assert(!farolkey::config::defaultConfig().fcitx5CommittedRewrite);

    const char* path = "farolkey_test.conf";
    {
        std::ofstream out(path);
        out << "method=vni\n";
        out << "enable_user_dictionary=false\n";
        out << "enable_static_expansion=true\n";
        out << "toggle_hotkey=Ctrl+Alt+Z\n";
        out << "fcitx5_preedit_mode=panel\n";
        out << "fcitx5_committed_rewrite=true\n";
    }

    const auto config = farolkey::config::loadConfigFile(path);
    assert(config.method == farolkey::core::InputMethod::Vni);
    assert(!config.enableUserDictionary);
    assert(config.enableStaticExpansion);
    assert(config.toggleHotkey == "Ctrl+Alt+Z");
    assert(config.fcitx5PreeditMode == farolkey::config::Fcitx5PreeditMode::Panel);
    assert(config.fcitx5CommittedRewrite);

    const auto errors = farolkey::config::validateConfig(config);
    assert(errors.empty());
    return 0;
}
