# Distributed Log File Analyzer

A high-performance log analysis system built in C using **Divide and Conquer** + **pthreads**.

## How it works
- **Divide** — splits the log file into N equal chunks at newline boundaries
- **Conquer** — spawns N parallel threads, each scanning its chunk independently  
- **Combine** — merges all thread results safely using mutex locks
- **Output** — writes `results.json` for the Python dashboard

## Project Structure
## Build & Run
```bash
make
python3 tests/generate_test_log.py 500000 test.log
./log_analyzer test.log 4
```

## Benchmark
```bash
./benchmark.sh
```

## Output
Results are saved to `results.json` — consumed by the Python dashboard teammate.

## Tech Used
- C (GCC), pthreads, mmap
- Linux system calls: mmap, madvise, pthread_create, pthread_mutex
