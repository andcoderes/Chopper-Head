#include <unity.h>
#include "lights/LightController.h"

static LightController* lc;

void setUp(void) {
    lc = new LightController();
    lc->setup();
    lc->setMaestroReady(true);
    mock_millis_value = 0;
}

void tearDown(void) {
    delete lc;
}

// --- getDuration lookup ---

void test_getDuration_101_returns_2450(void) {
    TEST_ASSERT_EQUAL_UINT32(2450, lc->getDuration(101));
}

void test_getDuration_103_returns_15500(void) {
    TEST_ASSERT_EQUAL_UINT32(15500, lc->getDuration(103));
}

void test_getDuration_107_returns_8300(void) {
    TEST_ASSERT_EQUAL_UINT32(8300, lc->getDuration(107));
}

void test_getDuration_unknown_returns_0(void) {
    TEST_ASSERT_EQUAL_UINT32(0, lc->getDuration(999));
}

// --- animate macro routing ---

void test_animate_macro_101_sets_redEye(void) {
    const int16_t macros[] = {101, 0, 0, 0};
    mock_millis_value = 1000;
    lc->animate("", macros, 4);
    TEST_ASSERT_EQUAL_STRING("101", lc->currentRoutine_.c_str());
    TEST_ASSERT_EQUAL_UINT32(2450, lc->limitTime_);
}

void test_animate_macro_103_turns_on_periscope(void) {
    const int16_t macros[] = {103, 0, 0, 0};
    mock_millis_value = 1000;
    lc->animate("", macros, 4);
    TEST_ASSERT_EQUAL_STRING("103", lc->currentRoutine_.c_str());
    TEST_ASSERT_TRUE(lc->periscope_.isOn_);
}

void test_animate_macro_107_sets_routine(void) {
    const int16_t macros[] = {107, 0, 0, 0};
    mock_millis_value = 1000;
    lc->animate("", macros, 4);
    TEST_ASSERT_EQUAL_STRING("107", lc->currentRoutine_.c_str());
    TEST_ASSERT_EQUAL_UINT32(8300, lc->limitTime_);
}

// --- animate button routing ---

void test_animate_button_du_turns_on_periscope(void) {
    const int16_t macros[] = {0, 0, 0, 0};
    lc->animate("du", macros, 0);
    TEST_ASSERT_TRUE(lc->periscope_.isOn_);
}

void test_animate_button_dd_turns_off_periscope(void) {
    const int16_t macros[] = {0, 0, 0, 0};
    lc->animate("du", macros, 0); // on first
    lc->animate("dd", macros, 0);
    TEST_ASSERT_FALSE(lc->periscope_.isOn_);
}

void test_animate_button_dl_turns_on_periscope(void) {
    const int16_t macros[] = {0, 0, 0, 0};
    lc->animate("dl", macros, 0);
    TEST_ASSERT_TRUE(lc->periscope_.isOn_);
}

void test_animate_button_dr_turns_on_periscope(void) {
    const int16_t macros[] = {0, 0, 0, 0};
    lc->animate("dr", macros, 0);
    TEST_ASSERT_TRUE(lc->periscope_.isOn_);
}

// --- standBy timeout reset ---

void test_standBy_resets_routine_after_timeout(void) {
    const int16_t macros[] = {101, 0, 0, 0};
    mock_millis_value = 1000;
    lc->animate("", macros, 4);
    TEST_ASSERT_EQUAL_STRING("101", lc->currentRoutine_.c_str());

    // Advance past the duration (2450ms)
    mock_millis_value = 1000 + 2451;
    lc->standBy();
    TEST_ASSERT_EQUAL_STRING("", lc->currentRoutine_.c_str());
}

void test_standBy_does_not_reset_before_timeout(void) {
    const int16_t macros[] = {101, 0, 0, 0};
    mock_millis_value = 1000;
    lc->animate("", macros, 4);

    // Just before timeout
    mock_millis_value = 1000 + 2400;
    lc->standBy();
    TEST_ASSERT_EQUAL_STRING("101", lc->currentRoutine_.c_str());
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_getDuration_101_returns_2450);
    RUN_TEST(test_getDuration_103_returns_15500);
    RUN_TEST(test_getDuration_107_returns_8300);
    RUN_TEST(test_getDuration_unknown_returns_0);
    RUN_TEST(test_animate_macro_101_sets_redEye);
    RUN_TEST(test_animate_macro_103_turns_on_periscope);
    RUN_TEST(test_animate_macro_107_sets_routine);
    RUN_TEST(test_animate_button_du_turns_on_periscope);
    RUN_TEST(test_animate_button_dd_turns_off_periscope);
    RUN_TEST(test_animate_button_dl_turns_on_periscope);
    RUN_TEST(test_animate_button_dr_turns_on_periscope);
    RUN_TEST(test_standBy_resets_routine_after_timeout);
    RUN_TEST(test_standBy_does_not_reset_before_timeout);
    UNITY_END();
    return 0;
}
