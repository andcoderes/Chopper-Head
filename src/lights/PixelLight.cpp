#include "PixelLight.h"

PixelLight::PixelLight(int pin, neoPixelType type)
    : pin_(pin),
      neo_(Adafruit_NeoPixel(1, pin, type)) {}

void PixelLight::setup() {
    neo_.begin();
    neo_.clear();
    neo_.show();
}

void PixelLight::setBlink(bool shouldBlink) {
    shouldBlink_ = shouldBlink;
}

void PixelLight::off() {
    isOn_ = false;
    neo_.setPixelColor(0, neo_.Color(0, 0, 0));
    neo_.show();
}

void PixelLight::on() {
    isOn_ = true;
    neo_.setPixelColor(0, neo_.Color(255, 0, 0));
    neo_.show();
}

void PixelLight::swapLight() {
    if (isOn_) {
        off();
    } else {
        on();
    }
}

void PixelLight::changeColor(uint8_t r, uint8_t g, uint8_t b) {
    neo_.setPixelColor(0, neo_.Color(r, g, b));
    neo_.show();
}

void PixelLight::setConnectionStatus(bool connected) {
    connectionMode_ = true;
    connected_ = connected;
    pulseValue_ = 0;
}

void PixelLight::standBy() {
    unsigned long currentTime = millis();

    // Connection-status mode (ladder pixel)
    if (connectionMode_) {
        if (latestMillis_ == 0 || (currentTime - latestMillis_) >= 100) {
            if (connected_) {
                // Connected: solid dim red
                neo_.setPixelColor(0, neo_.Color(50, 0, 0));
            } else {
                // Disconnected: pulse red brightness up/down
                if (pulseUp_) {
                    pulseValue_ += 15;
                    if (pulseValue_ >= 240) {
                        pulseUp_ = false;
                        pulseValue_ = 250;
                    }
                } else {
                    pulseValue_ -= 15;
                    if (pulseValue_ <= 5) {
                        pulseUp_ = true;
                        pulseValue_ = 5;
                    }
                }
                neo_.setPixelColor(0, neo_.Color(pulseValue_, 0, 0));
            }
            neo_.show();
            latestMillis_ = currentTime;
        }
        return;
    }

    // Normal standBy mode (pixel eye / periscope)
    if (isOn_ && (latestMillis_ == 0 || (currentTime - latestMillis_) >= 50)) {
        int chance = random(100);
        if (chance < 20) {
            blinkState_ = !blinkState_;
        }
        if (shouldBlink_ && !blinkState_) {
            neo_.setPixelColor(0, neo_.Color(0, 0, 0));
        } else {
            neo_.setPixelColor(0, neo_.Color(255, 0, 0));
        }
        neo_.show();
        latestMillis_ = currentTime;
    }
}
