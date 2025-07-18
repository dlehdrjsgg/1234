#include "install_event_tap.h"
#include "settings.h"
#include "json.hpp"
#include <Carbon/Carbon.h> // 전역 단축키를 위해 추가
#include <CoreFoundation/CoreFoundation.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <cstdlib>

// 전역 변수 정의
std::atomic<bool> g_feature_enabled = true;
std::atomic<bool> g_hotkey_enabled = false;
std::string g_tone_keyword = "비지니스";
std::string g_situation_context = "";

// install_event_tap.cpp에 정의된 전역 이벤트 탭에 대한 참조
extern CFMachPortRef gEventTap;

// osascript를 실행하고 결과를 문자열로 반환하는 헬퍼 함수
std::string run_osascript(const std::string& script) {
    FILE* pipe = popen(script.c_str(), "r");
    if (!pipe) return "";
    std::string result;
    char buf[256];
    while (fgets(buf, sizeof(buf), pipe)) {
        result += buf;
    }
    pclose(pipe);
    // osascript 결과에 포함된 마지막 개행 문자를 제거합니다.
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return result;
}

// 알림을 띄우는 함수
void show_notification(const std::string& title, const std::string& subtitle) {
    // 문자열 내의 따옴표를 이스케이프 처리합니다.
    std::string safe_subtitle = subtitle;
    // ... 더 정교한 이스케이프 로직이 필요할 수 있습니다.
    std::string command = "osascript -e 'display notification \"" + safe_subtitle + "\" with title \"" + title + "\"'";
    system(command.c_str());
}

// 설정 파일 경로를 가져오는 함수
std::filesystem::path get_config_path() {
    const char* home_dir = getenv("HOME");
    if (home_dir == nullptr) {
        // HOME 환경 변수가 없는 예외적인 경우, 현재 디렉토리에 저장
        return std::filesystem::path("./tuney_settings.json");
    }
    return std::filesystem::path(home_dir) / ".config" / "tuney" / "settings.json";
}

// 설정을 파일에 저장하는 함수
void save_settings() {
    auto config_path = get_config_path();
    try {
        std::filesystem::create_directories(config_path.parent_path());
        std::ofstream o(config_path);
        nlohmann::json settings = {
            {"feature_enabled", g_feature_enabled.load()},
            {"hotkey_enabled", g_hotkey_enabled.load()},
            {"tone_keyword", g_tone_keyword},
            {"situation_context", g_situation_context}
        };
        o << settings.dump(4);
    } catch (const std::exception& e) {
        std::cerr << "Failed to write config file: " << e.what() << std::endl;
    }
}
// --- 전역 단축키 관련 함수 ---
EventHotKeyRef g_hotkey_ref;

OSStatus hotkey_handler(EventHandlerCallRef next_handler, EventRef event, void* user_data) {
    (void)next_handler; (void)event; (void)user_data; // 미사용 인자

    // 기능 활성화 상태를 토글합니다.
    g_feature_enabled = !g_feature_enabled.load();

    // 이벤트 탭의 활성화 상태를 직접 제어합니다.
    if (gEventTap) {
        CGEventTapEnable(gEventTap, g_feature_enabled.load());
    }

    // 변경된 상태를 파일에 저장하여 다음 실행 시에도 유지되도록 합니다.
    save_settings();

    // 사용자에게 상태 변경을 알립니다.
    if (g_feature_enabled) {
        show_notification("Tuney", "입력 모니터링이 시작되었습니다.");
    } else {
        show_notification("Tuney", "입력 모니터링이 중지되었습니다.");
    }

    return noErr;
}

void register_hotkey() {
    EventTypeSpec event_type;
    event_type.eventClass = kEventClassKeyboard;
    event_type.eventKind = kEventHotKeyPressed;

    // 단축키 핸들러를 설치합니다.
    InstallApplicationEventHandler(&hotkey_handler, 1, &event_type, NULL, NULL);

    // Control + T 단축키를 시스템에 등록합니다.
    EventHotKeyID hotkey_id;
    hotkey_id.signature = 'htk1';
    hotkey_id.id = 1;
    RegisterEventHotKey(kVK_ANSI_T, controlKey, hotkey_id, GetApplicationEventTarget(), 0, &g_hotkey_ref);
}
// --------------------------

