#include "Arduino.h"
#include "WiFi.h"

// Global mock state
unsigned long mock_millis_value = 0;

// Global instances
HardwareSerial Serial;
HardwareSerial Serial2;
WiFiClass WiFi;
