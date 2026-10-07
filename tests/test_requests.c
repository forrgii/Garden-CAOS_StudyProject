#include "test_framework.h"
#include "smart_flowers.h"

TEST(test_find_request_empty) {
    Flower f[3];
    init_flowers(f, 3);
    ASSERT_EQ_INT(-1, find_request(f, 3));
}

TEST(test_find_request_single) {
    Flower f[3];
    init_flowers(f, 3);
    f[1].has_request  = 1;
    f[1].state        = WILTING;
    f[1].request_time = 5;
    ASSERT_EQ_INT(1, find_request(f, 3));
}

TEST(test_find_request_oldest) {
    Flower f[4];
    init_flowers(f, 4);

    f[0].has_request = 1; f[0].state = WILTING; f[0].request_time = 10;
    f[2].has_request = 1; f[2].state = WILTING; f[2].request_time = 3;
    f[3].has_request = 1; f[3].state = WILTING; f[3].request_time = 7;

    ASSERT_EQ_INT(2, find_request(f, 4));
}

TEST(test_find_request_ignores_wrong_state) {
    Flower f[3];
    init_flowers(f, 3);

    f[0].has_request  = 1;
    f[0].state        = NORMAL;
    f[0].request_time = 1;

    f[2].has_request  = 1;
    f[2].state        = WILTING;
    f[2].request_time = 20;

    ASSERT_EQ_INT(2, find_request(f, 3));
}

TEST(test_find_request_ignores_no_request_flag) {
    Flower f[3];
    init_flowers(f, 3);

    f[1].state        = WILTING;
    f[1].has_request  = 0;
    f[1].request_time = 1;

    ASSERT_EQ_INT(-1, find_request(f, 3));
}

void run_requests_tests(void) {
    printf("\n-- find_request --\n");
    RUN_TEST(test_find_request_empty);
    RUN_TEST(test_find_request_single);
    RUN_TEST(test_find_request_oldest);
    RUN_TEST(test_find_request_ignores_wrong_state);
    RUN_TEST(test_find_request_ignores_no_request_flag);
}
