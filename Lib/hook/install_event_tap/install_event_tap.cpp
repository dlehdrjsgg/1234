#include "install_event_tap.h"
#include "../hook_globals.h"
#include "event_callback.h"
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <iostream>

CFMachPortRef gEventTap = nullptr;

void installEventTap() {
    CGEventMask mask = CGEventMaskBit(kCGEventKeyDown)
                       | CGEventMaskBit(kCGEventKeyUp);


    gEventTap = CGEventTapCreate(
         kCGHIDEventTap,        // ← 여기만 바꿔주세요
         kCGHeadInsertEventTap,
         kCGEventTapOptionDefault,
         mask,
         eventCallback,
         nullptr
    );
    if (!gEventTap) {
        std::cerr << "이벤트 탭 생성 실패\n";
        exit(1);
    }
    std::cout << "[+] Event tap installed (HID, keyboard only)\n";
    CFRunLoopSourceRef src = CFMachPortCreateRunLoopSource(
        kCFAllocatorDefault,
        gEventTap,
        0
    );
    CFRunLoopAddSource(CFRunLoopGetCurrent(), src, kCFRunLoopCommonModes);
    CGEventTapEnable(gEventTap, true);
}