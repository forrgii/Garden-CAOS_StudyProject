#include "smart_flowers.h"
#include "logger.h"
#include <unistd.h>

static const char* LOG_DIR    = "logs";
static const char* LOG_PREFIX = "simulation";

void handle_sigint(int sig) {
    (void)sig;
    const char msg[] = "\nПрерывание...\n";
    ssize_t w = write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    (void)w;
    stop = 1;
}

static int read_int(const char* prompt, int default_value) {
    char buf[64];
    log_msg("%s [%d]: ", prompt, default_value);
    if (fgets(buf, sizeof(buf), stdin) == NULL) return default_value;
    if (buf[0] == '\n' || buf[0] == '\0') return default_value;
    int v = atoi(buf);
    return (v > 0) ? v : default_value;
}

static void read_params_interactive(void) {
    log_msg("\n=== Настройка параметров ===\n");
    N_FLOWERS           = read_int("Цветов", N_FLOWERS);
    N_GARDENERS         = read_int("Садовников", N_GARDENERS);
    SIMULATION_TIME     = read_int("Время (такты)", SIMULATION_TIME);
    log_msg("\n");
}

int main(int argc, char** argv) {
    signal(SIGINT, handle_sigint);
    srand((unsigned int)time(NULL));

    if (log_init(LOG_DIR, LOG_PREFIX) != 0) {
        fprintf(stderr, "Ошибка лога\n");
    }

    if (argc > 1) {
        N_FLOWERS = atoi(argv[1]);
        if (argc > 2) N_GARDENERS = atoi(argv[2]);
        if (argc > 3) SIMULATION_TIME = atoi(argv[3]);
    } else {
        read_params_interactive();
    }

    flowers_global = (Flower*)malloc((size_t)N_FLOWERS * sizeof(Flower));
    Gardener* gardeners = (Gardener*)malloc((size_t)N_GARDENERS * sizeof(Gardener));

    if (!flowers_global || !gardeners) {
        fprintf(stderr, "Ошибка malloc\n");
        return 1;
    }

    init_flowers(flowers_global, N_FLOWERS);
    init_gardeners(gardeners, N_GARDENERS);
    sem_init(&request_sem, 0, 0);

    log_msg("=== Многопоточная симуляция ===\n");
    log_msg("Цветов: %d, Садовников: %d, Время: %d тактов\n", N_FLOWERS, N_GARDENERS, SIMULATION_TIME);
    log_msg("--------------------------------\n");

    for (int i = 0; i < N_FLOWERS; i++) {
        pthread_create(&flowers_global[i].thread, NULL, flower_thread, &flowers_global[i]);
    }

    for (int i = 0; i < N_GARDENERS; i++) {
        pthread_create(&gardeners[i].thread, NULL, gardener_thread, &gardeners[i]);
    }

    for (int t = 0; t < SIMULATION_TIME && !stop; t++) {
        current_tick = t;
        usleep(TICK_MS * 1000);
    }

    stop = 1;
    for (int i = 0; i < N_GARDENERS; i++) {
        sem_post(&request_sem);
    }

    for (int i = 0; i < N_FLOWERS; i++) {
        pthread_join(flowers_global[i].thread, NULL);
    }
    for (int i = 0; i < N_GARDENERS; i++) {
        pthread_join(gardeners[i].thread, NULL);
    }

    log_msg("\n=== Завершение ===\n");
    int alive = 0, dead = 0, dry = 0;
    for (int i = 0; i < N_FLOWERS; i++) {
        if (flowers_global[i].state == DEAD) dead++;
        else if (flowers_global[i].state == DRYING) dry++;
        else alive++;
    }
    log_msg("Живых: %d, Засыхающих: %d, Погибших: %d\n", alive, dry, dead);

    sem_destroy(&request_sem);
    free(flowers_global);
    free(gardeners);
    log_close();

    return 0;
}