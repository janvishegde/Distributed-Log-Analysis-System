#ifndef LOG_ANALYZER_H
#define LOG_ANALYZER_H

#include <stddef.h>
#include <pthread.h>

#define MAX_THREADS       16
#define MAX_IP_ENTRIES    1024
#define OUTPUT_FILE       "results.json"
#define ANOMALY_THRESHOLD 1000

typedef struct { char *data; size_t size; int fd; } MappedFile;
typedef struct { const char *start; size_t length; int id; } Chunk;
typedef struct { char ip[46]; int count; int is_anomaly; } IPEntry;

typedef struct {
    long error_count;
    long warn_count;
    long info_count;
    long lines_processed;
    long status_2xx;
    long status_4xx;
    long status_5xx;
    IPEntry top_ips[MAX_IP_ENTRIES];
    int ip_count;
} ThreadResult;

typedef struct {
    long total_errors;
    long total_warnings;
    long total_info;
    long total_lines;
    long total_2xx;
    long total_4xx;
    long total_5xx;
    IPEntry ip_table[MAX_IP_ENTRIES];
    int ip_count;
    pthread_mutex_t lock;
} GlobalResult;

typedef struct {
    Chunk chunk;
    ThreadResult result;
    GlobalResult *global;
} WorkerArgs;

int   mmap_open(const char *path, MappedFile *mf);
void  mmap_close(MappedFile *mf);
int   partition_file(const MappedFile *mf, int n_chunks, Chunk *chunks_out);
void *worker_thread(void *arg);
void  global_result_init(GlobalResult *gr);
void  global_result_destroy(GlobalResult *gr);
void  merge_thread_result(GlobalResult *gr, const ThreadResult *tr);
int   write_json(const GlobalResult *gr, const char *path);

#endif
