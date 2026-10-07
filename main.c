#include "smart_flowers.h"
#include "logger.h"

#include <unistd.h>   // для write() в обработчике сигнала

// ============================================
//  Параметры логирования
// ============================================
static const char* LOG_DIR    = "logs";
static const char* LOG_PREFIX = "simulation";

// ============================================
//  Флаг завершения по сигналу
// ============================================
volatile sig_atomic_t stop = 0;

// ============================================
//  Обработчик SIGINT (Ctrl+C)
//  Используем write() — асинхронно-безопасен в обработчике.
// ============================================
void handle_sigint(int sig) {
    (void)sig;
    const char msg[] = "\nПолучен сигнал прерывания. Завершение...\n";
    ssize_t w = write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    (void)w;
    stop = 1;
}

// ============================================
//  Вспомогательная функция: безопасный ввод int
// ============================================
static int read_int(const char* prompt, int default_value) {
    char buf[64];
    log_msg("%s [%d]: ", prompt, default_value);
    if (fgets(buf, sizeof(buf), stdin) == NULL) return default_value;

    // Пустая строка — значение по умолчанию
    if (buf[0] == '\n' || buf[0] == '\0') return default_value;

    int v = atoi(buf);
    return (v > 0) ? v : default_value;
}

// ============================================
//  Вспомогательная функция: безопасный ввод double
// ============================================
static double read_double(const char* prompt, double default_value) {
    char buf[64];
    log_msg("%s [%.1f]: ", prompt, default_value);
    if (fgets(buf, sizeof(buf), stdin) == NULL) return default_value;
    if (buf[0] == '\n' || buf[0] == '\0') return default_value;

    double v = atof(buf);
    return (v > 0.0) ? v : default_value;
}

// ============================================
//  Интерактивный ввод параметров
// ============================================
static void read_params_interactive(void) {
    log_msg("\n=== Настройка параметров симуляции ===\n");
    log_msg("(нажмите Enter, чтобы оставить значение по умолчанию)\n\n");

    N_FLOWERS           = read_int   ("Количество цветов",              N_FLOWERS);
    N_GARDENERS         = read_int   ("Количество садовников",          N_GARDENERS);
    SIMULATION_TIME     = read_int   ("Длительность симуляции (такты)", SIMULATION_TIME);
    DRYING_RATE_MIN     = read_double("Мин. скорость высыхания",         DRYING_RATE_MIN);
    DRYING_RATE_MAX     = read_double("Макс. скорость высыхания",        DRYING_RATE_MAX);
    REQUEST_THRESHOLD   = read_double("Порог создания заявки",           REQUEST_THRESHOLD);
    DRY_THRESHOLD       = read_double("Порог засыхания",                 DRY_THRESHOLD);
    WATERING_DURATION   = read_int   ("Длительность полива",             WATERING_DURATION);
    RECOVERY_DURATION   = read_int   ("Длительность восстановления",     RECOVERY_DURATION);
    PROTECTION_INTERVAL = read_int   ("Защитный интервал (перелив)",     PROTECTION_INTERVAL);
    log_msg("\n");
}

