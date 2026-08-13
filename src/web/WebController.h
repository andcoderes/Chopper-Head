#pragma once

#include <DNSServer.h>
#include <WebServer.h>
#include "monitor_drain.h"
#include "../communication/MessageTypes.h"

using WebCommandCallback = void (*)(const HeadCommand&);

class WebController {
public:
    void setup();
    // suppressSerial2Drain: skip draining Serial2 this call (e.g. while
    // ServoController has an outstanding Maestro handshake awaiting its
    // reply — draining would steal the response bytes out from under it).
    void loop(bool suppressSerial2Drain = false);
    void setCommandCallback(WebCommandCallback cb) { commandCallback_ = cb; }

private:
    void handleHome();
    void handleMonitor();
    void handleMonitorApi();
    void handleCommandApi();
    void handleUpdate();
    void handleUpdateUpload();
    void handleInfoApi();

    DNSServer dns_;
    WebServer server_{80};
    MonDrainState drainSerial_;
    MonDrainState drainSerial2_;
    bool updateSuccess_ = false;
    WebCommandCallback commandCallback_ = nullptr;
};
