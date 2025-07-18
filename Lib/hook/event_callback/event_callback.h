#pragma once
#include <ApplicationServices/ApplicationServices.h>
#include <string>
#include <vector>
#include <sstream>
void postUnicodeChar(char16_t c);
// install_event_tap.cpp 에서 전달하는 콜백
CGEventRef eventCallback(CGEventTapProxy proxy,
                         CGEventType   type,
                         CGEventRef    event,
                         void*         refcon);