#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>

// Глобальные счётчики (определены в test_framework.c)
extern int tests_run;
extern int tests_passed;
extern int tests_failed;

#define COLOR_RED   "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_RESET "\033[0m"

#define TEST(name) static void name(void)

// Запуск теста с автоматической пометкой OK/FAIL
#define RUN_TEST(name) do {                                  \
    int _before_failed = tests_failed;                       \
    tests_run++;                                             \
    printf("  [TEST] %-45s ", #name);                        \
    fflush(stdout);                                          \
    name();                                                  \
    if (tests_failed > _before_failed) {                     \
        /* FAIL уже напечатан внутри ASSERT */               \
    } else {                                                 \
        tests_passed++;                                      \
        printf(COLOR_GREEN "OK" COLOR_RESET "\n");           \
    }                                                        \
} while (0)

#define ASSERT_TRUE(cond) do {                                    \
    if (!(cond)) {                                                \
        tests_failed++;                                           \
        printf(COLOR_RED "FAIL" COLOR_RESET "\n");                \
        printf("    %s:%d: ASSERT_TRUE(%s)\n",                    \
               __FILE__, __LINE__, #cond);                        \
        return;                                                   \
    }                                                             \
} while (0)

#define ASSERT_EQ_INT(expected, actual) do {                      \
    int _e = (int)(expected);                                     \
    int _a = (int)(actual);                                       \
    if (_e != _a) {                                               \
        tests_failed++;                                           \
        printf(COLOR_RED "FAIL" COLOR_RESET "\n");                \
        printf("    %s:%d: ожидалось %d, получено %d\n",          \
               __FILE__, __LINE__, _e, _a);                       \
        return;                                                   \
    }                                                             \
} while (0)

#define ASSERT_EQ_DOUBLE(expected, actual, eps) do {              \
    double _e = (expected);                                       \
    double _a = (actual);                                         \
    if (((_e) - (_a) > (eps)) || ((_a) - (_e) > (eps))) {         \
        tests_failed++;                                           \
        printf(COLOR_RED "FAIL" COLOR_RESET "\n");                \
        printf("    %s:%d: ожидалось %.4f, получено %.4f\n",      \
               __FILE__, __LINE__, _e, _a);                       \
        return;                                                   \
    }                                                             \
} while (0)

#define ASSERT_EQ_STR(expected, actual) do {                      \
    if (strcmp((expected), (actual)) != 0) {                      \
        tests_failed++;                                           \
        printf(COLOR_RED "FAIL" COLOR_RESET "\n");                \
        printf("    %s:%d: ожидалось \"%s\", получено \"%s\"\n",  \
               __FILE__, __LINE__, (expected), (actual));         \
        return;                                                   \
    }                                                             \
} while (0)

#endif
