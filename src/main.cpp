#include "watcher.h"
#include "llm_client.h"
#include <spdlog/spdlog.h>
#include <csignal>
#include <atomic>
#include <thread>
#include <iostream>

static std::atomic<bool> running{true};

static void signal_handler(int) {
    running = false;
}

int main(int argc, char* argv[]) {
    std::string watch_dir = (argc > 1) ? argv[1] : "./watch_dir";

    spdlog::set_level(spdlog::level::info);
    spdlog::info("vibegram starting — watching: {}", watch_dir);

    vibegram::LLMClient llm(vibegram::LLMConfig{});

    vibegram::Watcher watcher(watch_dir, [&llm](const std::string& filepath) {
        spdlog::info("Processing: {}", filepath);
        std::string result = llm.process_file(filepath);
        if (!result.empty()) {
            std::cout << "\n--- LLM response for " << filepath << " ---\n"
                      << result << "\n---\n" << std::endl;
        }
    });

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    watcher.start();

    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    spdlog::info("Shutting down.");
    return 0;
}
