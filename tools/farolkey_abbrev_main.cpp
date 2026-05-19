/// farolkey-abbrev — CLI quản lý từ điển / gõ tắt FarolKey (M13.4/M13.5)
///
/// Lệnh:
///   farolkey-abbrev list                              — liệt kê tất cả entries
///   farolkey-abbrev add <trigger> <expansion> [--mode vi|en|both]
///   farolkey-abbrev remove <trigger>
///   farolkey-abbrev export [<file>]                  — xuất ra file (mặc định stdout)
///   farolkey-abbrev import <file> [--merge]          — nhập từ file (mặc định replace)
///   farolkey-abbrev path                             — in đường dẫn file đang dùng

#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>

#include "farolkey/core/user_dict.h"

namespace fs = std::filesystem;

static std::string defaultDictPath() {
    const char* xdgConfig = std::getenv("XDG_CONFIG_HOME");
    const std::string base = xdgConfig && xdgConfig[0]
                                 ? xdgConfig
                                 : (std::string(std::getenv("HOME") ? std::getenv("HOME") : ".") + "/.config");
    return base + "/farolkey/user_dict.json";
}

static void printUsage() {
    std::cerr << "Cách dùng:\n"
              << "  farolkey-abbrev list\n"
              << "  farolkey-abbrev add <trigger> <expansion> [--mode vi|en|both]\n"
              << "  farolkey-abbrev remove <trigger>\n"
              << "  farolkey-abbrev export [<file>]      (mặc định: stdout)\n"
              << "  farolkey-abbrev import <file> [--merge]\n"
              << "  farolkey-abbrev path\n"
              << "\nTùy chọn môi trường:\n"
              << "  FAROLKEY_DICT_PATH=<path>  Ghi đè đường dẫn file từ điển\n";
}

static farolkey::core::AbbrevMode parseMode(const std::string& s) {
    if (s == "en")   return farolkey::core::AbbrevMode::En;
    if (s == "both") return farolkey::core::AbbrevMode::Both;
    if (s == "vi")   return farolkey::core::AbbrevMode::Vi;
    std::cerr << "Lỗi: --mode phải là vi, en, hoặc both (nhận: \"" << s << "\")\n";
    std::exit(1);
}

static const char* modeName(farolkey::core::AbbrevMode m) {
    switch (m) {
        case farolkey::core::AbbrevMode::En:   return "en";
        case farolkey::core::AbbrevMode::Both: return "both";
        default:                             return "vi";
    }
}

