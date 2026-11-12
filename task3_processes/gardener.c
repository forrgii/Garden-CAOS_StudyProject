#include "shared.h"
#include <stdarg.h>

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
    int id = (argc > 1) ? atoi(argv[1]) : 0;

    int shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    state = mmap(0, sizeof(SimState), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    mqd_t mq_req = mq_open(MQ_REQ_NAME, O_RDONLY);
    sem_t *sem_req = sem_open(SEM_REQ_NAME, 0);

    sleep(1); // Wait for coordinator

    while (!state->stop) {
        // Wait for a request
        sem_wait(sem_req);
        
        if (state->stop) break;

        Message req;
        if (mq_receive(mq_req, (char*)&req, sizeof(Message), NULL) > 0) {
            if (req.type != MSG_REQUEST) continue;

            int f_id = req.flower_id;
            char res_name[64]; snprintf(res_name, sizeof(res_name), "%s%d", MQ_RES_PREFIX, f_id);
            mqd_t mq_res = mq_open(res_name, O_WRONLY);
            
            char sem_res_name[64]; snprintf(sem_res_name, sizeof(sem_res_name), "%s%d", SEM_RES_PREFIX, f_id);
            sem_t *sem_res = sem_open(sem_res_name, 0);

            Message reply;
            reply.flower_id = f_id;

            // Check protection interval (15 ticks)
            if (state->current_tick - req.last_water_time < PROTECTION_INTERVAL) {
                log_msg("[Gardener %d] Detected OVERFLOW for flower %d. Sending DEAD.\n", id, f_id);
                reply.type = MSG_DEAD;
            } else {
                log_msg("[Gardener %d] Starting to water flower %d.\n", id, f_id);
                reply.type = MSG_WATER_START;
            }

            // Send response via Message Queue
            mq_send(mq_res, (char*)&reply, sizeof(Message), 0);
            
            // Signal the specific flower via Named Semaphore
            sem_post(sem_res); 
            
            mq_close(mq_res);
            sem_close(sem_res);
        }
    }
    log_msg("[Gardener %d] Exiting.\n", id);
    return 0;
}
