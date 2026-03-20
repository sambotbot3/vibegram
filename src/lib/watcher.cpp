#include "vibegram/watcher.h"

#include <filesystem>

#include <spdlog/spdlog.h>

namespace vibegram {

bool Watcher::is_vibe_file(const std::string& filepath) {
    return std::filesystem::path(filepath).extension() == ".vibe";
}

Watcher::Listener::Listener(FileCallback cb) : callback_(std::move(cb)) {}

void Watcher::Listener::handleFileAction(efsw::WatchID /*watchid*/,
                                          const std::string& dir,
                                          const std::string& filename,
                                          efsw::Action action,
                                          std::string /*oldFilename*/) {
    if (action == efsw::Actions::Add || action == efsw::Actions::Modified) {
        auto fullpath = (std::filesystem::path(dir) / filename).string();
        if (!Watcher::is_vibe_file(fullpath)) {
            spdlog::debug("Ignoring non-.vibe file: {}", fullpath);
            return;
        }
        spdlog::info("File {}: {}", action == efsw::Actions::Add ? "added" : "modified", fullpath);
        callback_(fullpath);
    }
}

Watcher::Watcher(const std::string& directory, FileCallback on_file_ready)
    : directory_(directory), listener_(std::move(on_file_ready)) {}

Watcher::~Watcher() { stop(); }

void Watcher::start() {
    watch_id_ = fw_.addWatch(directory_, &listener_, false);
    fw_.watch();
    spdlog::info("Watching directory for .vibe files: {}", directory_);
}

void Watcher::stop() {
    fw_.removeWatch(watch_id_);
}

} // namespace vibegram
