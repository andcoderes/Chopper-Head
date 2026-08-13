#pragma once
#include <Arduino.h>

// --- Pin assignments (ESP32 WROOM-32) ---
#define PIN_MAESTRO_TX      16   // UART2 TX -> Pololu Maestro
#define PIN_MAESTRO_RX      17   // UART2 RX (reserved)
#define PIN_LEFT_EYE_JEWEL  4    // NeoPixel data, 7 pixels
#define PIN_RIGHT_EYE_JEWEL 18   // NeoPixel data, 7 pixels
#define PIN_CENTER_EYE      5    // NeoPixel data, 1 pixel
#define PIN_LADDER          19   // NeoPixel data, 11 pixels
#define PIN_PERISCOPE       2    // NeoPixel data, 1 pixel

#define NUM_JEWEL_PIXELS    7
#define NUM_SINGLE_PIXEL    1
#define NUM_LADDER_PIXELS   11

// --- Secrets (ESP-NOW keys, controller MAC, WiFi AP password) ---
#ifdef UNIT_TEST
// Dummy values for native unit tests — no real ESP-NOW traffic or AP off-device.
static const uint8_t PMK_KEY[16] = {0};
static const uint8_t LMK_KEY[16] = {0};
static const uint8_t CONTROLLER_MAC[6] = {0};
#define WIFI_AP_PASSWORD "test"
#else
#include "secrets.h"  // defines PMK_KEY, LMK_KEY, CONTROLLER_MAC, WIFI_AP_PASSWORD — generated from .env, see README
#endif

// --- Timing constants ---
#define HEARTBEAT_INTERVAL_MS   1000   // How often to send telemetry
#define CONNECTION_TIMEOUT_MS   5000   // No data = disconnected

// --- WiFi AP for web server ---
#define WIFI_AP_SSID     "ChopperHead"
// WIFI_AP_PASSWORD comes from secrets.h (generated from .env, see README)
