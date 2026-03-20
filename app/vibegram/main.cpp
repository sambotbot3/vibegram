#include "vibegram/llm_client.h"
#include "vibegram/watcher.h"

#include <spdlog/spdlog.h>

#include <atomic>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

static std::atomic<bool> running{true};

static std::string load_prompt() {
    // Search upward from the executable or cwd for prompts/transpile.md
    for (auto dir = std::filesystem::current_path(); dir.has_parent_path(); dir = dir.parent_path()) {
        auto path = dir / "prompts" / "transpile.md";
        if (std::filesystem::exists(path)) {
            std::ifstream f(path);
            if (f.is_open()) {
                std::ostringstream ss;
                ss << f.rdbuf();
                return ss.str();
            }
        }
        if (dir == dir.parent_path()) break;
    }
    return "Transpile this plain-English description into clean, decoupled code.";
}

static void signal_handler(int) {
    running = false;
}

int main(int argc, char* argv[]) {
    std::string watch_dir =
        (argc > 1) ? argv[1] : std::filesystem::current_path().string();

    spdlog::set_level(spdlog::level::info);
    spdlog::info("vibegram starting - watching: {}", watch_dir);

    vibegram::LLMClient llm(vibegram::LLMConfig{});

    std::string prompt = load_prompt();
    spdlog::info("Loaded transpile prompt ({} chars)", prompt.size());

    vibegram::Watcher watcher(watch_dir, [&llm, &prompt](const std::string& filepath) {
        spdlog::info("Processing: {}", filepath);
        std::string result = llm.process_file(filepath, prompt);
        if (!result.empty()) {
            std::cout << "\n--- LLM response for " << filepath << " ---\n"
                      << result << "\n---\n" << std::endl;

            std::string out_path = filepath + ".out";
            std::ofstream out_file(out_path);
            if (out_file.is_open()) {
                out_file << result;
                spdlog::info("Wrote output to {}", out_path);
            } else {
                spdlog::error("Failed to write output to {}", out_path);
            }
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
