#pragma once
#include <ApplicationServices/ApplicationServices.h>

// CGEventPost + CFRelease 를 캡슐화
void postEvent(CGEventRef ev);