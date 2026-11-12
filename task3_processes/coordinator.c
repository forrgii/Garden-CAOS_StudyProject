#include "shared.h"

SimState *state;

void cleanup(int sig) {
    (void)sig;
    if (state) state->stop = 1;
}

int main(int argc, char** argv) {
    int sim_time = (argc > 1) ? atoi(argv[1]) : 300;
    int n_flowers = (argc > 2) ? atoi(argv[2]) : 5;

    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);

    // 1.Shared Memory 
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (ftruncate(shm_fd, sizeof(SimState)) == -1) {
        perror("ftruncate");
        return 1;
    }
    state = mmap(0, sizeof(SimState), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    state->current_tick = 0;
    state->stop = 0;

    // 2.Named Semaphores
    sem_unlink(SEM_LOG_NAME); sem_unlink(SEM_REQ_NAME);
    sem_open(SEM_LOG_NAME, O_CREAT, 0666, 1);
    sem_open(SEM_REQ_NAME, O_CREAT, 0666, 0);

    for (int i = 0; i < n_flowers; i++) {
        char name[64];
        snprintf(name, sizeof(name), "%s%d", SEM_RES_PREFIX, i);
        sem_unlink(name);
        sem_open(name, O_CREAT, 0666, 0);
    }

    // 3. Create Message Queues
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(Message);
    attr.mq_curmsgs = 0;

    mq_unlink(MQ_REQ_NAME);
    mq_open(MQ_REQ_NAME, O_CREAT | O_RDWR, 0666, &attr);

    for (int i = 0; i < n_flowers; i++) {
        char name[64];
        snprintf(name, sizeof(name), "%s%d", MQ_RES_PREFIX, i);
        mq_unlink(name);
        mq_open(name, O_CREAT | O_RDWR, 0666, &attr);
    }

    printf("[Coordinator] Resources initialized. Starting clock for %d ticks...\n", sim_time);
    sleep(1); // Let independent processes connect

    // 4. Run Clock
    for (int t = 0; t < sim_time && !state->stop; t++) {
        state->current_tick = t;
        usleep(TICK_MS * 1000);
    }

    printf("\n[Coordinator] Simulation time is over. Waiting for processes to exit...\n");
    state->stop = 1;
    
    // Wake up blocked gardeners so they can exit
    sem_t *sem_req_wake = sem_open(SEM_REQ_NAME, 0);
    for (int i = 0; i < 10; i++) sem_post(sem_req_wake); 
    
    sleep(2); // Give them time to gracefully terminate

    // 5. Cleanup IPC resources
    printf("[Coordinator] Cleaning up IPC resources...\n");
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_LOG_NAME);
    sem_unlink(SEM_REQ_NAME);
    
    for (int i = 0; i < n_flowers; i++) {
        char name[64];
        snprintf(name, sizeof(name), "%s%d", SEM_RES_PREFIX, i);
        sem_unlink(name);
        
        snprintf(name, sizeof(name), "%s%d", MQ_RES_PREFIX, i);
        mq_unlink(name);
    }
    mq_unlink(MQ_REQ_NAME);

    return 0;
}
