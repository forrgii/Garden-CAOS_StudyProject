#include "test_framework.h"
#include "smart_flowers.h"

// Внешние точки входа каждой группы тестов
void run_state_tests(void);
void run_flowers_tests(void);
void run_gardeners_tests(void);
void run_requests_tests(void);

int main(void) {
    // Фиксируем параметры, чтобы тесты были детерминированными
    DRYING_RATE_MIN     = 0.5;
    DRYING_RATE_MAX     = 1.5;
    PROTECTION_INTERVAL = 15;

    printf("\n=== Запуск юнит-тестов ===\n\n");

    run_state_tests();
    run_flowers_tests();
    run_gardeners_tests();
    run_requests_tests();

    printf("\n============================================\n");
    printf("Всего тестов:  %d\n", tests_run);
    printf("Пройдено:      " COLOR_GREEN "%d" COLOR_RESET "\n", tests_passed);
    printf("Провалено:     " COLOR_RED   "%d" COLOR_RESET "\n", tests_failed);
    printf("============================================\n");

    return (tests_failed == 0) ? 0 : 1;
}
