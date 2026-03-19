#include "llm_client.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <fstream>
#include <sstream>
#include <stdexcept>

using json = nlohmann::json;

namespace vibegram {

static size_t write_callback(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

LLMClient::LLMClient(LLMConfig config) : config_(std::move(config)) {
    if (config_.api_key.empty()) {
        if (const char* key = std::getenv("ANTHROPIC_API_KEY")) {
            config_.api_key = key;
        } else {
            throw std::runtime_error("ANTHROPIC_API_KEY not set");
        }
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

    json body = {
        {"model", config_.model},
        {"max_tokens", config_.max_tokens},
        {"messages", json::array({
            {{"role", "user"}, {"content", user_message}}
        })}
    };

    std::string request_body = body.dump();
    std::string response_data;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("x-api-key: " + config_.api_key).c_str());
    headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");

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
    if (resp.is_discarded() || !resp.contains("content")) {
        spdlog::error("Bad API response: {}", response_data);
        return "";
    }

    return resp["content"][0]["text"].get<std::string>();
}

} // namespace vibegram
