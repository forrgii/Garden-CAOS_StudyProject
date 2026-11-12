#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <mqueue.h>
#include <signal.h>
#include <stdbool.h>

// Resource names
#define SHM_NAME "/flower_shm"
#define MQ_REQ_NAME "/flower_mq_req"
#define MQ_RES_PREFIX "/flower_mq_res_"
#define SEM_LOG_NAME "/flower_sem_log"
#define SEM_REQ_NAME "/flower_sem_req"
#define SEM_RES_PREFIX "/flower_sem_res_"

#define MAX_FLOWERS 50
#define TICK_MS 100
#define PROTECTION_INTERVAL 15

typedef enum {
    NORMAL, WILTING, WATERING, DEAD
} FlowerState;

// Message Types
typedef enum {
    MSG_REQUEST = 0,
    MSG_WATER_START = 1,
    MSG_DEAD = 2
} MsgType;

// Message structure for MQ
typedef struct {
    MsgType type;
    int flower_id;
    int last_water_time;
} Message;

// Small shared memory just for the global clock and stop flag
typedef struct {
    volatile int current_tick;
    volatile sig_atomic_t stop;
} SimState;

#endif
