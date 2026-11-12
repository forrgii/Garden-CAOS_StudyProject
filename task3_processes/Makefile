CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -pthread -lrt

all: coordinator flower gardener

coordinator: coordinator.c shared.h
	$(CC) $(CFLAGS) -o coordinator coordinator.c $(LDFLAGS)

flower: flower.c shared.h
	$(CC) $(CFLAGS) -o flower flower.c $(LDFLAGS)

gardener: gardener.c shared.h
	$(CC) $(CFLAGS) -o gardener gardener.c $(LDFLAGS)

clean:
	rm -f coordinator flower gardener
