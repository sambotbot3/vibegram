#include "vibegram/llm_client.h"
#include "vibegram/watcher.h"

#include <iostream>
#include <stdexcept>
#include <string>

namespace {

int expect_fallback_credentials_rejected() {
    vibegram::LLMConfig config;
    config.api_key = "test-key";
    config.credential_source = "openai_api_key";

    try {
        vibegram::LLMClient client(config);
        (void)client;
    } catch (const std::runtime_error& error) {
        if (std::string(error.what()).find("Refusing to start with fallback credentials") != std::string::npos) {
            return 0;
        }
        std::cerr << "unexpected error: " << error.what() << '\n';
        return 1;
    }

    std::cerr << "expected fallback credentials to be rejected\n";
    return 1;
}

int expect_codex_source_can_construct() {
    vibegram::LLMConfig config;
    config.api_key = "test-key";
    config.credential_source = "codex_oauth";
    config.model = "gpt-5.4";

    vibegram::LLMClient client(config);
    const std::string result = client.process_file("/tmp/vibegram_missing_test_file.txt");
    if (!result.empty()) {
        std::cerr << "expected empty result for missing file\n";
        return 1;
    }

    return 0;
}

int expect_vibe_extension_filtering() {
    if (!vibegram::Watcher::is_vibe_file("/tmp/test.vibe")) {
        std::cerr << "expected .vibe file to be accepted\n";
        return 1;
    }

    if (vibegram::Watcher::is_vibe_file("/tmp/test.txt")) {
        std::cerr << "expected non-.vibe file to be rejected\n";
        return 1;
    }

    return 0;
}

} // namespace

int main() {
    if (expect_fallback_credentials_rejected() != 0) {
        return 1;
    }

    if (expect_codex_source_can_construct() != 0) {
        return 1;
    }

    if (expect_vibe_extension_filtering() != 0) {
        return 1;
    }

    return 0;
}
