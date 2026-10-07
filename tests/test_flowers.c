#include "test_framework.h"
#include "smart_flowers.h"

TEST(test_init_flowers_count) {
    const int n = 5;
    Flower f[5];
    init_flowers(f, n);
    for (int i = 0; i < n; i++) {
        ASSERT_EQ_INT(i, f[i].id);
    }
}

TEST(test_init_flowers_initial_state) {
    Flower f[3];
    init_flowers(f, 3);
    for (int i = 0; i < 3; i++) {
        ASSERT_EQ_INT(NORMAL, f[i].state);
        ASSERT_EQ_INT(0, f[i].has_request);
        ASSERT_EQ_INT(0, f[i].state_timer);
        ASSERT_EQ_INT(0, f[i].request_time);
        ASSERT_EQ_DOUBLE(100.0, f[i].moisture, 0.001);
        ASSERT_TRUE(f[i].drying_rate >= DRYING_RATE_MIN);
        ASSERT_TRUE(f[i].drying_rate <= DRYING_RATE_MAX);
    }
}

TEST(test_init_flowers_last_water_time) {
    Flower f[2];
    init_flowers(f, 2);
    for (int i = 0; i < 2; i++) {
        ASSERT_EQ_INT(-PROTECTION_INTERVAL, f[i].last_water_time);
    }
}

void run_flowers_tests(void) {
    printf("\n-- init_flowers --\n");
    RUN_TEST(test_init_flowers_count);
    RUN_TEST(test_init_flowers_initial_state);
    RUN_TEST(test_init_flowers_last_water_time);
}
