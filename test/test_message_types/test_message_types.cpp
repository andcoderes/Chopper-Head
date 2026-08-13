#include <unity.h>
#include "communication/MessageTypes.h"

void setUp(void) {}
void tearDown(void) {}

// --- Struct sizes ---
void test_HeadCommand_size_is_packed(void) {
    // int8_t(1) + int8_t(1) + char[8](8) + int16_t[4](8) + uint8_t(1) + uint8_t(1) + int8_t(1) = 21
    TEST_ASSERT_EQUAL(21, sizeof(HeadCommand));
}

void test_HeadTelemetry_size_is_packed(void) {
    // int8_t(1) + uint8_t(1) + uint8_t(1) + uint8_t(1) + uint32_t(4) = 8
    TEST_ASSERT_EQUAL(8, sizeof(HeadTelemetry));
}

// --- Field copy ---
void test_HeadCommand_field_copy(void) {
    HeadCommand cmd = {};
    cmd.msgType = 1;
    cmd.status  = STATUS_BUTTONS;
    strncpy(cmd.button, "du", sizeof(cmd.button));
    cmd.macro[0] = 101;
    cmd.macro[1] = 103;
    cmd.volume   = 42;
    cmd.idleMode = 1;
    cmd.connectionStatus = 1;

    HeadCommand copy;
    memcpy(&copy, &cmd, sizeof(HeadCommand));

    TEST_ASSERT_EQUAL_INT8(1, copy.msgType);
    TEST_ASSERT_EQUAL_INT8(STATUS_BUTTONS, copy.status);
    TEST_ASSERT_EQUAL_STRING("du", copy.button);
    TEST_ASSERT_EQUAL_INT16(101, copy.macro[0]);
    TEST_ASSERT_EQUAL_INT16(103, copy.macro[1]);
    TEST_ASSERT_EQUAL_UINT8(42, copy.volume);
    TEST_ASSERT_EQUAL_UINT8(1, copy.idleMode);
    TEST_ASSERT_EQUAL_INT8(1, copy.connectionStatus);
}

void test_HeadTelemetry_field_copy(void) {
    HeadTelemetry t = {};
    t.msgType = 2;
    t.connected = 1;
    t.animationRunning = 0;
    t.uptimeMs = 123456;

    HeadTelemetry copy;
    memcpy(&copy, &t, sizeof(HeadTelemetry));

    TEST_ASSERT_EQUAL_INT8(2, copy.msgType);
    TEST_ASSERT_EQUAL_UINT8(1, copy.connected);
    TEST_ASSERT_EQUAL_UINT8(0, copy.animationRunning);
    TEST_ASSERT_EQUAL_UINT32(123456, copy.uptimeMs);
}

// --- Status constants ---
void test_status_constants_have_expected_values(void) {
    TEST_ASSERT_EQUAL(0, STATUS_BUTTONS);
    TEST_ASSERT_EQUAL(-1, STATUS_CONNECTION);
    TEST_ASSERT_EQUAL(2, STATUS_SETTINGS);
    TEST_ASSERT_EQUAL(99, STATUS_DEBUG);
}

void test_HeadCommand_button_max_length(void) {
    HeadCommand cmd = {};
    strncpy(cmd.button, "1234567", sizeof(cmd.button));
    // 7 chars + null terminator fits in char[8]
    TEST_ASSERT_EQUAL_STRING("1234567", cmd.button);
}

void test_HeadCommand_macro_array_holds_4(void) {
    HeadCommand cmd = {};
    cmd.macro[0] = 101;
    cmd.macro[1] = 103;
    cmd.macro[2] = 107;
    cmd.macro[3] = 200;
    TEST_ASSERT_EQUAL_INT16(101, cmd.macro[0]);
    TEST_ASSERT_EQUAL_INT16(200, cmd.macro[3]);
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_HeadCommand_size_is_packed);
    RUN_TEST(test_HeadTelemetry_size_is_packed);
    RUN_TEST(test_HeadCommand_field_copy);
    RUN_TEST(test_HeadTelemetry_field_copy);
    RUN_TEST(test_status_constants_have_expected_values);
    RUN_TEST(test_HeadCommand_button_max_length);
    RUN_TEST(test_HeadCommand_macro_array_holds_4);
    UNITY_END();
    return 0;
}
