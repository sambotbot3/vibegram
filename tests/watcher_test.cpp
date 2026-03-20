#include "vibegram/watcher.h"

#include <iostream>
#include <string>

namespace {

int expect_valid_vibe_files() {
    const std::string cases[] = {
        "hello.vibe",
        "/tmp/hello.vibe",
        "relative/path/to/file.vibe",
    };

    for (const auto& path : cases) {
        if (!vibegram::Watcher::is_vibe_file(path)) {
            std::cerr << "expected vibe file to be accepted: " << path << '\n';
            return 1;
        }
    }
    return 0;
}

int expect_rejected_extensions() {
    const std::string cases[] = {
        "file.txt",
        "file.cpp",
        "hello.vibe.out",
        "noextension",
        "",
    };

    for (const auto& path : cases) {
        if (vibegram::Watcher::is_vibe_file(path)) {
            std::cerr << "expected non-vibe file to be rejected: '" << path << "'\n";
            return 1;
        }
    }
    return 0;
}

} // namespace

int main() {
    if (expect_valid_vibe_files() != 0) {
        return 1;
    }

    if (expect_rejected_extensions() != 0) {
        return 1;
    }

    return 0;
}
