#ifndef UNIT_TEST

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "config.h"
#include "communication/EspNowController.h"
#include "communication/MessageTypes.h"
#include "servo/ServoController.h"
#include "lights/LightController.h"
#include "web/WebController.h"
#include "web/monitor_buf.h"
#include "web/logger.h"

EspNowController espNow;
ServoController servo;
LightController lights;
WebController webController;

void logCommand(const HeadCommand& cmd) {
    if (cmd.button[0] != '\0') {
        logAll("CMD: btn=%s", cmd.button);
    } else if (cmd.macro[0] != 0) {
        logAll("CMD: macro=%d,%d,%d,%d",
               cmd.macro[0], cmd.macro[1], cmd.macro[2], cmd.macro[3]);
    } else {
        logAll("CMD: status=%d", cmd.status);
    }
}

void onCommand(const HeadCommand& cmd) {
    logCommand(cmd);

    switch (cmd.status) {
        case STATUS_BUTTONS:
            servo.animate(cmd.button, cmd.macro, 4);
            lights.animate(cmd.button, cmd.macro, 4);
            break;

        case STATUS_DEBUG:
            servo.animate(cmd.button, cmd.macro, 4);
            lights.animate(cmd.button, cmd.macro, 4);
            break;

        case STATUS_CONNECTION:
            lights.setConnectionStatus(cmd.connectionStatus == 1);
            break;

        default:
            break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

    servo.setup();
    lights.setup();

    // WiFi: AP+STA mode for ESP-NOW + web server coexistence
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
    WiFi.softAPConfig(IPAddress(192,168,5,1), IPAddress(192,168,5,1), IPAddress(255,255,255,0));
    esp_wifi_set_ps(WIFI_PS_NONE);

    espNow.setup();
    espNow.setCommandCallback(onCommand);

    webController.setup();
    webController.setCommandCallback(onCommand);

    logAll("Chopper V2 Head ready");
    logAll("AP IP: %s", WiFi.softAPIP().toString().c_str());
}

void loop() {
    if (!servo.isReady()) {
        servo.checkConnection();
    }
    lights.setMaestroReady(servo.isReady());

    // Sync from the ESP-NOW liveness timeout *before* dispatching any
    // command this tick, so an explicit STATUS_CONNECTION signal handled
    // during espNow.loop() below is the last word for this iteration,
    // not immediately clobbered by this stale-from-last-tick check.
    lights.setConnectionStatus(espNow.isConnected());

    espNow.loop();
    webController.loop(servo.isHandshakePending());
    lights.standBy();
}

#endif // UNIT_TEST
