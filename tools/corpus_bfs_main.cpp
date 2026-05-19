// Offline helper: find a Telex/VNI ASCII key sequence that produces a given preedit (one syllable),
// then commit with space — for corpus generation. Not used in IME hot path.

#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_set>
#include <vector>

#include "farolkey/config/config.h"
#include "farolkey/core/engine.h"
#include "farolkey/core/types.h"

namespace {

using farolkey::config::RuntimeConfig;
using farolkey::core::Engine;
using farolkey::core::InputMethod;
using farolkey::core::KeyEvent;
using farolkey::core::ProcessResult;

struct Node {
    std::vector<KeyEvent> path;
};

ProcessResult replay(Engine& eng, const std::vector<KeyEvent>& path) {
    eng.clearState();
    ProcessResult last{};
    for (const KeyEvent& ev : path) {
        last = eng.processKey(ev);
    }
    return last;
}

std::vector<char> alphabetTelexOrdered(const std::string& hintAsciiLower) {
    std::vector<char> out;
    out.reserve(32);
    std::array<bool, 256> seen{};
    for (unsigned char uc : hintAsciiLower) {
        const char lc = static_cast<char>(std::tolower(uc));
        if (lc >= 'a' && lc <= 'z' && !seen[static_cast<unsigned char>(lc)]) {
            seen[static_cast<unsigned char>(lc)] = true;
            out.push_back(lc);
        }
    }
    for (char c = 'a'; c <= 'z'; ++c) {
        if (!seen[static_cast<unsigned char>(c)]) {
            out.push_back(c);
        }
    }
    return out;
}

std::vector<char> alphabetVniOrdered(const std::string& hintAsciiLower) {
    std::vector<char> out = alphabetTelexOrdered(hintAsciiLower);
    for (char c = '0'; c <= '9'; ++c) {
        out.push_back(c);
    }
    return out;
}

std::size_t envSizeOr(const char* name, std::size_t fallback) {
    if (const char* p = std::getenv(name)) {
        char* end = nullptr;
        const unsigned long v = std::strtoul(p, &end, 10);
        if (end != p && v > 0UL && v < 500000000UL) {
            return static_cast<std::size_t>(v);
        }
    }
    return fallback;
}

int envIntOr(const char* name, int fallback) {
    if (const char* p = std::getenv(name)) {
        char* end = nullptr;
        const long v = std::strtol(p, &end, 10);
        if (end != p && v >= 4 && v <= 64) {
            return static_cast<int>(v);
        }
    }
    return fallback;
}

bool findSequence(RuntimeConfig cfg,
                  const std::string& target,
                  const std::string& keyHintAscii,
                  int maxDepth,
                  std::size_t maxNodes,
                  std::vector<KeyEvent>* outPath,
                  std::string* err) {
    Engine probe(cfg);
    std::string hint;
    for (unsigned char uc : keyHintAscii) {
        if (std::isalpha(static_cast<unsigned char>(uc)) != 0) {
            hint.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(uc))));
        }
    }
    const std::vector<char> alphabet = (cfg.method == InputMethod::Vni) ? alphabetVniOrdered(hint)
                                                                         : alphabetTelexOrdered(hint);

    std::queue<Node> q;
    std::unordered_set<std::string> seen;

    q.push(Node{});
    seen.insert("");

    std::size_t expanded = 0;
    while (!q.empty()) {
        if (expanded++ > maxNodes) {
            *err = "max_nodes exceeded";
            return false;
        }
        Node cur = std::move(q.front());
        q.pop();

        const ProcessResult last = replay(probe, cur.path);
        if (last.preedit == target) {
            *outPath = std::move(cur.path);
            return true;
        }
        if (static_cast<int>(cur.path.size()) >= maxDepth) {
            continue;
        }

        for (char c : alphabet) {
            std::vector<KeyEvent> next = cur.path;
            next.push_back(KeyEvent{.key = c});
            const std::string pd = replay(probe, next).preedit;
            // Prune overlong buffers (UTF-8 bytes); keeps BFS tractable for offline generation.
            if (pd.size() > target.size() + 14U) {
                continue;
            }
            if (seen.insert(pd).second) {
                q.push(Node{std::move(next)});
            }
        }
    }
    *err = "no path found";
    return false;
}

std::string jsonEscapeUtf8(const std::string& s) {
    std::string o;
    o.reserve(s.size() + 8);
    for (unsigned char uc : s) {
        switch (uc) {
            case '"':
                o += "\\\"";
                break;
            case '\\':
                o += "\\\\";
                break;
            case '\b':
                o += "\\b";
                break;
            case '\f':
                o += "\\f";
                break;
            case '\n':
                o += "\\n";
                break;
            case '\r':
                o += "\\r";
                break;
            case '\t':
                o += "\\t";
                break;
            default:
                if (uc < 0x20U) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", uc);
                    o += buf;
                } else {
                    o += static_cast<char>(uc);
                }
        }
    }
    return o;
}

