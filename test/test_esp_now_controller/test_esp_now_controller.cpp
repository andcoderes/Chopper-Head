#include <unity.h>
#include "communication/EspNowController.h"

static EspNowController* enc;

void setUp(void) {
    enc = new EspNowController();
    mock_millis_value = 0;
    // Reset static state
    EspNowController::dataReady_ = false;
    EspNowController::lastRecvTime_ = 0;
    EspNowController::everReceived_ = false;
    EspNowController::incomingBuffer_ = {};
}

void tearDown(void) {
    delete enc;
}

// --- onDataRecv defensive null-termination ---

void test_onDataRecv_forces_null_terminated_button(void) {
    HeadCommand raw = {};
    raw.msgType = 1;
    raw.status = STATUS_BUTTONS;
    memset(raw.button, 'A', sizeof(raw.button)); // fill all 8 bytes, no NUL

    esp_now_recv_info_t info = {};
    EspNowController::onDataRecv(&info, (const uint8_t*)&raw, sizeof(HeadCommand));

    TEST_ASSERT_EQUAL('\0', EspNowController::incomingBuffer_.button[7]);
}

// --- isConnected timeout logic ---

void test_isConnected_false_when_never_received(void) {
    mock_millis_value = 6000;
    TEST_ASSERT_FALSE(enc->isConnected());
}

void test_isConnected_false_at_boot_before_any_packet(void) {
    // Regression: lastRecvTime_ defaults to 0, so early after boot
    // (millis() still small) a naive timeout check alone would read as
    // "connected" even though no packet has ever arrived.
    mock_millis_value = 100; // well within CONNECTION_TIMEOUT_MS
    TEST_ASSERT_FALSE(enc->isConnected());
}

void test_isConnected_true_within_timeout(void) {
    EspNowController::everReceived_ = true;
    EspNowController::lastRecvTime_ = 1000;
    mock_millis_value = 5999; // 4999ms since last recv, < 5000
    TEST_ASSERT_TRUE(enc->isConnected());
}

void test_isConnected_false_at_exact_boundary(void) {
    EspNowController::everReceived_ = true;
    EspNowController::lastRecvTime_ = 1000;
    mock_millis_value = 6000; // exactly 5000ms since last recv
    TEST_ASSERT_FALSE(enc->isConnected());
}

void test_isConnected_false_after_timeout(void) {
    EspNowController::everReceived_ = true;
    EspNowController::lastRecvTime_ = 1000;
    mock_millis_value = 7000; // 6000ms since last recv
    TEST_ASSERT_FALSE(enc->isConnected());
}

void test_isConnected_true_just_received(void) {
    EspNowController::everReceived_ = true;
    EspNowController::lastRecvTime_ = 5000;
    mock_millis_value = 5000; // 0ms since last recv
    TEST_ASSERT_TRUE(enc->isConnected());
}

// --- loop callback dispatch ---

static HeadCommand lastDispatchedCmd;
static int callbackCallCount = 0;

void testCallback(const HeadCommand& cmd) {
    memcpy(&lastDispatchedCmd, &cmd, sizeof(HeadCommand));
    callbackCallCount++;
}

void test_loop_dispatches_callback_when_data_ready(void) {
    callbackCallCount = 0;
    enc->setCommandCallback(testCallback);

    // Simulate data received
    HeadCommand fakeCmd = {};
    fakeCmd.msgType = 1;
    fakeCmd.status = STATUS_BUTTONS;
    strncpy(fakeCmd.button, "y", sizeof(fakeCmd.button));
    memcpy((void*)&EspNowController::incomingBuffer_, &fakeCmd, sizeof(HeadCommand));
    EspNowController::dataReady_ = true;
    EspNowController::lastRecvTime_ = 0;

    mock_millis_value = 100;
    enc->loop();

    TEST_ASSERT_EQUAL(1, callbackCallCount);
    TEST_ASSERT_EQUAL_INT8(1, lastDispatchedCmd.msgType);
    TEST_ASSERT_EQUAL_STRING("y", lastDispatchedCmd.button);
}

void test_loop_does_not_dispatch_when_no_data(void) {
    callbackCallCount = 0;
    enc->setCommandCallback(testCallback);
    EspNowController::dataReady_ = false;
    mock_millis_value = 100;
    enc->loop();
    TEST_ASSERT_EQUAL(0, callbackCallCount);
}

// --- no crash without callback ---

void test_loop_no_crash_without_callback(void) {
    EspNowController::dataReady_ = true;
    HeadCommand fakeCmd = {};
    fakeCmd.msgType = 1;
    memcpy((void*)&EspNowController::incomingBuffer_, &fakeCmd, sizeof(HeadCommand));
    mock_millis_value = 100;
    // Should not crash even without callback set
    enc->loop();
    TEST_ASSERT_FALSE(EspNowController::dataReady_);
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_onDataRecv_forces_null_terminated_button);
    RUN_TEST(test_isConnected_false_when_never_received);
    RUN_TEST(test_isConnected_false_at_boot_before_any_packet);
    RUN_TEST(test_isConnected_true_within_timeout);
    RUN_TEST(test_isConnected_false_at_exact_boundary);
    RUN_TEST(test_isConnected_false_after_timeout);
    RUN_TEST(test_isConnected_true_just_received);
    RUN_TEST(test_loop_dispatches_callback_when_data_ready);
    RUN_TEST(test_loop_does_not_dispatch_when_no_data);
    RUN_TEST(test_loop_no_crash_without_callback);
    UNITY_END();
    return 0;
}
