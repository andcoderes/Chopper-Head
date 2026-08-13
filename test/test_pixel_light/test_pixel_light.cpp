#include <unity.h>
#include "lights/PixelLight.h"

static PixelLight* pl;

void setUp(void) {
    pl = new PixelLight(5);
    pl->setup();
    mock_millis_value = 0;
}

void tearDown(void) {
    delete pl;
}

// --- on / off / swap ---

void test_off_sets_isOn_false(void) {
    pl->off();
    TEST_ASSERT_FALSE(pl->isOn_);
}

void test_on_sets_isOn_true(void) {
    pl->off();
    pl->on();
    TEST_ASSERT_TRUE(pl->isOn_);
}

void test_on_sets_pixel_red(void) {
    pl->on();
    // Color(255,0,0) -> (255 << 16) = 0xFF0000
    TEST_ASSERT_EQUAL_UINT32(0xFF0000, pl->neo_.lastPixelColors[0]);
}

void test_off_sets_pixel_black(void) {
    pl->on();
    pl->off();
    TEST_ASSERT_EQUAL_UINT32(0x000000, pl->neo_.lastPixelColors[0]);
}

void test_swap_toggles_on_to_off(void) {
    pl->on();
    pl->swapLight();
    TEST_ASSERT_FALSE(pl->isOn_);
}

void test_swap_toggles_off_to_on(void) {
    pl->off();
    pl->swapLight();
    TEST_ASSERT_TRUE(pl->isOn_);
}

// --- setBlink ---

void test_setBlink_changes_shouldBlink(void) {
    pl->setBlink(false);
    TEST_ASSERT_FALSE(pl->shouldBlink_);
    pl->setBlink(true);
    TEST_ASSERT_TRUE(pl->shouldBlink_);
}

// --- setConnectionStatus ---

void test_setConnectionStatus_enables_connection_mode(void) {
    pl->setConnectionStatus(true);
    TEST_ASSERT_TRUE(pl->connectionMode_);
    TEST_ASSERT_TRUE(pl->connected_);
    TEST_ASSERT_EQUAL(0, pl->pulseValue_);
}

void test_setConnectionStatus_disconnected(void) {
    pl->setConnectionStatus(false);
    TEST_ASSERT_TRUE(pl->connectionMode_);
    TEST_ASSERT_FALSE(pl->connected_);
}

// --- standBy pulse bounds ---

void test_standBy_pulse_increases_when_disconnected(void) {
    pl->setConnectionStatus(false);
    mock_millis_value = 100;
    pl->standBy();
    TEST_ASSERT_EQUAL(15, pl->pulseValue_);
    TEST_ASSERT_TRUE(pl->pulseUp_);
}

void test_standBy_pulse_caps_at_250(void) {
    pl->setConnectionStatus(false);
    // Pulse up in increments of 15 from 0: after 16 steps reaches 240 -> caps at 250
    for (int i = 1; i <= 16; i++) {
        mock_millis_value = i * 100;
        pl->standBy();
    }
    TEST_ASSERT_EQUAL(250, pl->pulseValue_);
    TEST_ASSERT_FALSE(pl->pulseUp_);
}

void test_standBy_pulse_bottoms_at_5(void) {
    pl->setConnectionStatus(false);
    // Go up to 250 (16 steps)
    for (int i = 1; i <= 16; i++) {
        mock_millis_value = i * 100;
        pl->standBy();
    }
    // Now go down: 250-15=235, 220, 205, ..., 10, then <=5 clamps to 5
    // From 250, need ceil(250/15) = 17 steps but it clamps when <=5
    // 250 -> 235 -> 220 -> ... -> 10 -> -5 => 5 (17 steps down)
    for (int i = 17; i <= 33; i++) {
        mock_millis_value = i * 100;
        pl->standBy();
    }
    TEST_ASSERT_EQUAL(5, pl->pulseValue_);
    TEST_ASSERT_TRUE(pl->pulseUp_);
}

// --- standBy blink logic ---

void test_standBy_normal_mode_calls_show(void) {
    int showBefore = pl->neo_.showCallCount;
    mock_millis_value = 100;
    pl->standBy(); // normal mode (not connectionMode_)
    TEST_ASSERT_GREATER_THAN(showBefore, pl->neo_.showCallCount);
}

void test_standBy_skips_if_interval_not_elapsed(void) {
    mock_millis_value = 100;
    pl->standBy(); // first call sets latestMillis_
    int showBefore = pl->neo_.showCallCount;
    mock_millis_value = 120; // only 20ms later, needs 50
    pl->standBy();
    TEST_ASSERT_EQUAL(showBefore, pl->neo_.showCallCount);
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_off_sets_isOn_false);
    RUN_TEST(test_on_sets_isOn_true);
    RUN_TEST(test_on_sets_pixel_red);
    RUN_TEST(test_off_sets_pixel_black);
    RUN_TEST(test_swap_toggles_on_to_off);
    RUN_TEST(test_swap_toggles_off_to_on);
    RUN_TEST(test_setBlink_changes_shouldBlink);
    RUN_TEST(test_setConnectionStatus_enables_connection_mode);
    RUN_TEST(test_setConnectionStatus_disconnected);
    RUN_TEST(test_standBy_pulse_increases_when_disconnected);
    RUN_TEST(test_standBy_pulse_caps_at_250);
    RUN_TEST(test_standBy_pulse_bottoms_at_5);
    RUN_TEST(test_standBy_normal_mode_calls_show);
    RUN_TEST(test_standBy_skips_if_interval_not_elapsed);
    UNITY_END();
    return 0;
}