void printJsonLine(const std::string& id,
                   const std::string& methodStr,
                   const std::string& target,
                   const std::vector<KeyEvent>& path,
                   const std::string& commitExpect) {
    (void)target;
    std::cout << "{\"corpus_schema_version\":1,\"id\":\"" << id << "\",\"tags\":[\"generated.bfs\",\"source.batch\","
              << "\"method." << methodStr << "\"],\"config\":\"" << methodStr
              << "\",\"sequence\":[";
    for (std::size_t i = 0; i < path.size(); ++i) {
        if (i > 0) {
            std::cout << ',';
        }
        const unsigned char uk = static_cast<unsigned char>(path[i].key);
        std::cout << "{\"key\":\"" << static_cast<char>(uk) << "\"}";
    }
    std::cout << ",{\"key\":\" \"}],\"expect\":{\"preedit\":\"\",\"commit\":\""
              << jsonEscapeUtf8(commitExpect) << "\"}}\n";
}

int usage() {
    std::cerr << "Usage:\n"
              << "  farolkey_corpus_bfs --one telex|vni <utf8_word> [ascii_hint]\n"
              << "  farolkey_corpus_bfs --batch  (stdin: METHOD<TAB>WORD[<TAB>ascii_hint>])\n"
              << "Env: FAROLKEY_CORPUS_BFS_MAX_DEPTH (default 26), FAROLKEY_CORPUS_BFS_MAX_NODES (default 1200000)\n";
    return 2;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        return usage();
    }
    const std::string mode = argv[1];
    if (mode == "--one") {
        if (argc != 4 && argc != 5) {
            return usage();
        }
        const std::string method = argv[2];
        const std::string word = argv[3];
        const std::string hint = (argc == 5) ? std::string(argv[4]) : std::string{};
        RuntimeConfig cfg = farolkey::config::defaultConfig();
        if (method == "vni") {
            cfg.method = InputMethod::Vni;
        } else if (method != "telex") {
            std::cerr << "method must be telex or vni\n";
            return 2;
        }

        std::vector<KeyEvent> path;
        std::string err;
        const int kMaxDepth = envIntOr("FAROLKEY_CORPUS_BFS_MAX_DEPTH", 26);
        const std::size_t kMaxNodes = envSizeOr("FAROLKEY_CORPUS_BFS_MAX_NODES", 1200000);
        if (!findSequence(cfg, word, hint, kMaxDepth, kMaxNodes, &path, &err)) {
            std::cerr << "SKIP\t" << method << '\t' << word << '\t' << err << '\n';
            return 1;
        }

        Engine verify(cfg);
        for (const KeyEvent& ev : path) {
            verify.processKey(ev);
        }
        const ProcessResult fin = verify.processKey(KeyEvent{.key = ' '});
        const std::string wantCommit = word + ' ';
        if (fin.commit != wantCommit || !fin.preedit.empty()) {
            std::cerr << "INTERNAL\tcommit mismatch want \"" << wantCommit << "\" got \"" << fin.commit << "\"\n";
            return 3;
        }
        printJsonLine("GEN-one", method, word, path, fin.commit);
        return 0;
    }

    if (mode == "--batch") {
        std::string line;
        int lineNo = 0;
        int ok = 0;
        int skip = 0;
        while (std::getline(std::cin, line)) {
            ++lineNo;
            if (line.empty()) {
                continue;
            }
            const std::string::size_type tab = line.find('\t');
            if (tab == std::string::npos) {
                std::cout << "SKIP\tline:" << lineNo << "\tno tab separator\n";
                ++skip;
                continue;
            }
            const std::string method = line.substr(0, tab);
            std::string rest = line.substr(tab + 1);
            std::string word;
            std::string hint;
            const std::string::size_type tab2 = rest.find('\t');
            if (tab2 == std::string::npos) {
                word = std::move(rest);
            } else {
                word = rest.substr(0, tab2);
                hint = rest.substr(tab2 + 1);
            }
            if (word.empty()) {
                std::cout << "SKIP\tline:" << lineNo << "\tempty word\n";
                ++skip;
                continue;
            }

            RuntimeConfig cfg = farolkey::config::defaultConfig();
            if (method == "vni") {
                cfg.method = InputMethod::Vni;
            } else if (method != "telex") {
                std::cout << "SKIP\tline:" << lineNo << "\tbad method " << method << '\n';
                ++skip;
                continue;
            }

            std::vector<KeyEvent> path;
            std::string err;
            const int kMaxDepth = envIntOr("FAROLKEY_CORPUS_BFS_MAX_DEPTH", 26);
            const std::size_t kMaxNodes = envSizeOr("FAROLKEY_CORPUS_BFS_MAX_NODES", 1200000);
            if (!findSequence(cfg, word, hint, kMaxDepth, kMaxNodes, &path, &err)) {
                std::cout << "SKIP\t" << method << '\t' << word << '\t' << err << '\n';
                ++skip;
                continue;
            }

            Engine verify(cfg);
            for (const KeyEvent& ev : path) {
                verify.processKey(ev);
            }
            const ProcessResult fin = verify.processKey(KeyEvent{.key = ' '});
            const std::string wantCommit = word + ' ';
            if (fin.commit != wantCommit) {
                std::cout << "SKIP\t" << method << '\t' << word << "\tverify_mismatch\n";
                ++skip;
                continue;
            }

            const std::string id = std::string("GEN-") + method + "-" + std::to_string(lineNo);
            printJsonLine(id, method, word, path, fin.commit);
            ++ok;
        }
        std::cerr << "[farolkey_corpus_bfs] batch ok=" << ok << " skip=" << skip << '\n';
        return 0;
    }

    return usage();
}
