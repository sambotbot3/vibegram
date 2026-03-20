#include "vibegram/llm_client.h"

#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>

using json = nlohmann::json;

namespace vibegram {
namespace {

std::filesystem::path codex_dir() {
    const char* home = std::getenv("HOME");
    if (!home || home[0] == '\0') {
        return {};
    }
    return std::filesystem::path(home) / ".codex";
}

constexpr const char* kOpenAIResponsesUrl = "https://api.openai.com/v1/responses";
constexpr const char* kAnthropicMessagesUrl = "https://api.anthropic.com/v1/messages";
constexpr const char* kDefaultOpenAIModel = "gpt-5.4";
constexpr const char* kDefaultAnthropicModel = "claude-sonnet-4-20250514";

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }

    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string unquote(std::string value) {
    value = trim(std::move(value));
    if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
        return value.substr(1, value.size() - 2);
    }
    return value;
}

std::string load_codex_model() {
    auto dir = codex_dir();
    if (dir.empty()) {
        return kDefaultOpenAIModel;
    }
    std::ifstream config_file(dir / "config.toml");
    if (!config_file.is_open()) {
        return kDefaultOpenAIModel;
    }

    std::string line;
    while (std::getline(config_file, line)) {
        const auto trimmed = trim(line);
        if (trimmed.rfind("model", 0) != 0) {
            continue;
        }

        const auto equals_pos = trimmed.find('=');
        if (equals_pos == std::string::npos) {
            continue;
        }

        return unquote(trimmed.substr(equals_pos + 1));
    }

    return kDefaultOpenAIModel;
}

bool try_load_codex_oauth(LLMConfig& config) {
    auto dir = codex_dir();
    if (dir.empty()) {
        return false;
    }
    auto auth_path = dir / "auth.json";
    if (!std::filesystem::exists(auth_path)) {
        return false;
    }

    std::ifstream auth_file(auth_path);
    if (!auth_file.is_open()) {
        return false;
    }

    json auth = json::parse(auth_file, nullptr, false);
    if (auth.is_discarded()) {
        throw std::runtime_error("Failed to parse Codex auth.json");
    }

    const auto tokens_it = auth.find("tokens");
    if (tokens_it == auth.end() || !tokens_it->is_object()) {
        return false;
    }

    const auto access_token_it = tokens_it->find("access_token");
    if (access_token_it == tokens_it->end() || !access_token_it->is_string()) {
        return false;
    }

    const std::string access_token = access_token_it->get<std::string>();
    if (access_token.empty()) {
        return false;
    }

    config.provider = Provider::OpenAI;
    config.api_url = kOpenAIResponsesUrl;
    config.api_key = access_token;
    config.model = load_codex_model();
    config.credential_source = "codex_oauth";
    return true;
}

void load_openai_api_key(LLMConfig& config) {
    if (const char* key = std::getenv("OPENAI_API_KEY")) {
        config.provider = Provider::OpenAI;
        config.api_url = kOpenAIResponsesUrl;
        config.api_key = key;
        if (config.model.empty()) {
            config.model = kDefaultOpenAIModel;
        }
        config.credential_source = "openai_api_key";
    }
}

void load_anthropic_api_key(LLMConfig& config) {
    if (const char* key = std::getenv("ANTHROPIC_API_KEY")) {
        config.provider = Provider::Anthropic;
        config.api_url = kAnthropicMessagesUrl;
        config.api_key = key;
        config.model = kDefaultAnthropicModel;
        config.credential_source = "anthropic_api_key";
    }
}

std::string extract_openai_text(const json& response) {
    const auto output_it = response.find("output");
    if (output_it == response.end() || !output_it->is_array()) {
        return "";
    }

    std::ostringstream text;
    for (const auto& item : *output_it) {
        if (!item.is_object() || item.value("type", "") != "message") {
            continue;
        }

        const auto content_it = item.find("content");
        if (content_it == item.end() || !content_it->is_array()) {
            continue;
        }

        for (const auto& content : *content_it) {
            if (!content.is_object() || content.value("type", "") != "output_text") {
                continue;
            }

            const auto text_it = content.find("text");
            if (text_it != content.end() && text_it->is_string()) {
                text << text_it->get<std::string>();
            }
        }
    }

    return text.str();
}

} // namespace

static size_t write_callback(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

LLMConfig LLMClient::resolve_config(LLMConfig config) {
    if (!config.api_key.empty()) {
        if (config.credential_source.empty()) {
            config.credential_source = "explicit_config";
        }
        return config;
    }

    if (try_load_codex_oauth(config)) {
        return config;
    }

    load_openai_api_key(config);
    if (!config.api_key.empty()) {
        return config;
    }

    load_anthropic_api_key(config);
    if (!config.api_key.empty()) {
        return config;
    }

    throw std::runtime_error(
        "No credentials found. Checked ~/.codex/auth.json, OPENAI_API_KEY, and ANTHROPIC_API_KEY");
}

LLMClient::LLMClient(LLMConfig config) : config_(resolve_config(std::move(config))) {
    spdlog::info("LLM credential source: {}", config_.credential_source);

    if (config_.credential_source != "codex_oauth") {
        throw std::runtime_error(
            "Codex OAuth not found. Refusing to start with fallback credentials for now.");
    }
}

std::string LLMClient::process_file(const std::string& filepath, const std::string& prompt) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        spdlog::error("Cannot open file: {}", filepath);
        return "";
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string contents = ss.str();

    if (contents.empty()) {
        spdlog::warn("File is empty: {}", filepath);
        return "";
    }

    std::string message = prompt + "\n\nFilename: " + filepath + "\n\n```\n" + contents + "\n```";
    return call_api(message);
}

std::string LLMClient::call_api(const std::string& user_message) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to init curl");
    }

    json body;
    if (config_.provider == Provider::OpenAI) {
        body = {
            {"model", config_.model},
            {"input", user_message},
            {"max_output_tokens", config_.max_tokens},
        };
    } else {
        body = {
            {"model", config_.model},
            {"max_tokens", config_.max_tokens},
            {"messages", json::array({
                {{"role", "user"}, {"content", user_message}}
            })}
        };
    }

    std::string request_body = body.dump();
    std::string response_data;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (config_.provider == Provider::OpenAI) {
        headers = curl_slist_append(headers, ("Authorization: Bearer " + config_.api_key).c_str());
    } else {
        headers = curl_slist_append(headers, ("x-api-key: " + config_.api_key).c_str());
        headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");
    }

    curl_easy_setopt(curl, CURLOPT_URL, config_.api_url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_data);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        spdlog::error("curl error: {}", curl_easy_strerror(res));
        return "";
    }

    auto resp = json::parse(response_data, nullptr, false);
    if (resp.is_discarded()) {
        spdlog::error("Bad API response: {}", response_data);
        return "";
    }

    if (config_.provider == Provider::OpenAI) {
        const std::string output_text = extract_openai_text(resp);
        if (output_text.empty()) {
            spdlog::error("Bad OpenAI response: {}", response_data);
        }
        return output_text;
    }

    if (!resp.contains("content")) {
        spdlog::error("Bad Anthropic response: {}", response_data);
        return "";
    }

    return resp["content"][0]["text"].get<std::string>();
}

} // namespace vibegram
