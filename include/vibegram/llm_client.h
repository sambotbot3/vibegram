#pragma once

#include <string>

namespace vibegram {

enum class Provider {
    OpenAI,
    Anthropic,
};

struct LLMConfig {
    std::string api_url = "https://api.openai.com/v1/responses";
    std::string api_key;
    std::string model = "gpt-5.4";
    int max_tokens = 1024;
    Provider provider = Provider::OpenAI;
    std::string credential_source;
};

class LLMClient {
public:
    explicit LLMClient(LLMConfig config);

    // Send file contents to the LLM with a prompt, return the response text.
    std::string process_file(const std::string& filepath,
                             const std::string& prompt = "Describe this file's contents concisely.");

private:
    static LLMConfig resolve_config(LLMConfig config);
    std::string call_api(const std::string& user_message);
    LLMConfig config_;
};

} // namespace vibegram
