#ifndef SMART_FLOWERS_H
#define SMART_FLOWERS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

#define TICK_MS 100

extern int N_FLOWERS;
extern int N_GARDENERS;
extern int SIMULATION_TIME;
extern double DRYING_RATE_MIN;
extern double DRYING_RATE_MAX;
extern double REQUEST_THRESHOLD;
extern double DRY_THRESHOLD;
extern int WATERING_DURATION;
extern int RECOVERY_DURATION;
extern int PROTECTION_INTERVAL;

typedef enum {
    NORMAL, WILTING, WATERING, RECOVERING, DRYING, DEAD
} FlowerState;

typedef struct {
    int id;
    double moisture;
    double drying_rate;
    FlowerState state;
    int has_request;
    int request_time;
    int state_timer;      // <--- ВОТ ОНО, теперь оно снова здесь
    int last_water_time;
    pthread_t thread;
} Flower;

typedef struct {
    int id;
    int is_busy;
    int current_flower_id;
    pthread_t thread;
} Gardener;

extern pthread_mutex_t sim_mutex;
extern sem_t request_sem;
extern volatile sig_atomic_t stop;
extern volatile int current_tick;
extern Flower* flowers_global;

const char* state_name(FlowerState s);
void init_flowers(Flower* flowers, int n);
void init_gardeners(Gardener* gardeners, int n);
int find_request(Flower* flowers, int n_flowers);

void* flower_thread(void* arg);
void* gardener_thread(void* arg);

#endif