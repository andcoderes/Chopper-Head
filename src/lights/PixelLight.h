#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class PixelLight {
public:
    PixelLight(int pin, neoPixelType type = NEO_GRB);
    void setup();
    void standBy();
    void changeColor(uint8_t r, uint8_t g, uint8_t b);
    void off();
    void on();
    void swapLight();
    void setBlink(bool shouldBlink);
    void setConnectionStatus(bool connected);

#ifdef UNIT_TEST
public:
#else
private:
#endif
    int pin_;
    Adafruit_NeoPixel neo_;
    unsigned long latestMillis_ = 0;
    bool blinkState_ = false;
    bool shouldBlink_ = true;
    bool isOn_ = true;
    bool connectionMode_ = false;
    bool connected_ = false;
    int pulseValue_ = 0;
    bool pulseUp_ = true;
};
