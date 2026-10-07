#ifndef SMART_FLOWERS_H
#define SMART_FLOWERS_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

// Параметры модели (значения по умолчанию)
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

// Состояния цветка
typedef enum {
    NORMAL,
    WILTING,
    WATERING,
    RECOVERING,
    DRYING,
    DEAD
} FlowerState;

// Структура цветка
typedef struct {
    int id;
    double moisture;
    double drying_rate;
    FlowerState state;
    int has_request;
    int request_time;
    int state_timer;
    int last_water_time;
} Flower;

// Структура садовника
typedef struct {
    int id;
    int is_busy;
    int current_flower_id;
    int busy_timer;
} Gardener;

// Прототипы функций
const char* state_name(FlowerState s);
void init_flowers(Flower* flowers, int n);
void init_gardeners(Gardener* gardeners, int n);
int find_request(Flower* flowers, int n_flowers);

#endif