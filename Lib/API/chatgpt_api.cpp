#include "chatgpt_api.h"
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include "json.hpp"
using json = nlohmann::json;
#include <cstdlib>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string ask_chatgpt(const std::string& prompt, const std::string& extra) {
    std::string api_key;
    // 1. 환경변수에서 OPENAI_KEY를 우선 시도
    const char* env_key = std::getenv("OPENAI_KEY");
    if (env_key) {
        api_key = env_key;
    } else {
        // 2. .env 파일에서 직접 읽기
        std::ifstream env_file(".env");
        if (env_file.is_open()) {
            std::string line;
            while (std::getline(env_file, line)) {
                if (line.find("OPENAI_KEY=") == 0) {
                    api_key = line.substr(std::string("OPENAI_KEY=").length());
                    break;
                }
            }
            env_file.close();
        }
    }
    if (api_key.empty()) {
        return "OPENAI_KEY를 환경변수나 .env 파일에서 찾을 수 없습니다.";
    }

    CURL* curl = curl_easy_init();
    if (!curl) return "CURL 초기화 실패";

    json messages = json::array();
    if (!extra.empty()) {
        messages.push_back({{"role", "user"}, {"content", extra}});
    }
    messages.push_back({{"role", "user"}, {"content", prompt}});
    json req = {
        {"model", "gpt-4o"},
        {"messages", messages}
    };

    std::string readBuffer;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string bearer = "Authorization: Bearer " + api_key;
    headers = curl_slist_append(headers, bearer.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/chat/completions");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    const std::string body = req.dump();
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, body.size());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    CURLcode res = curl_easy_perform(curl);
    std::string result;
    if (res == CURLE_OK) {
        try {
            json jres = json::parse(readBuffer);
            result = jres["choices"][0]["message"]["content"];
        } catch (...) {
            result = "응답 파싱 오류: " + readBuffer;
        }
    } else {
        result = "CURL 오류: " + std::string(curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    return result;
}