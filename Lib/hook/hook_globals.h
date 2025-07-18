#pragma once
#include <ApplicationServices/ApplicationServices.h>

// 이벤트 탭 핸들을 전역으로 공유하기 위한 선언
extern CFMachPortRef gEventTap;