#define _GNU_SOURCE

#include "smart_flowers.h"
#include "logger.h"
#include <unistd.h>

int N_FLOWERS = 10;
int N_GARDENERS = 2;
int SIMULATION_TIME = 200;
double DRYING_RATE_MIN = 0.5;
double DRYING_RATE_MAX = 1.5;
double REQUEST_THRESHOLD = 40.0;
double DRY_THRESHOLD = 20.0;
int WATERING_DURATION = 5;
int RECOVERY_DURATION = 10;
int PROTECTION_INTERVAL = 15;

pthread_mutex_t sim_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t request_sem;
volatile sig_atomic_t stop = 0;
volatile int current_tick = 0;
Flower* flowers_global = NULL;

const char* state_name(FlowerState s) {
    switch(s) {
        case NORMAL: return "NORMAL";
        case WILTING: return "WILTING";
        case WATERING: return "WATERING";
        case RECOVERING: return "RECOVERING";
        case DRYING: return "DRYING";
        case DEAD: return "DEAD";
        default: return "UNKNOWN";
    }
}

void init_flowers(Flower* flowers, int n) {
    for (int i = 0; i < n; i++) {
        flowers[i].id = i;
        flowers[i].moisture = 100.0;
        flowers[i].drying_rate = DRYING_RATE_MIN + (rand() / (double)RAND_MAX) * (DRYING_RATE_MAX - DRYING_RATE_MIN);
        flowers[i].state = NORMAL;
        flowers[i].has_request = 0;
        flowers[i].request_time = 0;
        flowers[i].state_timer = 0;
        flowers[i].last_water_time = -PROTECTION_INTERVAL;
    }
}

void init_gardeners(Gardener* gardeners, int n) {
    for (int i = 0; i < n; i++) {
        gardeners[i].id = i;
        gardeners[i].is_busy = 0;
        gardeners[i].current_flower_id = -1;
    }
}

int find_request(Flower* flowers, int n_flowers) {
    int best_id = -1;
    int best_time = 0;
    for (int i = 0; i < n_flowers; i++) {
        if (flowers[i].has_request && flowers[i].state == WILTING) {
            if (best_id == -1 || flowers[i].request_time < best_time) {
                best_id = i;
                best_time = flowers[i].request_time;
            }
        }
    }
    return best_id;
}

void* flower_thread(void* arg) {
    Flower* f = (Flower*)arg;

    while (!stop && f->state != DEAD) {
        usleep(TICK_MS * 1000);

        pthread_mutex_lock(&sim_mutex);
        if (f->state == DEAD) {
            pthread_mutex_unlock(&sim_mutex);
            break;
        }

        if (f->state == WATERING) {
            pthread_mutex_unlock(&sim_mutex);
            continue;
        }

        if (f->state == RECOVERING) {
            f->state_timer--;
            if (f->state_timer <= 0) {
                f->state = NORMAL;
                log_msg("Цветок %d: восстановление завершено\n", f->id);
            }
            pthread_mutex_unlock(&sim_mutex);
            continue;
        }

        f->moisture -= f->drying_rate;
        if (f->moisture < 0) f->moisture = 0;

        if (f->moisture <= 0) {
            f->state = DEAD;
            f->has_request = 0;
            log_msg("Цветок %d: погиб от засухи\n", f->id);
        } else if (f->moisture <= DRY_THRESHOLD && f->state != DRYING) {
            f->state = DRYING;
            log_msg("Цветок %d: начал засыхать (вл. %.1f)\n", f->id, f->moisture);
        } else if (f->moisture <= REQUEST_THRESHOLD && f->state == NORMAL) {
            f->state = WILTING;
            f->has_request = 1;
            f->request_time = current_tick;
            log_msg("Цветок %d: создана заявка (вл. %.1f)\n", f->id, f->moisture);
            sem_post(&request_sem);
        }

        pthread_mutex_unlock(&sim_mutex);
    }
    return NULL;
}

void* gardener_thread(void* arg) {
    Gardener* g = (Gardener*)arg;

    while (!stop) {
        sem_wait(&request_sem);
        if (stop) break;

        pthread_mutex_lock(&sim_mutex);
        int fid = find_request(flowers_global, N_FLOWERS);
        
        if (fid != -1) {
            Flower* f = &flowers_global[fid];

            if (current_tick - f->last_water_time < PROTECTION_INTERVAL) {
                f->state = DEAD;
                f->has_request = 0;
                log_msg("Садовник %d: перелив! Цветок %d погиб\n", g->id, f->id);
            } else {
                f->has_request = 0;
                f->state = WATERING;
                g->is_busy = 1;
                log_msg("Садовник %d: начал полив цветка %d\n", g->id, f->id);
                pthread_mutex_unlock(&sim_mutex);

                usleep((useconds_t)(WATERING_DURATION * TICK_MS * 1000));

                pthread_mutex_lock(&sim_mutex);
                f->state = RECOVERING;
                f->state_timer = RECOVERY_DURATION;
                f->moisture = 100.0;
                f->last_water_time = current_tick;
                g->is_busy = 0;
                log_msg("Садовник %d: закончил полив цветка %d\n", g->id, f->id);
            }
        }
        pthread_mutex_unlock(&sim_mutex);
    }
    return NULL;
}