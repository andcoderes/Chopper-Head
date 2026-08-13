#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define JEWEL_BASE_R 0
#define JEWEL_BASE_G 0
#define JEWEL_BASE_B 40

class JewelEye {
public:
    JewelEye(int pin, int numPixels);
    void setup();
    void standBy();
    void pulse();
    void changeColor(uint8_t r, uint8_t g, uint8_t b);
    void setDefaultColor();
    void redEye();

#ifdef UNIT_TEST
public:
#else
private:
#endif
    int pin_;
    int numPixels_;
    Adafruit_NeoPixel neo_;
    unsigned long latestMillis_ = 0;
    int baseR = JEWEL_BASE_R;
    int baseG = JEWEL_BASE_G;
    int baseB = JEWEL_BASE_B;
    int pulseValue_ = 5;
    bool pulseDirection_ = true;
};
