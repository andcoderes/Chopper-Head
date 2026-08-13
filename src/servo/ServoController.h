#pragma once
#include <Arduino.h>
#include <PololuMaestro.h>
#include "../config.h"

// Animation scripts on the Maestro:
// 0 = take out arms
// 1 = put arms back
// 2 = say hi
// 3 = say hi left arm
// 4 = periscope out
// 5 = periscope in
// 6 = periscope animation

class ServoController {
public:
    void setup();
    void animate(const char* button, const int16_t macros[], int macroCount);
    void execute(int script);
    void stop();
    bool isReady() const;
    bool checkConnection();
    // True while a getErrors handshake byte has been sent and its 2-byte
    // reply hasn't arrived yet — callers sharing Serial2 (e.g. the web
    // monitor's drain) should not consume bytes while this is true.
    bool isHandshakePending() const { return checkPending_; }

#ifdef UNIT_TEST
public:
#else
private:
#endif
    MiniMaestro maestro_{Serial2};

    static const int NUM_KEYS = 7;
    String arrayKeys_[NUM_KEYS] = {"y", "x", "b", "du", "dd", "dr", "dl"};
    int currentIndex_[NUM_KEYS] = {0, 0, 0, 0, 0, 0, 0};
    int scripts_[NUM_KEYS][2] = {{0,1}, {2,2}, {3,3}, {4,4}, {5,5}, {6,6}, {6,6}};

    bool outComponent_[2] = {false, false};
    int animationRequiredHandOut_[3] = {1, 2, 3};
    int outHandAnimation_ = 0;
    int inHandAnimation_ = 1;

    bool ready_ = false;
    bool checkPending_ = false;
    unsigned long checkSentTime_ = 0;

    void playScriptButton(const String& keysAction);
    bool checkAnimation(int animation);
};
