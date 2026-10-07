#include "test_framework.h"
#include "smart_flowers.h"

TEST(test_state_name_all_states) {
    ASSERT_EQ_STR("NORMAL",     state_name(NORMAL));
    ASSERT_EQ_STR("WILTING",    state_name(WILTING));
    ASSERT_EQ_STR("WATERING",   state_name(WATERING));
    ASSERT_EQ_STR("RECOVERING", state_name(RECOVERING));
    ASSERT_EQ_STR("DRYING",     state_name(DRYING));
    ASSERT_EQ_STR("DEAD",       state_name(DEAD));
}

void run_state_tests(void) {
    printf("-- state_name --\n");
    RUN_TEST(test_state_name_all_states);
}
