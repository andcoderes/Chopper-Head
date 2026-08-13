#include "ServoController.h"
#include "../web/logger.h"

void ServoController::setup() {
    Serial2.begin(9600, SERIAL_8N1, PIN_MAESTRO_RX, PIN_MAESTRO_TX);
    delay(500);
}

bool ServoController::isReady() const {
    return ready_;
}

bool ServoController::checkConnection() {
    if (ready_) return true;

    if (!checkPending_) {
        // Flush any stale data
        while (Serial2.available()) Serial2.read();
        // Send getErrors compact protocol command
        logAll("Maestro: trying to connect...");
        Serial2.write(0xA1);
        checkPending_ = true;
        checkSentTime_ = millis();
        return false;
    }

    // Waiting for response
    if (Serial2.available() >= 2) {
        Serial2.read();
        Serial2.read();
        ready_ = true;
        checkPending_ = false;
        logAll("Maestro: connected");
        return true;
    }

    // Timeout after 100ms — retry next loop
    if (millis() - checkSentTime_ >= 100) {
        checkPending_ = false;
    }

    return false;
}

void ServoController::stop() {
    maestro_.stopScript();
}

void ServoController::animate(const char* button, const int16_t macros[], int macroCount) {
    // Try button first
    if (button[0] != '\0') {
        logAll("Servo: btn=%s", button);
        playScriptButton(String(button));
        return;
    }

    // Try macros - build a key string from the macro values
    String macrosString = "";
    for (int i = 0; i < macroCount && macros[i] != 0; i++) {
        macrosString += String(macros[i]);
    }

    if (macrosString != "") {
        logAll("Servo: macro=%s", macrosString.c_str());
        playScriptButton(macrosString);
    }
}

void ServoController::playScriptButton(const String& keysAction) {
    int actionIndex = -1;
    for (int i = 0; i < NUM_KEYS; i++) {
        if (arrayKeys_[i] == keysAction) {
            actionIndex = i;
            break;
        }
    }

    if (actionIndex != -1) {
        int currentIndex = currentIndex_[actionIndex];
        int scriptToRun = scripts_[actionIndex][currentIndex];
        currentIndex = (currentIndex + 1) % 2;
        currentIndex_[actionIndex] = currentIndex;

        if (checkAnimation(scriptToRun)) {
            logAll("Servo: restart script %d", scriptToRun);
            maestro_.restartScript(scriptToRun);
        }
    }
}

void ServoController::execute(int scriptToRun) {
    if (checkAnimation(scriptToRun)) {
        maestro_.restartScript(scriptToRun);
    }
}

bool ServoController::checkAnimation(int animation) {
    if (animation == outHandAnimation_) {
        outComponent_[0] = true;
        return true;
    }
    if (outComponent_[0]) {
        if (animation == inHandAnimation_) {
            outComponent_[0] = false;
            return true;
        }
        for (int x = 0; x < 3; x++) {
            if (animationRequiredHandOut_[x] == animation) {
                return true;
            }
        }
    }
    if (animation > 3) {
        return true;
    }
    return false;
}
