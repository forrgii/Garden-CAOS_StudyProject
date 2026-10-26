#include "logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>

static int  log_fd = -1;
static char log_path[512];
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

int log_init(const char* dir, const char* prefix) {
    if (dir == NULL || prefix == NULL) return -1;
    mkdir(dir, 0755);

    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char datebuf[64];
    strftime(datebuf, sizeof(datebuf), "%Y-%m-%d_%H-%M-%S", tm_info);

    int n = snprintf(log_path, sizeof(log_path), "%s/%s_%s.log", dir, prefix, datebuf);
    if (n <= 0 || n >= (int)sizeof(log_path)) return -1;

    log_fd = open(log_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (log_fd < 0) return -1;

    char header[256];
    int hn = snprintf(header, sizeof(header), "=== Log | %s ===\n", datebuf);
    if (hn > 0) {
        ssize_t w = write(log_fd, header, (size_t)hn);
        (void)w;
    }
    return 0;
}

void log_close(void) {
    pthread_mutex_lock(&log_mutex);
    if (log_fd >= 0) {
        close(log_fd);
        log_fd = -1;
    }
    pthread_mutex_unlock(&log_mutex);
}

void log_msg(const char* fmt, ...) {
    char buf[2048];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (n <= 0) return;
    if (n >= (int)sizeof(buf)) n = (int)sizeof(buf) - 1;

    pthread_mutex_lock(&log_mutex);

    fwrite(buf, 1, (size_t)n, stdout);
    fflush(stdout);

    if (log_fd >= 0) {
        ssize_t w = write(log_fd, buf, (size_t)n);
        (void)w;
    }

    pthread_mutex_unlock(&log_mutex);
}

void log_file_only(const char* fmt, ...) {
    char buf[2048];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (n <= 0) return;
    if (n >= (int)sizeof(buf)) n = (int)sizeof(buf) - 1;

    pthread_mutex_lock(&log_mutex);
    if (log_fd >= 0) {
        ssize_t w = write(log_fd, buf, (size_t)n);
        (void)w;
    }
    pthread_mutex_unlock(&log_mutex);
}

const char* log_get_path(void) {
    return (log_fd >= 0) ? log_path : NULL;
}