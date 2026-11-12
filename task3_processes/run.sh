#!/bin/bash

# Default values
FLOWERS=5
GARDENERS=2
TIME=150

echo "Starting Coordinator..."
./coordinator $TIME $FLOWERS &
COORD_PID=$!

sleep 1 # Give time to initialize IPC

echo "Starting $FLOWERS flowers..."
for i in $(seq 0 $((FLOWERS-1))); do
    ./flower $i &
done

echo "Starting $GARDENERS gardeners..."
for i in $(seq 0 $((GARDENERS-1))); do
    ./gardener $i &
done

# Handle Ctrl+C gracefully
trap "echo -e '\nKilling all processes...'; kill $COORD_PID; pkill -f './flower'; pkill -f './gardener'; exit 0" SIGINT SIGTERM

# Wait for coordinator to finish
wait $COORD_PID

echo "All processes finished."
