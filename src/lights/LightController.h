#pragma once
#include <Arduino.h>
#include "JewelEye.h"
#include "Ladder.h"
#include "PixelLight.h"
#include "../config.h"

class LightController {
public:
    LightController();
    void setup();
    void standBy();
    void animate(const char* button, const int16_t macros[], int macroCount);
    void setConnectionStatus(bool connected);
    void setMaestroReady(bool ready);

#ifdef UNIT_TEST
public:
#else
private:
#endif
    JewelEye leftEye_;
    JewelEye rightEye_;
    PixelLight centerEye_;
    Ladder ladder_;
    PixelLight periscope_;

    bool maestroReady_ = false;
    String currentRoutine_ = "";
    unsigned long latestTime_ = 0;
    unsigned long limitTime_ = 0;
    unsigned long routineTime_ = 0;
    int customAnimationStep_ = 0;

    // Macro IDs and their durations
    static const int NUM_MACROS = 3;
    int macroIds_[NUM_MACROS] = {101, 103, 107};
    int macroDurations_[NUM_MACROS] = {2450, 15500, 8300};

    unsigned long getDuration(int macroId);
};
