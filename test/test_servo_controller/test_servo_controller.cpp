#include <unity.h>
#include "servo/ServoController.h"

static ServoController* sc;

void setUp(void) {
    sc = new ServoController();
    mock_millis_value = 0;
}

void tearDown(void) {
    delete sc;
}

// --- checkAnimation state machine ---

// animation 0 (outHandAnimation_) always allowed, sets outComponent_[0]
void test_checkAnimation_arm_out_always_allowed(void) {
    TEST_ASSERT_TRUE(sc->checkAnimation(0));
    TEST_ASSERT_TRUE(sc->outComponent_[0]);
}

// animation 1 (inHandAnimation_) blocked when arms are NOT out
void test_checkAnimation_arm_in_blocked_when_not_out(void) {
    TEST_ASSERT_FALSE(sc->checkAnimation(1));
}

// animation 1 allowed when arms ARE out, clears outComponent_
void test_checkAnimation_arm_in_allowed_when_out(void) {
    sc->checkAnimation(0); // arm out
    TEST_ASSERT_TRUE(sc->checkAnimation(1)); // arm in
    TEST_ASSERT_FALSE(sc->outComponent_[0]);
}

// animations requiring hand out (1,2,3) are gated
void test_checkAnimation_say_hi_requires_arm_out(void) {
    TEST_ASSERT_FALSE(sc->checkAnimation(2)); // blocked
    sc->checkAnimation(0); // arm out
    TEST_ASSERT_TRUE(sc->checkAnimation(2)); // now allowed
}

void test_checkAnimation_say_hi_left_requires_arm_out(void) {
    TEST_ASSERT_FALSE(sc->checkAnimation(3)); // blocked
    sc->checkAnimation(0); // arm out
    TEST_ASSERT_TRUE(sc->checkAnimation(3)); // now allowed
}

// animations > 3 (periscope etc.) are always independent
void test_checkAnimation_periscope_scripts_always_allowed(void) {
    TEST_ASSERT_TRUE(sc->checkAnimation(4));
    TEST_ASSERT_TRUE(sc->checkAnimation(5));
    TEST_ASSERT_TRUE(sc->checkAnimation(6));
}

// --- animate: button routing ---

void test_animate_button_y_first_press_runs_arm_out(void) {
    const int16_t macros[] = {0, 0, 0, 0};
    sc->animate("y", macros, 4);
    TEST_ASSERT_EQUAL(0, sc->maestro_.lastScriptRestarted);
    TEST_ASSERT_EQUAL(1, sc->maestro_.restartCallCount);
}

void test_animate_button_y_toggle_runs_arm_in(void) {
    const int16_t macros[] = {0, 0, 0, 0};
    sc->animate("y", macros, 4); // arm out (script 0)
    sc->animate("y", macros, 4); // arm in  (script 1)
    TEST_ASSERT_EQUAL(1, sc->maestro_.lastScriptRestarted);
    TEST_ASSERT_EQUAL(2, sc->maestro_.restartCallCount);
}

void test_animate_button_du_runs_periscope_out(void) {
    const int16_t macros[] = {0, 0, 0, 0};
    sc->animate("du", macros, 4);
    // du maps to scripts_[3] = {4,4}, first press -> script 4
    TEST_ASSERT_EQUAL(4, sc->maestro_.lastScriptRestarted);
}

void test_animate_button_dd_runs_periscope_in(void) {
    const int16_t macros[] = {0, 0, 0, 0};
    sc->animate("dd", macros, 4);
    TEST_ASSERT_EQUAL(5, sc->maestro_.lastScriptRestarted);
}

// --- animate: macro routing ---
void test_animate_macro_button_empty_uses_macros(void) {
    const int16_t macros[] = {101, 0, 0, 0};
    // macro string "101" doesn't match any key, so no restart
    sc->animate("", macros, 4);
    TEST_ASSERT_EQUAL(0, sc->maestro_.restartCallCount);
}

// --- execute ---
void test_execute_runs_independent_script(void) {
    sc->execute(5);
    TEST_ASSERT_EQUAL(5, sc->maestro_.lastScriptRestarted);
    TEST_ASSERT_EQUAL(1, sc->maestro_.restartCallCount);
}

void test_execute_blocks_arm_dependent_when_not_out(void) {
    sc->execute(2); // requires arm out
    TEST_ASSERT_EQUAL(0, sc->maestro_.restartCallCount);
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(test_checkAnimation_arm_out_always_allowed);
    RUN_TEST(test_checkAnimation_arm_in_blocked_when_not_out);
    RUN_TEST(test_checkAnimation_arm_in_allowed_when_out);
    RUN_TEST(test_checkAnimation_say_hi_requires_arm_out);
    RUN_TEST(test_checkAnimation_say_hi_left_requires_arm_out);
    RUN_TEST(test_checkAnimation_periscope_scripts_always_allowed);
    RUN_TEST(test_animate_button_y_first_press_runs_arm_out);
    RUN_TEST(test_animate_button_y_toggle_runs_arm_in);
    RUN_TEST(test_animate_button_du_runs_periscope_out);
    RUN_TEST(test_animate_button_dd_runs_periscope_in);
    RUN_TEST(test_animate_macro_button_empty_uses_macros);
    RUN_TEST(test_execute_runs_independent_script);
    RUN_TEST(test_execute_blocks_arm_dependent_when_not_out);
    UNITY_END();
    return 0;
}
