#include <cassert>
#include <fstream>

#include "cbakey/config/config.h"

int main() {
    assert(!cbakey::config::defaultConfig().fcitx5CommittedRewrite);

    const char* path = "cbakey_test.conf";
    {
        std::ofstream out(path);
        out << "method=vni\n";
        out << "enable_user_dictionary=false\n";
        out << "enable_static_expansion=true\n";
        out << "toggle_hotkey=Ctrl+Alt+Z\n";
        out << "fcitx5_preedit_mode=panel\n";
        out << "fcitx5_committed_rewrite=true\n";
    }

    const auto config = cbakey::config::loadConfigFile(path);
    assert(config.method == cbakey::core::InputMethod::Vni);
    assert(!config.enableUserDictionary);
    assert(config.enableStaticExpansion);
    assert(config.toggleHotkey == "Ctrl+Alt+Z");
    assert(config.fcitx5PreeditMode == cbakey::config::Fcitx5PreeditMode::Panel);
    assert(config.fcitx5CommittedRewrite);

    const auto errors = cbakey::config::validateConfig(config);
    assert(errors.empty());
    return 0;
}
