#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class Ladder {
public:
    Ladder(int pin, int numPixels);
    void setup();
    void standBy();
    void changeColor(uint8_t r, uint8_t g, uint8_t b);
    void setConnectionStatus(bool connected);

#ifdef UNIT_TEST
public:
#else
private:
#endif
    int pin_;
    int numPixels_;
    int currentPixel_ = 0;
    bool direction_ = true;  // true = up, false = down
    Adafruit_NeoPixel neo_;
    unsigned long latestMillis_ = 0;
    bool connected_ = false;
    int pulseValue_ = 0;

    void bouncePixel();
    void connectedAnimation();
    void disconnectedAnimation();
};