int main(int argc, char* argv[]) {
    const char* envPath = std::getenv("FAROLKEY_DICT_PATH");
    const std::string dictPath = envPath ? envPath : defaultDictPath();

    if (argc < 2) {
        printUsage();
        return 1;
    }

    const std::string cmd = argv[1];

    // --- path ---
    if (cmd == "path") {
        std::cout << dictPath << "\n";
        return 0;
    }

    // --- list ---
    if (cmd == "list") {
        const auto dict = farolkey::core::UserDict::loadFromFile(dictPath);
        if (dict.empty()) {
            std::cout << "(từ điển trống hoặc file chưa tồn tại: " << dictPath << ")\n";
            return 0;
        }
        std::cout << "Trigger\t\tMode\tExpansion\n";
        std::cout << "-------\t\t----\t---------\n";
        for (const std::string& key : dict.keys()) {
            const auto* e = dict.lookup(key);
            if (!e) continue;
            std::cout << e->trigger << "\t\t" << modeName(e->abbrev_mode)
                      << "\t" << e->expansion << "\n";
        }
        std::cout << "Tổng: " << dict.size() << " entries (file: " << dictPath << ")\n";
        return 0;
    }

    // --- add ---
    if (cmd == "add") {
        if (argc < 4) {
            std::cerr << "Lỗi: add cần <trigger> <expansion>\n";
            return 1;
        }
        const std::string trigger   = argv[2];
        const std::string expansion = argv[3];
        farolkey::core::AbbrevMode mode = farolkey::core::AbbrevMode::Vi;
        for (int i = 4; i < argc; ++i) {
            if (std::strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
                mode = parseMode(argv[++i]);
            }
        }
        if (trigger.empty() || expansion.empty()) {
            std::cerr << "Lỗi: trigger và expansion không được rỗng\n";
            return 1;
        }

        auto dict = farolkey::core::UserDict::loadFromFile(dictPath);

        // Warn on duplicate
        if (dict.lookup(trigger)) {
            std::cerr << "Cảnh báo: trigger \"" << trigger << "\" đã tồn tại — sẽ được cập nhật.\n";
        }

        dict.upsert({trigger, expansion, mode, false});

        // Ensure parent directory exists
        fs::create_directories(fs::path(dictPath).parent_path());
        if (!dict.saveToFile(dictPath)) {
            std::cerr << "Lỗi: không thể ghi file " << dictPath << "\n";
            return 1;
        }
        std::cout << "Đã thêm: \"" << trigger << "\" → \"" << expansion
                  << "\" [" << modeName(mode) << "]\n";
        return 0;
    }

    // --- remove ---
    if (cmd == "remove") {
        if (argc < 3) {
            std::cerr << "Lỗi: remove cần <trigger>\n";
            return 1;
        }
        const std::string trigger = argv[2];
        auto dict = farolkey::core::UserDict::loadFromFile(dictPath);
        if (!dict.remove(trigger)) {
            std::cerr << "Lỗi: trigger \"" << trigger << "\" không tìm thấy\n";
            return 1;
        }
        if (!dict.saveToFile(dictPath)) {
            std::cerr << "Lỗi: không thể ghi file " << dictPath << "\n";
            return 1;
        }
        std::cout << "Đã xóa: \"" << trigger << "\"\n";
        return 0;
    }

    // --- export ---
    if (cmd == "export") {
        const auto dict = farolkey::core::UserDict::loadFromFile(dictPath);
        const std::string outPath = (argc >= 3) ? argv[2] : "";

        auto writeDict = [&](FILE* f) {
            for (const std::string& key : dict.keys()) {
                const auto* e = dict.lookup(key);
                if (!e) continue;
                // Simple JSON escape for trigger/expansion
                auto esc = [](const std::string& s) {
                    std::string out;
                    for (char c : s) {
                        if      (c == '"')  out += "\\\"";
                        else if (c == '\\') out += "\\\\";
                        else if (c == '\n') out += "\\n";
                        else if (c == '\t') out += "\\t";
                        else                out += c;
                    }
                    return out;
                };
                std::fprintf(f,
                             "{\"trigger\":\"%s\",\"expansion\":\"%s\",\"abbrev_mode\":\"%s\"}\n",
                             esc(e->trigger).c_str(), esc(e->expansion).c_str(),
                             modeName(e->abbrev_mode));
            }
        };

        if (outPath.empty()) {
            writeDict(stdout);
        } else {
            FILE* f = std::fopen(outPath.c_str(), "w");
            if (!f) {
                std::cerr << "Lỗi: không thể mở " << outPath << " để ghi\n";
                return 1;
            }
            writeDict(f);
            std::fclose(f);
            std::cout << "Đã export " << dict.size() << " entries → " << outPath << "\n";
        }
        return 0;
    }

    // --- import ---
    if (cmd == "import") {
        if (argc < 3) {
            std::cerr << "Lỗi: import cần <file>\n";
            return 1;
        }
        const std::string importPath = argv[2];
        bool merge = false;
        for (int i = 3; i < argc; ++i) {
            if (std::strcmp(argv[i], "--merge") == 0) merge = true;
        }

        const auto incoming = farolkey::core::UserDict::loadFromFile(importPath);
        if (incoming.empty()) {
            std::cerr << "Cảnh báo: file import trống hoặc không đọc được: " << importPath << "\n";
            return 1;
        }

        auto dict = merge ? farolkey::core::UserDict::loadFromFile(dictPath)
                          : farolkey::core::UserDict{};

        int added = 0, updated = 0, skipped = 0;
        for (const std::string& key : incoming.keys()) {
            const auto* e = incoming.lookup(key);
            if (!e) continue;
            if (merge && dict.lookup(key)) {
                // In merge mode: warn on conflict, skip (user can use add to override)
                std::cerr << "Cảnh báo: trigger \"" << key << "\" đã tồn tại, bỏ qua (dùng 'add' để ghi đè).\n";
                ++skipped;
            } else {
                const bool replaced = dict.upsert(*e);
                replaced ? ++updated : ++added;
            }
        }

        fs::create_directories(fs::path(dictPath).parent_path());
        if (!dict.saveToFile(dictPath)) {
            std::cerr << "Lỗi: không thể ghi file " << dictPath << "\n";
            return 1;
        }
        std::cout << "Import xong: +" << added << " mới, ~" << updated
                  << " cập nhật, " << skipped << " bỏ qua. File: " << dictPath << "\n";
        return 0;
    }

    std::cerr << "Lệnh không hợp lệ: " << cmd << "\n\n";
    printUsage();
    return 1;
}
