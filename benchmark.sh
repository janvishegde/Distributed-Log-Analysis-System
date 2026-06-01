#!/bin/bash

echo "Generating 1 million line test log..."
python3 tests/generate_test_log.py 1000000 test.log

echo ""
echo "=========================================="
echo "   BENCHMARK — Threads vs Time"
echo "=========================================="
printf "%-10s %-15s %-10s\n" "Threads" "Time (ms)" "Speedup"
echo "------------------------------------------"

BASE_TIME=0

for T in 1 2 4 8; do
    TIME=$(python3 -c "
import subprocess, time
best = 999999
for _ in range(3):
    start = time.time()
    subprocess.run(['./log_analyzer', 'test.log', '$T'], capture_output=True)
    ms = (time.time() - start) * 1000
    if ms < best:
        best = ms
print(f'{best:.0f}')
")

    if [ "$T" = "1" ]; then
        BASE_TIME=$TIME
        SPEEDUP="1.00x  (baseline)"
    else
        SPEEDUP=$(python3 -c "print(f'{$BASE_TIME/$TIME:.2f}x')")
    fi

    printf "%-10s %-15s %-10s\n" "$T" "$TIME" "$SPEEDUP"
done

echo "=========================================="
