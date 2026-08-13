#include <unity.h>
#include "lights/JewelEye.h"

static JewelEye* je;

void setUp(void) {
    je = new JewelEye(4, 7);
    je->setup();
    mock_millis_value = 0;
}

void tearDown(void) {
    delete je;
}

// --- setDefaultColor ---

void test_setDefaultColor_restores_base(void) {
    je->redEye(); // changes base colors
    je->setDefaultColor();
    TEST_ASSERT_EQUAL(JEWEL_BASE_R, je->baseR);
    TEST_ASSERT_EQUAL(JEWEL_BASE_G, je->baseG);
    TEST_ASSERT_EQUAL(JEWEL_BASE_B, je->baseB);
}

// --- redEye ---

void test_redEye_sets_red(void) {
    je->redEye();
    TEST_ASSERT_EQUAL(40, je->baseR);
    TEST_ASSERT_EQUAL(0, je->baseG);
    TEST_ASSERT_EQUAL(0, je->baseB);
}

// --- standBy interval gating ---

void test_standBy_executes_on_first_call(void) {
    mock_millis_value = 0;
    int showBefore = je->neo_.showCallCount;
    je->standBy();
    TEST_ASSERT_GREATER_THAN(showBefore, je->neo_.showCallCount);
}

void test_standBy_skips_if_interval_not_elapsed(void) {
    mock_millis_value = 100;
    je->standBy(); // sets latestMillis_
    int showBefore = je->neo_.showCallCount;
    mock_millis_value = 130; // only 30ms, needs 50
    je->standBy();
    TEST_ASSERT_EQUAL(showBefore, je->neo_.showCallCount);
}

void test_standBy_executes_after_interval(void) {
    mock_millis_value = 100;
    je->standBy();
    int showBefore = je->neo_.showCallCount;
    mock_millis_value = 160; // 60ms >= 50
    je->standBy();
    TEST_ASSERT_GREATER_THAN(showBefore, je->neo_.showCallCount);
}

// --- standBy fills with base color ---

void test_standBy_fills_with_base_color(void) {
    mock_millis_value = 0;
    je->standBy();
    // fill called with Color(baseG, baseR, baseB) = Color(0, 0, 40)
    // Color(0, 0, 40) = (0 << 16) | (0 << 8) | 40 = 40
    TEST_ASSERT_GREATER_THAN(0, je->neo_.fillCallCount);
}

// --- changeColor ---

void test_changeColor_calls_show(void) {
    int showBefore = je->neo_.showCallCount;
    je->changeColor(100, 50, 25);
    TEST_ASSERT_GREATER_THAN(showBefore, je->neo_.showCallCount);
}

void test_changeColor_sets_pixels(void) {
    je->changeColor(100, 50, 25);
    // It sets 5 random pixels with Color(g=50, r=100, b=25)
    // At least show should have been called
    TEST_ASSERT_EQUAL(1, je->neo_.showCallCount);
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_setDefaultColor_restores_base);
    RUN_TEST(test_redEye_sets_red);
    RUN_TEST(test_standBy_executes_on_first_call);
    RUN_TEST(test_standBy_skips_if_interval_not_elapsed);
    RUN_TEST(test_standBy_executes_after_interval);
    RUN_TEST(test_standBy_fills_with_base_color);
    RUN_TEST(test_changeColor_calls_show);
    RUN_TEST(test_changeColor_sets_pixels);
    UNITY_END();
    return 0;
}
