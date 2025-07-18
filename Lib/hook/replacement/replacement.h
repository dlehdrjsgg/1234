#pragma once
#include <ApplicationServices/ApplicationServices.h>

// event_callback.cpp 에서 호출.
// 인자로 받은 event 를 직접 수정하거나, 대체 입력을
// postEvent() 로 처리. 교체가 끝나면 true 리턴.
bool processReplacement(CGEventRef event);