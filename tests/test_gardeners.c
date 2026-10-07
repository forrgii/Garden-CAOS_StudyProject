#include "test_framework.h"
#include "smart_flowers.h"

TEST(test_init_gardeners) {
    const int n = 4;
    Gardener g[4];
    init_gardeners(g, n);
    for (int i = 0; i < n; i++) {
        ASSERT_EQ_INT(i,  g[i].id);
        ASSERT_EQ_INT(0,  g[i].is_busy);
        ASSERT_EQ_INT(-1, g[i].current_flower_id);
        ASSERT_EQ_INT(0,  g[i].busy_timer);
    }
}

void run_gardeners_tests(void) {
    printf("\n-- init_gardeners --\n");
    RUN_TEST(test_init_gardeners);
}