// 앱 최초 실행 시 설정을 위한 대화상자를 띄우는 함수
void setup_first_run(const std::filesystem::path& config_path) {
    std::cout << "[DEBUG] setup_first_run() called, config path: " << config_path << std::endl;
    if (std::filesystem::exists(config_path)) {
        std::filesystem::remove(config_path);
    }
    // --- 1. 톤 설정 ---
    std::string tone_script =
        "osascript -e 'try\n"
        "set TONE to choose from list {\"비지니스\", \"친구\", \"캐주얼\", \"공손하게\"} "
        "with prompt \"대화의 톤을 설정하세요:\" with title \"Tuney 설정 - 1/3단계\" default items {\"비지니스\"}\n"
        "if TONE is not false then\n"
        "    return item 1 of TONE\n"
        "else\n"
        "    return \"\"\n"
        "end if\n"
        "on error\n"
        "    return \"\"\n"
        "end try'";
    std::string tone_result = run_osascript(tone_script);
    std::cout << "[DEBUG] Tone result: " << tone_result << std::endl;
    g_tone_keyword = tone_result.empty() ? "비지니스" : tone_result; // 사용자가 취소하면 기본값 사용

    // --- 2. 상황 설명 ---
    std::string context_script = "osascript -e 'try' -e 'text returned of (display dialog \"현재 대화 상황을 간단히 설명해주세요.\\n(예: 동료에게 보내는 업무 이메일)\" with title \"Tuney 설정 - 2/3단계\" default answer \"\")' -e 'on error' -e 'return \"\"' -e 'end try'";
    g_situation_context = run_osascript(context_script);
    std::cout << "[DEBUG] Context result: " << g_situation_context << std::endl;

    // --- 3. 단축키 설정 ---
    bool hotkey_enabled_local = false;  // 단축키는 사용자가 선택해야 활성화
    const char* hotkey_script =
        "osascript -e 'display dialog \"Control+T 단축키로 문장 교정 기능을 켜고 끄는 옵션을 사용하시겠습니까?\" "
                     "with title \"Tuney 설정 - 3/3단계\" "
                     "buttons {\"아니요\",\"예\"} default button \"예\"'";
    std::string hotkey_result = run_osascript(hotkey_script);
    std::cout << "[DEBUG] Hotkey result: " << hotkey_result << std::endl;
    // `display dialog`는 버튼 이름을 반환합니다.
    if (hotkey_result.find("예") != std::string::npos) {
        hotkey_enabled_local = true;
    }

    // 전역 변수 설정 (최초 실행 시 기능은 항상 켜짐으로 시작)
    g_feature_enabled = true;
    g_hotkey_enabled = hotkey_enabled_local;

    save_settings();
    std::cout << "Settings file created at: " << config_path << std::endl;
}

void load_settings(const std::filesystem::path& config_path) {
    try {
        std::ifstream i(config_path);
        if (i.is_open()) {
            nlohmann::json settings;
            i >> settings;
            g_feature_enabled = settings.value("feature_enabled", true);
            g_hotkey_enabled = settings.value("hotkey_enabled", false);
            g_tone_keyword = settings.value("tone_keyword", "비지니스");
            g_situation_context = settings.value("situation_context", "");
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to read config file, using defaults: " << e.what() << std::endl;
    }
}

int main() {
    auto config_path = get_config_path();
    // 매번 실행할 때마다 설정 대화상자 실행
    setup_first_run(config_path);

    // Lib/hook/install_event_tap 모듈에서 이벤트 탭을 설치합니다.
    installEventTap();

    // 로드된 설정에 따라 이벤트 탭의 초기 상태를 설정합니다.
    if (gEventTap) CGEventTapEnable(gEventTap, g_feature_enabled.load());

    // 프로세스 타입을 백그라운드 앱으로 변경하여 시스템 이벤트를 안정적으로 수신합니다.
    // 이 작업을 통해 Dock에 아이콘이 표시되지 않으면서도 전역 단축키가 올바르게 동작합니다.
    ProcessSerialNumber psn;
    if (GetCurrentProcess(&psn) == noErr)
    {
        TransformProcessType(&psn, kProcessTransformToUIElementApplication);
    }

    if (g_hotkey_enabled) {
        register_hotkey();
        std::cout << "Control+T hotkey for toggling is enabled." << std::endl;
    }

    if (g_feature_enabled.load()) {
        std::cout << "TUNEY is active. Monitoring keyboard input." << std::endl;
    } else {
        std::cout << "TUNEY is inactive. Press Control+T to start monitoring." << std::endl;
    }

    // 이벤트 루프 시작
    CFRunLoopRun();

    return 0;
}
