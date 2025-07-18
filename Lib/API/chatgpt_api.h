// chatgpt_api.h
#pragma once
#include <string>

// prompt만 넘기면 .env에서 API 키를 읽어 답변 반환
std::string ask_chatgpt(const std::string& prompt, const std::string& extra = "");