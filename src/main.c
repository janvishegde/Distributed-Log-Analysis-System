#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include "../include/log_analyzer.h"

static int get_cpu_count(void)
{
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    if (n <= 0 || n > MAX_THREADS) return 4;
    return (int)n;
}

static double elapsed_ms(struct timespec *start, struct timespec *end)
{
    return (double)(end->tv_sec  - start->tv_sec)  * 1000.0
         + (double)(end->tv_nsec - start->tv_nsec) / 1e6;
}

int main(int argc, char *argv[])
{
    if (argc < 2) { fprintf(stderr, "Usage: %s <logfile> [num_threads]\n", argv[0]); return EXIT_FAILURE; }

    const char *log_path  = argv[1];
    int         n_threads = (argc >= 3) ? atoi(argv[2]) : get_cpu_count();

    if (n_threads <= 0 || n_threads > MAX_THREADS) {
        fprintf(stderr, "[main] Invalid thread count %d (max %d)\n", n_threads, MAX_THREADS);
        return EXIT_FAILURE;
    }

    printf("╔══════════════════════════════════════╗\n");
    printf("║  Distributed Log File Analyzer       ║\n");
    printf("║  Threads: %-3d  File: %-16s  ║\n", n_threads, log_path);
    printf("╚══════════════════════════════════════╝\n\n");

    struct timespec t_start, t_end;
    clock_gettime(CLOCK_MONOTONIC, &t_start);

    MappedFile mf;
    if (mmap_open(log_path, &mf) != 0) return EXIT_FAILURE;

    Chunk chunks[MAX_THREADS];
    int n_chunks = partition_file(&mf, n_threads, chunks);
    if (n_chunks <= 0) { mmap_close(&mf); return EXIT_FAILURE; }

    GlobalResult global;
    global_result_init(&global);

    pthread_t  threads[MAX_THREADS];
    WorkerArgs args[MAX_THREADS];

    printf("\n[main] Spawning %d threads...\n", n_chunks);

    for (int i = 0; i < n_chunks; i++) {
        args[i].chunk  = chunks[i];
        args[i].global = &global;
        if (pthread_create(&threads[i], NULL, worker_thread, &args[i]) != 0) {
            perror("[main] pthread_create failed");
            mmap_close(&mf);
            global_result_destroy(&global);
            return EXIT_FAILURE;
        }
    }

    for (int i = 0; i < n_chunks; i++) pthread_join(threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &t_end);
    double ms = elapsed_ms(&t_start, &t_end);

    printf("\n══════════════════════════════════════\n");
    printf("  RESULTS\n");
    printf("══════════════════════════════════════\n");
    printf("  Total lines    : %ld\n",  global.total_lines);
    printf("  ERROR entries  : %ld\n",  global.total_errors);
    printf("  WARN  entries  : %ld\n",  global.total_warnings);
    printf("  INFO  entries  : %ld\n",  global.total_info);
    printf("  Unique IPs     : %d\n",   global.ip_count);
    printf("  Processing time: %.2f ms\n", ms);
    printf("══════════════════════════════════════\n\n");

    write_json(&global, OUTPUT_FILE);

    global_result_destroy(&global);
    mmap_close(&mf);
    return EXIT_SUCCESS;
}
