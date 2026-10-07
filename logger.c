#include "logger.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

static int  log_fd = -1;
static char log_path[512];

// ============================================
//  Инициализация
// ============================================
int log_init(const char* dir, const char* prefix) {
    if (dir == NULL || prefix == NULL) return -1;

    // Создаём директорию (если её нет)
    mkdir(dir, 0755);

    // Формируем имя с timestamp:
    //   <dir>/<prefix>_YYYY-MM-DD_HH-MM-SS.log
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char datebuf[64];
    strftime(datebuf, sizeof(datebuf), "%Y-%m-%d_%H-%M-%S", tm_info);

    int n = snprintf(log_path, sizeof(log_path),
                     "%s/%s_%s.log", dir, prefix, datebuf);
    if (n <= 0 || n >= (int)sizeof(log_path)) {
        return -1;
    }

    // Открываем файл: создаём, если нет; если есть — перезаписываем
    // (одинаковое имя возможно только при запуске в ту же секунду).
    log_fd = open(log_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (log_fd < 0) {
        perror("log_init: open");
        log_path[0] = '\0';
        return -1;
    }

    // Заголовок с датой/временем запуска
    char header[256];
    int hn = snprintf(header, sizeof(header),
                      "=== Smart Flowers log | started %s ===\n", datebuf);
    if (hn > 0) {
        ssize_t w = write(log_fd, header, (size_t)hn);
        (void)w;
    }

    return 0;
}

// ============================================
//  Закрытие
// ============================================
void log_close(void) {
    if (log_fd >= 0) {
        close(log_fd);
        log_fd = -1;
    }
}

// ============================================
//  Вывод в stdout + файл
// ============================================
void log_msg(const char* fmt, ...) {
    char buf[2048];
    va_list args;

    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (n <= 0) return;
    if (n >= (int)sizeof(buf)) n = (int)sizeof(buf) - 1;

    // В консоль
    fwrite(buf, 1, (size_t)n, stdout);
    fflush(stdout);

    // В файл
    if (log_fd >= 0) {
        ssize_t w = write(log_fd, buf, (size_t)n);
        (void)w;
    }
}

// ============================================
//  Вывод только в файл
// ============================================
void log_file_only(const char* fmt, ...) {
    char buf[2048];
    va_list args;

    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (n <= 0) return;
    if (n >= (int)sizeof(buf)) n = (int)sizeof(buf) - 1;

    if (log_fd >= 0) {
        ssize_t w = write(log_fd, buf, (size_t)n);
        (void)w;
    }
}

// ============================================
//  Путь к логу
// ============================================
const char* log_get_path(void) {
    return (log_fd >= 0) ? log_path : NULL;
}
