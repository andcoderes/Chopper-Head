#include <unity.h>
#include "servo/ServoController.h"
#include "lights/JewelEye.h"
#include "lights/LightController.h"

static ServoController* sc;
static JewelEye* je;
static LightController* lc;

void setUp(void) {
    mock_millis_value = 0;
    Serial2.mockAvailable = 0;
    Serial2.mockReadValue = 0;
    Serial2.lastWrittenByte = 0;
    Serial2.writeCallCount = 0;
}

void tearDown(void) {}

// --- ServoController: starts not ready ---

void test_servo_starts_not_ready(void) {
    sc = new ServoController();
    TEST_ASSERT_FALSE(sc->isReady());
    delete sc;
}

// --- checkConnection sends 0xA1 on first call ---

void test_checkConnection_sends_getErrors_command(void) {
    sc = new ServoController();
    Serial2.writeCallCount = 0;
    sc->checkConnection();
    TEST_ASSERT_EQUAL(0xA1, Serial2.lastWrittenByte);
    TEST_ASSERT_EQUAL(1, Serial2.writeCallCount);
    delete sc;
}

// --- checkConnection returns false when no response ---

void test_checkConnection_returns_false_when_no_response(void) {
    sc = new ServoController();
    Serial2.mockAvailable = 0;
    bool result = sc->checkConnection();
    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_FALSE(sc->isReady());
    delete sc;
}

// --- checkConnection returns true when 2 bytes available ---

void test_checkConnection_returns_true_when_response_available(void) {
    sc = new ServoController();
    // First call: sends command
    sc->checkConnection();
    // Simulate Maestro responding with 2 bytes
    Serial2.mockAvailable = 2;
    Serial2.mockReadValue = 0;
    bool result = sc->checkConnection();
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(sc->isReady());
    delete sc;
}

// --- checkConnection resets pending on timeout ---

void test_checkConnection_resets_pending_on_timeout(void) {
    sc = new ServoController();
    mock_millis_value = 0;
    sc->checkConnection(); // sends command at t=0
    int writesBefore = Serial2.writeCallCount;

    // Advance past timeout (100ms)
    mock_millis_value = 100;
    Serial2.mockAvailable = 0;
    sc->checkConnection(); // should reset pending

    // Next call should send command again
    mock_millis_value = 101;
    sc->checkConnection();
    TEST_ASSERT_EQUAL(writesBefore + 1, Serial2.writeCallCount);
    delete sc;
}

// --- isReady stays true once connected ---

void test_isReady_stays_true_after_connection(void) {
    sc = new ServoController();
    sc->checkConnection(); // send
    Serial2.mockAvailable = 2;
    sc->checkConnection(); // receive → ready
    TEST_ASSERT_TRUE(sc->isReady());
    // Subsequent calls should still return true
    TEST_ASSERT_TRUE(sc->checkConnection());
    delete sc;
}

// --- JewelEye pulse respects 100ms interval ---

void test_pulse_skips_before_100ms(void) {
    je = new JewelEye(4, 7);
    je->setup();
    mock_millis_value = 1;
    je->pulse(); // first call executes, sets latestMillis_ = 1
    int showAfterFirst = je->neo_.showCallCount;

    mock_millis_value = 50; // only 49ms elapsed
    je->pulse();
    TEST_ASSERT_EQUAL(showAfterFirst, je->neo_.showCallCount);
    delete je;
}

void test_pulse_executes_at_100ms(void) {
    je = new JewelEye(4, 7);
    je->setup();
    mock_millis_value = 0;
    je->pulse();
    int showAfterFirst = je->neo_.showCallCount;

    mock_millis_value = 100;
    je->pulse();
    TEST_ASSERT_GREATER_THAN(showAfterFirst, je->neo_.showCallCount);
    delete je;
}

void test_pulse_increases_value_upward(void) {
    je = new JewelEye(4, 7);
    je->setup();
    mock_millis_value = 0;
    je->pulse(); // pulseValue_ goes from 5 → 20
    TEST_ASSERT_EQUAL(20, je->pulseValue_);
    TEST_ASSERT_TRUE(je->pulseDirection_);
    delete je;
}

void test_pulse_reverses_at_top(void) {
    je = new JewelEye(4, 7);
    je->setup();
    je->pulseValue_ = 235;
    je->pulseDirection_ = true;
    mock_millis_value = 0;
    je->pulse(); // 235 + 15 = 250, hits >= 240
    TEST_ASSERT_EQUAL(250, je->pulseValue_);
    TEST_ASSERT_FALSE(je->pulseDirection_);
    delete je;
}

// --- LightController starts in startup mode ---

void test_lightcontroller_starts_not_maestro_ready(void) {
    lc = new LightController();
    lc->setup();
    TEST_ASSERT_FALSE(lc->maestroReady_);
    delete lc;
}

void test_lightcontroller_startup_mode_pulses_eyes(void) {
    lc = new LightController();
    lc->setup();
    mock_millis_value = 0;
    int leftShowBefore = lc->leftEye_.neo_.showCallCount;
    int rightShowBefore = lc->rightEye_.neo_.showCallCount;
    lc->standBy();
    TEST_ASSERT_GREATER_THAN(leftShowBefore, lc->leftEye_.neo_.showCallCount);
    TEST_ASSERT_GREATER_THAN(rightShowBefore, lc->rightEye_.neo_.showCallCount);
    delete lc;
}

void test_lightcontroller_startup_mode_animates_center_eye(void) {
    // LightController::standBy()'s !maestroReady_ branch calls
    // centerEye_.standBy() alongside leftEye_/rightEye_/ladder_ — it's part
    // of the startup animation, not skipped.
    lc = new LightController();
    lc->setup();
    mock_millis_value = 0;
    int centerShowBefore = lc->centerEye_.neo_.showCallCount;
    lc->standBy();
    TEST_ASSERT_EQUAL(centerShowBefore + 1, lc->centerEye_.neo_.showCallCount);
    delete lc;
}

void test_lightcontroller_transitions_on_maestro_ready(void) {
    lc = new LightController();
    lc->setup();
    lc->setMaestroReady(true);
    TEST_ASSERT_TRUE(lc->maestroReady_);

    // After setting ready, standBy should run normal mode (center eye updates)
    mock_millis_value = 0;
    int centerShowBefore = lc->centerEye_.neo_.showCallCount;
    lc->standBy();
    TEST_ASSERT_GREATER_THAN(centerShowBefore, lc->centerEye_.neo_.showCallCount);
    delete lc;
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    // ServoController connection check
    RUN_TEST(test_servo_starts_not_ready);
    RUN_TEST(test_checkConnection_sends_getErrors_command);
    RUN_TEST(test_checkConnection_returns_false_when_no_response);
    RUN_TEST(test_checkConnection_returns_true_when_response_available);
    RUN_TEST(test_checkConnection_resets_pending_on_timeout);
    RUN_TEST(test_isReady_stays_true_after_connection);
    // JewelEye pulse
    RUN_TEST(test_pulse_skips_before_100ms);
    RUN_TEST(test_pulse_executes_at_100ms);
    RUN_TEST(test_pulse_increases_value_upward);
    RUN_TEST(test_pulse_reverses_at_top);
    // LightController startup mode
    RUN_TEST(test_lightcontroller_starts_not_maestro_ready);
    RUN_TEST(test_lightcontroller_startup_mode_pulses_eyes);
    RUN_TEST(test_lightcontroller_startup_mode_animates_center_eye);
    RUN_TEST(test_lightcontroller_transitions_on_maestro_ready);
    UNITY_END();
    return 0;
}
