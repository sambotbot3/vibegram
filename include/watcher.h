#pragma once

#include <efsw/efsw.hpp>
#include <functional>
#include <string>

namespace vibegram {

using FileCallback = std::function<void(const std::string& filepath)>;

class Watcher {
public:
    explicit Watcher(const std::string& directory, FileCallback on_file_ready);
    ~Watcher();

    void start();
    void stop();

private:
    class Listener : public efsw::FileWatchListener {
    public:
        explicit Listener(FileCallback cb);
        void handleFileAction(efsw::WatchID watchid, const std::string& dir,
                              const std::string& filename, efsw::Action action,
                              std::string oldFilename) override;
    private:
        FileCallback callback_;
    };

    std::string directory_;
    efsw::FileWatcher fw_;
    Listener listener_;
    efsw::WatchID watch_id_{};
};

} // namespace vibegram
