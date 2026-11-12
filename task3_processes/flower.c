#include "shared.h"
#include <stdarg.h>

// Shared logging function via Named Semaphore
void log_msg(const char* fmt, ...) {
    sem_t *sem_log = sem_open(SEM_LOG_NAME, 0);
    if (sem_log != SEM_FAILED) sem_wait(sem_log);
    
    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    fflush(stdout);

    if (sem_log != SEM_FAILED) sem_post(sem_log);
}

SimState *state;

int main(int argc, char** argv) {
    if (argc < 2) { printf("Usage: %s <flower_id>\n", argv[0]); return 1; }
    int id = atoi(argv[1]);

    // Connect to Shared Memory
    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    state = mmap(0, sizeof(SimState), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Open Queues and Semaphores
    mqd_t mq_req = mq_open(MQ_REQ_NAME, O_WRONLY);
    char res_name[64]; snprintf(res_name, sizeof(res_name), "%s%d", MQ_RES_PREFIX, id);
    mqd_t mq_res = mq_open(res_name, O_RDONLY);

    char sem_res_name[64]; snprintf(sem_res_name, sizeof(sem_res_name), "%s%d", SEM_RES_PREFIX, id);
    sem_t *sem_res = sem_open(sem_res_name, 0);

    double moisture = 100.0;
    double drying_rate = 0.5 + (rand() / (double)RAND_MAX) * 1.0;
    FlowerState my_state = NORMAL;
    int last_water_time = -15;

    sleep(1); // Wait for coordinator

    while (!state->stop) {
        usleep(TICK_MS * 1000);
        
        if (my_state == NORMAL || my_state == WILTING) {
            moisture -= drying_rate;
            if (moisture <= 0) {
                my_state = DEAD;
                log_msg("[Flower %d] Died of drought.\n", id);
                break;
            } else if (moisture <= 40.0 && my_state == NORMAL) {
                my_state = WILTING;
                log_msg("[Flower %d] Wilting. Sending REQUEST.\n", id);
                
                Message msg = { MSG_REQUEST, id, last_water_time };
                mq_send(mq_req, (char*)&msg, sizeof(Message), 0);
                
                sem_t *sem_req = sem_open(SEM_REQ_NAME, 0);
                sem_post(sem_req); // Wake up a gardener
            }
        }

        if (my_state == WILTING) {
            // Wait for the gardener's response
            sem_wait(sem_res);
            
            Message reply;
            if (mq_receive(mq_res, (char*)&reply, sizeof(Message), NULL) > 0) {
                if (reply.type == MSG_WATER_START) {
                    my_state = WATERING;
                    log_msg("[Flower %d] Being watered...\n", id);
                    
                    usleep(5 * TICK_MS * 1000); // Watering duration
                    
                    moisture = 100.0;
                    my_state = NORMAL;
                    last_water_time = state->current_tick;
                    log_msg("[Flower %d] Recovered and back to NORMAL.\n", id);
                } else if (reply.type == MSG_DEAD) {
                    my_state = DEAD;
                    log_msg("[Flower %d] Killed by OVERFLOW.\n", id);
                    break;
                }
            }
        }
    }

    log_msg("[Flower %d] Exiting.\n", id);
    return 0;
}
