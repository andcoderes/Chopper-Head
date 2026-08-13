#pragma once

#include "Arduino.h"

#define MAX_MOCK_PIXELS 16

class Adafruit_NeoPixel {
public:
    Adafruit_NeoPixel() : numPixels_(0), pin_(0), type_(0) { clearPixels(); }
    Adafruit_NeoPixel(uint16_t n, int16_t p, neoPixelType t = NEO_GRB)
        : numPixels_(n), pin_(p), type_(t) { clearPixels(); }

    void begin() { beginCallCount++; }
    void show()  { showCallCount++; }

    void setPixelColor(uint16_t n, uint32_t c) {
        if (n < MAX_MOCK_PIXELS) {
            lastPixelColors[n] = c;
        }
        lastSetPixel = n;
        lastSetColor = c;
    }

    void clear() {
        for (uint16_t i = 0; i < numPixels_ && i < MAX_MOCK_PIXELS; i++) {
            lastPixelColors[i] = 0;
        }
        lastSetColor = 0;
    }

    void fill(uint32_t c, uint16_t first = 0, uint16_t count = 0) {
        uint16_t end = (count == 0) ? numPixels_ : first + count;
        for (uint16_t i = first; i < end && i < MAX_MOCK_PIXELS; i++) {
            lastPixelColors[i] = c;
        }
        lastSetColor = c;
        fillCallCount++;
    }

    static uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
        return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }

    static uint32_t Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
        return ((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }

    uint16_t numPixels() const { return numPixels_; }

    // --- Inspection variables ---
    uint32_t lastSetColor = 0;
    uint16_t lastSetPixel = 0;
    int showCallCount = 0;
    int beginCallCount = 0;
    int fillCallCount = 0;
    uint32_t lastPixelColors[MAX_MOCK_PIXELS] = {};

    void clearPixels() {
        for (int i = 0; i < MAX_MOCK_PIXELS; i++) lastPixelColors[i] = 0;
        lastSetColor = 0;
        lastSetPixel = 0;
        showCallCount = 0;
        beginCallCount = 0;
        fillCallCount = 0;
    }

private:
    uint16_t numPixels_;
    int16_t pin_;
    neoPixelType type_;
};
