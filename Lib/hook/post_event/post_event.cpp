#include "post_event.h"

void postEvent(CGEventRef ev) {
    CGEventPost(kCGAnnotatedSessionEventTap, ev);
    CFRelease(ev);
}