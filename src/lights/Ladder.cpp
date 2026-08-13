#include "Ladder.h"

Ladder::Ladder(int pin, int numPixels)
    : pin_(pin),
      numPixels_(numPixels),
      neo_(Adafruit_NeoPixel(numPixels, pin, NEO_GRB + NEO_KHZ800)) {}

void Ladder::setup() {
    neo_.begin();
}

void Ladder::bouncePixel() {
    if (direction_) {
        currentPixel_++;
        if (currentPixel_ >= numPixels_) {
            direction_ = false;
            currentPixel_ = numPixels_ - 2;
        }
    } else {
        currentPixel_--;
        if (currentPixel_ <= -1) {
            direction_ = true;
            currentPixel_ = 1;
        }
    }
}

void Ladder::connectedAnimation() {
    unsigned long now = millis();
    if (latestMillis_ == 0 || (now - latestMillis_) >= 100) {
        bouncePixel();
        neo_.fill(neo_.Color(50, 0, 0));
        neo_.setPixelColor(currentPixel_, neo_.Color(255, 0, 0));
        neo_.show();
        latestMillis_ = now;
    }
}

void Ladder::disconnectedAnimation() {
    unsigned long now = millis();
    if (latestMillis_ == 0 || (now - latestMillis_) >= 100) {
        if (direction_) {
            pulseValue_ += 15;
            if (pulseValue_ >= 240) {
                direction_ = false;
                pulseValue_ = 250;
            }
        } else {
            pulseValue_ -= 15;
            if (pulseValue_ <= 5) {
                direction_ = true;
                pulseValue_ = 5;
            }
        }
        neo_.fill(neo_.Color(pulseValue_, 0, 0));
        neo_.show();
        latestMillis_ = now;
    }
}

void Ladder::changeColor(uint8_t r, uint8_t g, uint8_t b) {
    if (r == 0 && g == 0 && b == 0) {
        neo_.fill(neo_.Color(0, 0, 0));
        neo_.show();
        return;
    }
    bouncePixel();
    neo_.fill(neo_.Color(max(0, r - 10), max(0, g - 10), max(0, b - 10)));
    neo_.setPixelColor(currentPixel_, neo_.Color(min(255, r + 50), min(255, g + 50), min(255, b + 50)));
    neo_.show();
}

void Ladder::setConnectionStatus(bool connected) {
    if (connected == connected_) return;  // called every loop tick — only reset on an actual transition
    currentPixel_ = 0;
    connected_ = connected;
}

void Ladder::standBy() {
    if (connected_) {
        connectedAnimation();
    } else {
        disconnectedAnimation();
    }
}
