#include "watcher.h"
#include <spdlog/spdlog.h>
#include <filesystem>

namespace vibegram {

Watcher::Listener::Listener(FileCallback cb) : callback_(std::move(cb)) {}

void Watcher::Listener::handleFileAction(efsw::WatchID /*watchid*/,
                                          const std::string& dir,
                                          const std::string& filename,
                                          efsw::Action action,
                                          std::string /*oldFilename*/) {
    if (action == efsw::Actions::Add || action == efsw::Actions::Modified) {
        auto fullpath = (std::filesystem::path(dir) / filename).string();
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
    spdlog::info("Watching directory: {}", directory_);
}

void Watcher::stop() {
    fw_.removeWatch(watch_id_);
}

} // namespace vibegram
