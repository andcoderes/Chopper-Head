#ifndef UNIT_TEST

#define MONITOR_BUF_OWNER
#include "WebController.h"
#include "logger.h"
#include "web_pages.h"
#include <Update.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "0.0.0"
#endif

void WebController::setup() {
    monDrainInit(drainSerial_,  "LOG: ", 'i');
    monDrainInit(drainSerial2_, "S2: ",  'r');

    server_.on("/",        HTTP_GET,  [this]() { handleHome(); });
    server_.on("/monitor", HTTP_GET,  [this]() { handleMonitor(); });
    server_.on("/api/monitor", HTTP_GET, [this]() { handleMonitorApi(); });
    server_.on("/update",  HTTP_GET,  [this]() { handleUpdate(); });
    server_.on("/update",  HTTP_POST,
        [this]() { handleUpdateUpload(); },
        [this]() {
            HTTPUpload& upload = server_.upload();
            if (upload.status == UPLOAD_FILE_START) {
                updateSuccess_ = false;
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                    Update.printError(Serial);
                }
            } else if (upload.status == UPLOAD_FILE_WRITE) {
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                    Update.printError(Serial);
                }
            } else if (upload.status == UPLOAD_FILE_END) {
                if (Update.end(true)) {
                    updateSuccess_ = true;
                    logAll("OTA update success, rebooting...");
                } else {
                    Update.printError(Serial);
                }
            }
        }
    );
    server_.on("/api/command", HTTP_POST, [this]() { handleCommandApi(); });
    server_.on("/api/info", HTTP_GET, [this]() { handleInfoApi(); });

    // DNS: resolve all queries to the AP IP so "chopper.head" (or any URL) works
    dns_.start(53, "*", WiFi.softAPIP());

    // mDNS: advertise "chopper.local" for devices that support it
    if (MDNS.begin("chopper")) {
        MDNS.addService("http", "tcp", 80);
    }

    server_.begin();
    logAll("Web server started on http://chopper.head");
}

void WebController::loop(bool suppressSerial2Drain) {
    dns_.processNextRequest();
    server_.handleClient();

    // Drain Serial (USB console) into monitor buffer
    uint32_t now = millis();
    monDrainSeedLabel(drainSerial_);
    while (Serial.available()) {
        monDrainByte(drainSerial_, Serial.read(), now);
    }
    monDrainSilenceCheck(drainSerial_, now);

    // Drain Serial2 (Maestro UART) into monitor buffer — unless a
    // ServoController handshake is pending, in which case leave the bytes
    // alone so checkConnection() can see its 2-byte reply intact.
    if (!suppressSerial2Drain) {
        monDrainSeedLabel(drainSerial2_);
        while (Serial2.available()) {
            monDrainByte(drainSerial2_, Serial2.read(), now);
        }
        monDrainSilenceCheck(drainSerial2_, now);
    }
}

void WebController::handleHome() {
    server_.send(200, "text/html", WEB_PAGE_HOME);
}

void WebController::handleMonitor() {
    server_.send(200, "text/html", WEB_PAGE_MONITOR);
}

void WebController::handleMonitorApi() {
    String json;
    json.reserve(MON_LINES * (MON_LINE_LEN + 24));
    json += "{\"seq\":";
    json += sMonSeq;
    json += ",\"lines\":[";

    uint16_t count = sMonCount;
    uint16_t start = (sMonHead + MON_LINES - count) % MON_LINES;
    for (uint16_t i = 0; i < count; i++) {
        uint16_t idx = (start + i) % MON_LINES;
        if (i > 0) json += ',';
        json += "{\"t\":\"";
        monJsonAppendEscaped(json, sMonBuf[idx].text);
        json += "\",\"c\":\"";
        switch (sMonBuf[idx].cls) {
            case 't': json += "tx"; break;
            case 'r': json += "rx"; break;
            default:  json += "info"; break;
        }
        json += "\"}";
    }
    json += "]}";
    server_.send(200, "application/json", json);
}

void WebController::handleCommandApi() {
    if (!commandCallback_) {
        server_.send(500, "text/plain", "No command handler");
        return;
    }

    String button = server_.arg("button");
    String macro  = server_.arg("macro");

    HeadCommand cmd = {};
    cmd.msgType = 1;
    cmd.status  = STATUS_BUTTONS;
    cmd.connectionStatus = 1;

    if (button.length() > 0) {
        strncpy(cmd.button, button.c_str(), sizeof(cmd.button) - 1);
    }

    if (macro.length() > 0) {
        // Parse comma-separated macro values (e.g. "101,103")
        int idx = 0;
        int start = 0;
        for (int i = 0; i <= (int)macro.length() && idx < 4; i++) {
            if (i == (int)macro.length() || macro[i] == ',') {
                if (i > start) {
                    cmd.macro[idx++] = (int16_t)macro.substring(start, i).toInt();
                }
                start = i + 1;
            }
        }
    }

    // Log to monitor
    String logMsg = "WEB> ";
    if (button.length() > 0) {
        logMsg += "btn=" + button;
    }
    if (macro.length() > 0) {
        if (button.length() > 0) logMsg += " ";
        logMsg += "macro=" + macro;
    }
    monAppend(logMsg.c_str(), 't');

    commandCallback_(cmd);
    server_.send(200, "text/plain", "OK");
}

void WebController::handleUpdate() {
    server_.send(200, "text/html", WEB_PAGE_UPDATE);
}

void WebController::handleUpdateUpload() {
    server_.send(200, "text/plain", updateSuccess_ ? "OK" : "FAIL");
    if (updateSuccess_) {
        delay(500);
        ESP.restart();
    }
}

void WebController::handleInfoApi() {
    String json;
    json.reserve(256);
    json += "{\"version\":\"";
    json += FIRMWARE_VERSION;
    json += "\",\"uptime\":";
    json += millis();
    json += ",\"heap\":";
    json += ESP.getFreeHeap();
    json += ",\"mac\":\"";
    json += WiFi.macAddress();
    json += "\",\"espnow\":true}";
    server_.send(200, "application/json", json);
}

#endif // UNIT_TEST
