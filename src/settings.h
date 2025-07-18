#pragma once
#include <atomic>
#include <string>

// 기능 활성화 여부를 저장하는 전역 변수
extern std::atomic<bool> g_feature_enabled;
extern std::string g_tone_keyword;
extern std::string g_situation_context;