// ============================================
//  Точка входа
// ============================================
int main(int argc, char** argv) {
    signal(SIGINT, handle_sigint);
    srand((unsigned int)time(NULL));

    // --- Инициализация логирования ---
    if (log_init(LOG_DIR, LOG_PREFIX) != 0) {
        fprintf(stderr, "Предупреждение: лог-файл недоступен, вывод только в консоль.\n");
    }
    const char* log_path = log_get_path();
    if (log_path != NULL) {
        log_msg("Лог-файл: %s\n", log_path);
    }

    // --- Режим 1: аргументы командной строки (быстрый запуск) ---
    if (argc > 1) {
        if (argc > 1) N_FLOWERS       = atoi(argv[1]);
        if (argc > 2) N_GARDENERS     = atoi(argv[2]);
        if (argc > 3) SIMULATION_TIME = atoi(argv[3]);
        log_msg("Параметры заданы через командную строку.\n");
    }
    // --- Режим 2: интерактивный ввод ---
    else {
        read_params_interactive();
    }

    // --- Выделение памяти ---
    Flower*   flowers   = (Flower*)malloc((size_t)N_FLOWERS   * sizeof(Flower));
    Gardener* gardeners = (Gardener*)malloc((size_t)N_GARDENERS * sizeof(Gardener));

    if (flowers == NULL || gardeners == NULL) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        free(flowers);
        free(gardeners);
        log_close();
        return 1;
    }

    init_flowers(flowers, N_FLOWERS);
    init_gardeners(gardeners, N_GARDENERS);

    // --- Заголовок симуляции ---
    log_msg("=== Симуляция умных цветов ===\n");
    log_msg("Цветов: %d, Садовников: %d, Время: %d тактов\n",
            N_FLOWERS, N_GARDENERS, SIMULATION_TIME);
    log_msg("Скорость высыхания: [%.1f .. %.1f]\n",
            DRYING_RATE_MIN, DRYING_RATE_MAX);
    log_msg("Порог заявки: %.1f, Порог засыхания: %.1f\n",
            REQUEST_THRESHOLD, DRY_THRESHOLD);
    log_msg("Полив: %d, Восстановление: %d, Защитный интервал: %d\n",
            WATERING_DURATION, RECOVERY_DURATION, PROTECTION_INTERVAL);
    log_msg("----------------------------------------\n");

    // ============================================
    //  Основной цикл симуляции
    // ============================================
    for (int t = 0; t < SIMULATION_TIME && !stop; t++) {
        log_msg("\n--- Такт %d ---\n", t);

        // ----------------------------------------
        //  Фаза 1. Обновление состояния цветов
        // ----------------------------------------
        for (int i = 0; i < N_FLOWERS; i++) {
            Flower* f = &flowers[i];
            if (f->state == DEAD) continue;

            // --- Полив ---
            if (f->state == WATERING) {
                f->state_timer--;
                if (f->state_timer <= 0) {
                    f->state = RECOVERING;
                    f->state_timer = RECOVERY_DURATION;
                    log_msg("  Цветок %d: полив завершён, переход в восстановление\n", f->id);
                }
                continue;
            }

            // --- Восстановление ---
            if (f->state == RECOVERING) {
                f->state_timer--;
                if (f->state_timer <= 0) {
                    f->state = NORMAL;
                    log_msg("  Цветок %d: восстановление завершено, нормальное состояние\n", f->id);
                }
                continue;
            }

            // --- Обычная жизнь: высыхание, пороги, заявки ---
            if (f->state == NORMAL || f->state == WILTING || f->state == DRYING) {
                f->moisture -= f->drying_rate;
                if (f->moisture < 0) f->moisture = 0;

                if (f->moisture <= 0 && f->state != DEAD) {
                    f->state = DEAD;
                    f->has_request = 0;
                    log_msg("  Цветок %d: погиб от засухи (влажность 0)\n", f->id);
                    continue;
                }

                if (f->moisture <= DRY_THRESHOLD && f->state != DRYING) {
                    f->state = DRYING;
                    log_msg("  Цветок %d: начал засыхать (влажность %.1f)\n",
                            f->id, f->moisture);
                }

                if (f->moisture <= REQUEST_THRESHOLD && f->state == NORMAL) {
                    f->state = WILTING;
                    f->has_request = 1;
                    f->request_time = t;
                    log_msg("  Цветок %d: увядает, создана заявка (влажность %.1f)\n",
                            f->id, f->moisture);
                } else if (f->moisture > REQUEST_THRESHOLD && f->state == WILTING) {
                    f->state = NORMAL;
                    f->has_request = 0;
                    log_msg("  Цветок %d: восстановился, заявка снята (влажность %.1f)\n",
                            f->id, f->moisture);
                }
            }
        }

        // ----------------------------------------
        //  Фаза 2. Обновление садовников
        // ----------------------------------------
        for (int i = 0; i < N_GARDENERS; i++) {
            Gardener* g = &gardeners[i];

            if (g->is_busy) {
                // Садовник поливает — уменьшаем таймер
                g->busy_timer--;
                if (g->busy_timer <= 0) {
                    Flower* f = &flowers[g->current_flower_id];
                    f->last_water_time = t;
                    f->state = RECOVERING;
                    f->state_timer = RECOVERY_DURATION;
                    f->moisture = 100.0;
                    g->is_busy = 0;
                    g->current_flower_id = -1;
                    log_msg("  Садовник %d: закончил полив цветка %d\n",
                            g->id, f->id);
                }
            } else {
                // Свободен — ищем заявку
                int flower_id = find_request(flowers, N_FLOWERS);
                if (flower_id != -1) {
                    Flower* f = &flowers[flower_id];

                    // Проверка защитного интервала (перелив)
                    if (t - f->last_water_time < PROTECTION_INTERVAL) {
                        f->state = DEAD;
                        f->has_request = 0;
                        log_msg("  Садовник %d: обнаружил перелив цветка %d! Цветок погиб.\n",
                                g->id, f->id);
                    } else {
                        // Начинаем полив
                        f->has_request = 0;
                        f->state = WATERING;
                        f->state_timer = WATERING_DURATION;
                        g->is_busy = 1;
                        g->current_flower_id = f->id;
                        g->busy_timer = WATERING_DURATION;
                        log_msg("  Садовник %d: начал полив цветка %d\n",
                                g->id, f->id);
                    }
                }
            }
        }

        // ----------------------------------------
        //  Краткая сводка состояния всех цветов
        // ----------------------------------------
        log_msg("  Состояние цветов: ");
        for (int i = 0; i < N_FLOWERS; i++) {
            log_msg("%d:%s(%.0f) ",
                    flowers[i].id,
                    state_name(flowers[i].state),
                    flowers[i].moisture);
        }
        log_msg("\n");
    }

    // ============================================
    //  Итоговая статистика
    // ============================================
    log_msg("\n=== Завершение симуляции ===\n");
    int alive = 0, dead = 0, dry = 0;
    for (int i = 0; i < N_FLOWERS; i++) {
        if (flowers[i].state == DEAD)        dead++;
        else if (flowers[i].state == DRYING) dry++;
        else                                 alive++;
    }
    log_msg("Живых цветов: %d, Засыхающих: %d, Погибших: %d\n",
            alive, dry, dead);

    // ============================================
    //  Освобождение ресурсов
    // ============================================
    free(flowers);
    free(gardeners);

    if (log_path != NULL) {
        log_msg("Лог сохранён в %s\n", log_path);
    }
    log_close();

    return 0;
}