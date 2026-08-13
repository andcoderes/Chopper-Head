#include "EspNowController.h"
#include "../web/logger.h"

// Static member definitions
volatile bool EspNowController::dataReady_ = false;
HeadCommand EspNowController::incomingBuffer_ = {};
unsigned long EspNowController::lastRecvTime_ = 0;
volatile bool EspNowController::everReceived_ = false;

// Cross-core critical section guarding incomingBuffer_. noInterrupts()/
// interrupts() only mask the current core on the dual-core ESP32 and do not
// protect against onDataRecv() (WiFi task, can run on either core) writing
// concurrently with loop() (Arduino task) reading.
static portMUX_TYPE bufMux_ = portMUX_INITIALIZER_UNLOCKED;

void EspNowController::onDataRecv(const esp_now_recv_info_t* info,
                                   const uint8_t* data, int len) {
    if (len != sizeof(HeadCommand)) return;

    portENTER_CRITICAL(&bufMux_);
    memcpy(&incomingBuffer_, data, sizeof(HeadCommand));
    // Force null-termination: the sender's raw bytes aren't guaranteed to
    // contain one, and downstream code treats button[] as a C string.
    incomingBuffer_.button[sizeof(incomingBuffer_.button) - 1] = '\0';
    portEXIT_CRITICAL(&bufMux_);
    lastRecvTime_ = millis();
    everReceived_ = true;
    dataReady_ = true;
}

void EspNowController::onDataSent(const esp_now_send_info_t* tx_info,
                                   esp_now_send_status_t status) {
    // ESP-NOW controller is optional — silently ignore send failures
}

void EspNowController::setup() {
    // WiFi mode is set in main.cpp (AP_STA for web server coexistence)

    if (esp_now_init() != ESP_OK) {
        logAll("ESP-NOW init failed");
        return;
    }

    // Set Primary Master Key
    esp_now_set_pmk(PMK_KEY);

    // Register controller as encrypted peer
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, CONTROLLER_MAC, 6);
    peerInfo.channel = 0;
    memcpy(peerInfo.lmk, LMK_KEY, 16);
    peerInfo.encrypt = true;
    peerInfo.ifidx = WIFI_IF_STA;

    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        logAll("Failed to add controller peer");
        return;
    }

    // Register callbacks
    esp_now_register_recv_cb(onDataRecv);
    esp_now_register_send_cb(onDataSent);

    logAll("MAC Address: %s", WiFi.macAddress().c_str());
    logAll("ESP-NOW initialized");
}

void EspNowController::setCommandCallback(CommandCallback cb) {
    callback_ = cb;
}

void EspNowController::loop() {
    // Check for incoming data
    if (dataReady_) {
        dataReady_ = false;
        HeadCommand cmd;
        portENTER_CRITICAL(&bufMux_);
        memcpy(&cmd, (const void*)&incomingBuffer_, sizeof(HeadCommand));
        portEXIT_CRITICAL(&bufMux_);

        if (callback_) {
            callback_(cmd);
        }
    }

    // Send periodic telemetry
    unsigned long now = millis();
    if (now - lastHeartbeat_ >= HEARTBEAT_INTERVAL_MS) {
        sendTelemetry(false);
        lastHeartbeat_ = now;
    }
}

void EspNowController::sendTelemetry(bool animationRunning) {
    HeadTelemetry telem = {};
    telem.msgType = 2;
    telem.connected = isConnected() ? 1 : 0;
    telem.animationRunning = animationRunning ? 1 : 0;
    telem.uptimeMs = millis();

    esp_now_send(CONTROLLER_MAC, (uint8_t*)&telem, sizeof(telem));
}

bool EspNowController::isConnected() {
    return everReceived_ && (millis() - lastRecvTime_) < CONNECTION_TIMEOUT_MS;
}
