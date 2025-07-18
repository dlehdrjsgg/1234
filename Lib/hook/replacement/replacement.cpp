#include "replacement.h"
#include "post_event.h"
#include <string>

// 최근 입력된 Unicode 글자를 저장
static std::u16string bufferChars;

// keycode 상수
static constexpr CGKeyCode BACKSPACE = 51;

bool processReplacement(CGEventRef event) {
    // 1) 입력된 Unicode 한 글자 읽기
    UniChar uniBuf[1];
    UniCharCount len = 0;
    CGEventKeyboardGetUnicodeString(event, 1, &len, uniBuf);
    if (len != 1)
        return false;

    // 2) 버퍼에 추가·유지
    bufferChars.push_back(uniBuf[0]);
    if (bufferChars.size() > 2)
        bufferChars.erase(0, bufferChars.size() - 2);

    // 3) “안녕” 이면 대체
    if (bufferChars == u"안녕") {
        // (a) 앞글자 두 번 지우기
        for (int i = 0; i < 2; ++i) {
            auto delDown = CGEventCreateKeyboardEvent(nullptr, BACKSPACE, true);
            auto delUp   = CGEventCreateKeyboardEvent(nullptr, BACKSPACE, false);
            postEvent(delDown);
            postEvent(delUp);
        }
        // (b) “안녕하세요” 입력
        std::u16string rep = u"안녕하세요";
        for (auto ch : rep) {
            UniChar uc = static_cast<UniChar>(ch);
            // down
            auto rd = CGEventCreateKeyboardEvent(nullptr, (CGKeyCode)0, true);
            CGEventKeyboardSetUnicodeString(rd, 1, &uc);
            postEvent(rd);
            // up
            auto ru = CGEventCreateKeyboardEvent(nullptr, (CGKeyCode)0, false);
            CGEventKeyboardSetUnicodeString(ru, 1, &uc);
            postEvent(ru);
        }
        bufferChars.clear();
        // 원본 이벤트 드롭
        return true;
    }
    return false;
}