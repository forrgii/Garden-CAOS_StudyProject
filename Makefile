# ============================================
#  Makefile для проекта "Умные цветы" (вариант 14)
# ============================================

CC      = gcc
TARGET  = smart_flowers
SRCS    = main.c smart_flowers.c logger.c
OBJS    = $(SRCS:.c=.o)
HEADERS = smart_flowers.h logger.h

# --- Тесты ---
TEST_TARGET = test_smart_flowers
TEST_DIR    = tests
TEST_SRCS   = $(TEST_DIR)/test_runner.c    \
              $(TEST_DIR)/test_framework.c \
              $(TEST_DIR)/test_state.c     \
              $(TEST_DIR)/test_flowers.c   \
              $(TEST_DIR)/test_gardeners.c \
              $(TEST_DIR)/test_requests.c  \
              smart_flowers.c
TEST_OBJS   = $(TEST_SRCS:.c=.o)
TEST_INC    = -I. -I$(TEST_DIR)

# --- Логи ---
LOG_DIR   = logs

# --- Флаги ---
WARN      = -Wall -Wextra -Wpedantic -Wshadow -Wconversion

DBG_FLAGS   = -std=c11 -O0 -g3 -DDEBUG $(WARN) \
              -fsanitize=address,undefined -fno-omit-frame-pointer
DBG_LDFLAGS = -fsanitize=address,undefined

REL_FLAGS   = -std=c11 -O2 -DNDEBUG $(WARN)
REL_LDFLAGS =

VG_FLAGS    = -std=c11 -O0 -g3 -DDEBUG $(WARN)
VG_LDFLAGS  =

BUILD ?= debug

ifeq ($(BUILD),debug)
    CFLAGS  = $(DBG_FLAGS)
    LDFLAGS = $(DBG_LDFLAGS)
else ifeq ($(BUILD),release)
    CFLAGS  = $(REL_FLAGS)
    LDFLAGS = $(REL_LDFLAGS)
else ifeq ($(BUILD),valgrind)
    CFLAGS  = $(VG_FLAGS)
    LDFLAGS = $(VG_LDFLAGS)
else
    $(error Неизвестный BUILD=$(BUILD). Используйте debug|release|valgrind)
endif

# ============================================
.PHONY: all debug release clean clean-logs clean-all \
        run run-args gdb valgrind test help \
        logs-list logs-last

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

debug:
	$(MAKE) BUILD=debug all

release:
	$(MAKE) BUILD=release all

# --- Запуск ---
run: $(TARGET)
	./$(TARGET)

run-args: $(TARGET)
	./$(TARGET) 15 3 300

# --- Отладка ---
gdb: debug
	gdb -q ./$(TARGET)

# --- Valgrind: сборка без санитайзеров + лог с timestamp ---
valgrind:
	$(MAKE) clean
	$(MAKE) BUILD=valgrind all
	@mkdir -p $(LOG_DIR)
	@VGLOG=$$(date +"$(LOG_DIR)/valgrind_%Y-%m-%d_%H-%M-%S.log"); \
	valgrind --leak-check=full \
	         --show-leak-kinds=all \
	         --track-origins=yes \
	         --log-file=$$VGLOG \
	         ./$(TARGET) 10 2 50; \
	echo ""; \
	echo "Отчёт valgrind сохранён в $$VGLOG"

# --- Тесты ---
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(CFLAGS) $(TEST_INC) -o $@ $^ $(LDFLAGS)

$(TEST_DIR)/%.o: $(TEST_DIR)/%.c $(HEADERS) $(TEST_DIR)/test_framework.h
	$(CC) $(CFLAGS) $(TEST_INC) -c $< -o $@

# ============================================
#  Работа с логами
# ============================================

logs-list:
	@if [ -d $(LOG_DIR) ]; then \
	    echo "Сохранённые логи:"; \
	    ls -lh $(LOG_DIR)/; \
	else \
	    echo "Директория $(LOG_DIR)/ ещё не создана."; \
	fi

logs-last:
	@LATEST=$$(ls -t $(LOG_DIR)/simulation_*.log 2>/dev/null | head -1); \
	if [ -n "$$LATEST" ]; then \
	    echo "=== $$LATEST ==="; \
	    cat "$$LATEST"; \
	else \
	    echo "Логи симуляции не найдены."; \
	fi

# ============================================
#  Очистка
# ============================================

clean:
	rm -f $(OBJS) $(TEST_OBJS) $(TARGET) $(TEST_TARGET)

clean-logs:
	rm -rf $(LOG_DIR)
	rm -f *.log
	@echo "Все логи удалены."

clean-all: clean clean-logs
	@echo "Проект полностью очищен."

# ============================================
#  Справка
# ============================================
help:
	@echo "Доступные цели:"
	@echo "  make            - сборка debug (по умолчанию)"
	@echo "  make debug      - сборка с -O0 -g3 и санитайзерами"
	@echo "  make release    - сборка с -O2 -DNDEBUG"
	@echo "  make run        - интерактивный запуск"
	@echo "  make run-args   - запуск с 15 цветами, 3 садовниками, 300 тактов"
	@echo "  make gdb        - сборка debug + запуск под gdb"
	@echo "  make valgrind   - сборка без санитайзеров + проверка утечек"
	@echo "  make test       - сборка и запуск юнит-тестов"
	@echo "  make logs-list  - список сохранённых логов"
	@echo "  make logs-last  - показать последний лог симуляции"
	@echo "  make clean      - удалить .o и бинарники"
	@echo "  make clean-logs - удалить все логи"
	@echo "  make clean-all  - удалить и бинарники, и логи"