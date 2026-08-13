#pragma once

#include <Arduino.h>

#ifdef UNIT_TEST

// In unit tests, logAll is a no-op
inline void logAll(const char* fmt, ...) { (void)fmt; }

#else

#include "monitor_buf.h"

// Logs a message to Serial (USB) and the web monitor buffer simultaneously.
// Use like printf().
inline void logAll(const char* fmt, ...) __attribute__((format(printf, 1, 2)));
inline void logAll(const char* fmt, ...) {
    char buf[MON_LINE_LEN];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    Serial.println(buf);
    monAppend(buf, 'i');
}

#endif
