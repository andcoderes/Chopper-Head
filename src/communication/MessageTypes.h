#pragma once
#include <Arduino.h>

// Status codes (matching V1 protocol)
#define STATUS_BUTTONS    0
#define STATUS_CONNECTION -1
#define STATUS_SETTINGS   2
#define STATUS_DEBUG      99

// --- Controller -> Head ---
struct HeadCommand {
    int8_t  msgType;           // always 1
    int8_t  status;            // 0=buttons, 2=settings, -1=connection, 99=debug
    char    button[8];         // null-terminated button string (e.g. "y", "du", "dd")
    int16_t macro[4];          // up to 4 macro values (e.g. 101, 103, 107)
    uint8_t volume;
    uint8_t idleMode;
    int8_t  connectionStatus;  // 0=disconnected, 1=connected
} __attribute__((packed));

// --- Head -> Controller ---
struct HeadTelemetry {
    int8_t  msgType;           // always 2
    uint8_t connected;         // 1 if ESP-NOW link is active
    uint8_t animationRunning;  // 1 if a servo animation is in progress
    uint8_t padding;
    uint32_t uptimeMs;
} __attribute__((packed));
