#include "JewelEye.h"

JewelEye::JewelEye(int pin, int numPixels)
    : pin_(pin),
      numPixels_(numPixels),
      neo_(Adafruit_NeoPixel(numPixels, pin, NEO_RGBW)) {}

void JewelEye::setup() {
    neo_.begin();
}

void JewelEye::setDefaultColor() {
    baseR = JEWEL_BASE_R;
    baseG = JEWEL_BASE_G;
    baseB = JEWEL_BASE_B;
}

void JewelEye::redEye() {
    baseR = 40;
    baseG = 0;
    baseB = 0;
}

void JewelEye::changeColor(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < 5; i++) {
        neo_.setPixelColor(random(0, numPixels_), neo_.Color(g, r, b));
    }
    neo_.show();
}

void JewelEye::pulse() {
    unsigned long currentTime = millis();
    if (latestMillis_ == 0 || (currentTime - latestMillis_) >= 100) {
        if (pulseDirection_) {
            pulseValue_ += 15;
            if (pulseValue_ >= 240) {
                pulseDirection_ = false;
                pulseValue_ = 250;
            }
        } else {
            pulseValue_ -= 15;
            if (pulseValue_ <= 5) {
                pulseDirection_ = true;
                pulseValue_ = 5;
            }
        }
        neo_.fill(neo_.Color(0, 0, pulseValue_));
        neo_.show();
        latestMillis_ = millis();
    }
}

void JewelEye::standBy() {
    unsigned long currentTime = millis();
    if (latestMillis_ == 0 || (currentTime - latestMillis_) >= 50) {
        neo_.fill(neo_.Color(baseG, baseR, baseB));
        for (int i = 0; i < 3; i++) {
            int deltaG = baseG == 0 ? 0 : baseG + 60;
            int deltaR = baseR == 0 ? 0 : baseR + 60;
            int deltaB = baseB == 0 ? 0 : baseB + 60;
            neo_.setPixelColor(random(0, numPixels_), neo_.Color(deltaG, deltaR, deltaB));
        }
        neo_.show();
        latestMillis_ = millis();
    }
}
