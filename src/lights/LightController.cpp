#include "LightController.h"
#include "../web/logger.h"

LightController::LightController()
    : leftEye_(PIN_LEFT_EYE_JEWEL, NUM_JEWEL_PIXELS),
      rightEye_(PIN_RIGHT_EYE_JEWEL, NUM_JEWEL_PIXELS),
      centerEye_(PIN_CENTER_EYE),
      ladder_(PIN_LADDER, NUM_LADDER_PIXELS),
      periscope_(PIN_PERISCOPE) {}

void LightController::setup() {
    leftEye_.setup();
    rightEye_.setup();
    centerEye_.setup();
    ladder_.setup();
    periscope_.setup();
    periscope_.setBlink(false);
    periscope_.off();
    // Ladder starts in connection-status mode
    ladder_.setConnectionStatus(false);
}

unsigned long LightController::getDuration(int macroId) {
    for (int i = 0; i < NUM_MACROS; i++) {
        if (macroIds_[i] == macroId) {
            return (unsigned long)macroDurations_[i];
        }
    }
    return 0;
}

void LightController::setConnectionStatus(bool connected) {
    ladder_.setConnectionStatus(connected);
}

void LightController::setMaestroReady(bool ready) {
    maestroReady_ = ready;
}

void LightController::standBy() {
    if (!maestroReady_) {
        leftEye_.pulse();
        rightEye_.pulse();
        centerEye_.standBy();
        ladder_.standBy();
        return;
    }

    if (currentRoutine_ == "107") {
        if (customAnimationStep_ == 0) {
            leftEye_.changeColor(random(30, 190), random(30, 190), random(30, 190));
            rightEye_.changeColor(random(30, 190), random(30, 190), random(30, 190));
            ladder_.changeColor(random(30, 190), random(30, 190), random(30, 190));
            centerEye_.changeColor(random(30, 190), random(30, 190), random(30, 190));
            if (millis() - routineTime_ > 4000) {
                customAnimationStep_ = 1;
            }
        } else if (customAnimationStep_ == 1) {
            leftEye_.changeColor(0, 0, 0);
            rightEye_.changeColor(0, 0, 0);
            ladder_.changeColor(0, 0, 0);
            centerEye_.changeColor(0, 0, 0);
            if (millis() - routineTime_ > 6000) {
                customAnimationStep_ = 2;
            }
        } else if (customAnimationStep_ == 2) {
            leftEye_.changeColor(random(30, 190), 0, 0);
            rightEye_.changeColor(random(30, 190), 0, 0);
            ladder_.changeColor(random(30, 190), 0, 0);
            centerEye_.changeColor(random(30, 190), 0, 0);
        }
    } else {
        leftEye_.standBy();
        rightEye_.standBy();
        ladder_.standBy();
        centerEye_.standBy();
        periscope_.standBy();
    }

    // Reset timed routine when duration expires
    if (currentRoutine_ != "" && millis() - latestTime_ > limitTime_) {
        logAll("reboot elements");
        leftEye_.setDefaultColor();
        rightEye_.setDefaultColor();
        // Ladder/PixelLight have no setDefaultColor() equivalent — calling
        // their own standBy() forces an immediate repaint via their normal
        // animation logic instead of leaving them at the routine's last
        // changeColor() value until their next self-timed tick.
        ladder_.standBy();
        centerEye_.standBy();
        latestTime_ = millis();
        periscope_.off();
        currentRoutine_ = "";
        customAnimationStep_ = 0;
    }
}

void LightController::animate(const char* button, const int16_t macros[], int macroCount) {
    // Macro-based animations
    if (macroCount > 0 && macros[0] == 101) {
        currentRoutine_ = "101";
        latestTime_ = millis();
        limitTime_ = getDuration(101);
        logAll("Red eye");
        leftEye_.redEye();
        rightEye_.redEye();
    } else if (macroCount > 0 && macros[0] == 103) {
        currentRoutine_ = "103";
        latestTime_ = millis();
        limitTime_ = getDuration(103);
        periscope_.on();
    } else if (macroCount > 0 && macros[0] == 107) {
        currentRoutine_ = "107";
        routineTime_ = millis();
        latestTime_ = millis();
        limitTime_ = getDuration(107);
    }

    // Button-based animations
    String btn(button);
    if (btn == "du") {
        periscope_.on();
    } else if (btn == "dd") {
        periscope_.off();
    } else if (btn == "dl" || btn == "dr") {
        periscope_.on();
    }
}
