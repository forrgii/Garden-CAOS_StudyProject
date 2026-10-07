#include "smart_flowers.h"

int N_FLOWERS            = 10;
int N_GARDENERS          = 2;
int SIMULATION_TIME      = 200;
double DRYING_RATE_MIN   = 0.5;
double DRYING_RATE_MAX   = 1.5;
double REQUEST_THRESHOLD = 40.0;
double DRY_THRESHOLD     = 20.0;
int WATERING_DURATION    = 5;
int RECOVERY_DURATION    = 10;
int PROTECTION_INTERVAL  = 15;

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
        gardeners[i].busy_timer = 0;
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