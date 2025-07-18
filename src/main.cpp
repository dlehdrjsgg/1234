#include "install_event_tap.h"
#include <CoreFoundation/CoreFoundation.h>
#include <iostream>

int main() {
    // Lib/hook/install_event_tap 모듈에서 이벤트 탭을 설치합니다.
    installEventTap();

    std::cout << "TUNEY\n";

    // 이벤트 루프 시작
    CFRunLoopRun();

    return 0;
}