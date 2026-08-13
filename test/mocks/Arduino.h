#pragma once

// Mock Arduino.h for native unit tests

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>

// --- Arduino types ---
typedef uint8_t byte;
typedef bool boolean;

// --- Constants ---
#define HIGH 1
#define LOW  0
#define INPUT  0
#define OUTPUT 1
#define LED_BUILTIN 2

#define SERIAL_8N1 0x800001c

#define WIFI_IF_STA 0
#define WIFI_STA    1

// NeoPixel type aliases
typedef uint16_t neoPixelType;
#define NEO_GRB    ((neoPixelType)0x0001)
#define NEO_RGBW   ((neoPixelType)0x0002)
#define NEO_KHZ800 ((neoPixelType)0x0100)

// Arduino min/max macros
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

// --- Controllable millis ---
extern unsigned long mock_millis_value;
inline unsigned long millis() { return mock_millis_value; }

// --- Stubs ---
inline void delay(unsigned long) {}
inline long random(long max) { return std::rand() % max; }
inline long random(long min, long max) {
    if (max <= min) return min;
    return min + (std::rand() % (max - min));
}
inline void randomSeed(unsigned long) {}
inline void pinMode(uint8_t, uint8_t) {}
inline void digitalWrite(uint8_t, uint8_t) {}
inline int  digitalRead(uint8_t) { return 0; }
inline int  analogRead(uint8_t)  { return 0; }
inline void noInterrupts() {}
inline void interrupts() {}

// --- Cross-core critical section stubs (single-threaded in native tests) ---
typedef int portMUX_TYPE;
#define portMUX_INITIALIZER_UNLOCKED 0
inline void portENTER_CRITICAL(portMUX_TYPE*) {}
inline void portEXIT_CRITICAL(portMUX_TYPE*) {}

// --- Minimal String class (backed by std::string) ---
class String {
public:
    String() : s_() {}
    String(const char* c) : s_(c ? c : "") {}
    String(const String& o) : s_(o.s_) {}
    String(int val) : s_(std::to_string(val)) {}
    String(long val) : s_(std::to_string(val)) {}
    String(unsigned long val) : s_(std::to_string(val)) {}

    String& operator=(const String& o) { s_ = o.s_; return *this; }
    String& operator=(const char* c)   { s_ = c ? c : ""; return *this; }

    String  operator+(const String& o) const { return String((s_ + o.s_).c_str()); }
    String& operator+=(const String& o) { s_ += o.s_; return *this; }
    String& operator+=(const char* c)   { s_ += c ? c : ""; return *this; }

    bool operator==(const String& o) const { return s_ == o.s_; }
    bool operator==(const char* c)   const { return s_ == (c ? c : ""); }
    bool operator!=(const String& o) const { return s_ != o.s_; }
    bool operator!=(const char* c)   const { return s_ != (c ? c : ""); }

    const char* c_str() const { return s_.c_str(); }
    int length() const { return (int)s_.size(); }

    friend bool operator==(const char* lhs, const String& rhs) {
        return rhs == lhs;
    }

private:
    std::string s_;
};

// --- HardwareSerial stub ---
class HardwareSerial {
public:
    void begin(unsigned long) {}
    void begin(unsigned long, int, int, int) {}
    void print(const char*) {}
    void print(int) {}
    void print(unsigned long) {}
    void println(const char* = "") {}
    void println(int) {}
    void println(unsigned long) {}
    void println(const String& s) { (void)s; }
    int  available() { return mockAvailable; }
    int  read()      { return mockReadValue; }
    void write(uint8_t b) { lastWrittenByte = b; writeCallCount++; }
    String macAddress() { return String("AA:BB:CC:DD:EE:FF"); }

    // --- Test inspection ---
    int mockAvailable = 0;
    int mockReadValue = 0;
    uint8_t lastWrittenByte = 0;
    int writeCallCount = 0;
};

// --- Global serial instances (defined in mock_arduino.cpp) ---
extern HardwareSerial Serial;
extern HardwareSerial Serial2;
