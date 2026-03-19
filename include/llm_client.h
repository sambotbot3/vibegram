#pragma once

#include <string>

namespace vibegram {

struct LLMConfig {
    std::string api_url = "https://api.anthropic.com/v1/messages";
    std::string api_key;       // set via ANTHROPIC_API_KEY env var
    std::string model = "claude-sonnet-4-20250514";
    int max_tokens = 1024;
};

class LLMClient {
public:
    explicit LLMClient(LLMConfig config);

    // Send file contents to the LLM with a prompt, return the response text.
    std::string process_file(const std::string& filepath,
                             const std::string& prompt = "Describe this file's contents concisely.");

private:
    std::string call_api(const std::string& user_message);
    LLMConfig config_;
};

} // namespace vibegram
