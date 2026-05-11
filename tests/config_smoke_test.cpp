#include <cassert>
#include <fstream>

#include "cbakey/config/config.h"

int main() {
    const char* path = "cbakey_test.conf";
    {
        std::ofstream out(path);
        out << "method=vni\n";
        out << "enable_user_dictionary=false\n";
        out << "enable_static_expansion=true\n";
        out << "toggle_hotkey=Ctrl+Alt+Z\n";
    }

    const auto config = cbakey::config::loadConfigFile(path);
    assert(config.method == cbakey::core::InputMethod::Vni);
    assert(!config.enableUserDictionary);
    assert(config.enableStaticExpansion);
    assert(config.toggleHotkey == "Ctrl+Alt+Z");

    const auto errors = cbakey::config::validateConfig(config);
    assert(errors.empty());
    return 0;
}
