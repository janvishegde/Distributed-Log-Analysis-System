#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/log_analyzer.h"
#include "../include/parser.h"

void *worker_thread(void *arg)
{
    WorkerArgs   *wargs  = (WorkerArgs *)arg;
    const char   *ptr    = wargs->chunk.start;
    const char   *end    = ptr + wargs->chunk.length;
    ThreadResult *result = &wargs->result;
    memset(result, 0, sizeof(ThreadResult));

    // Thread-local tracking structures (Step 3 & Step 4)
    ThreadMetrics local_metrics;
    memset(&local_metrics, 0, sizeof(ThreadMetrics));

    IPTracker local_tracker;
    memset(&local_tracker, 0, sizeof(IPTracker));

    while (ptr < end) {
        const char *line_start = ptr;
        while (ptr < end && *ptr != '\n') ptr++;
        size_t line_len = (size_t)(ptr - line_start);
        if (ptr < end) ptr++;
        if (line_len == 0) continue;

        // Copy raw line to temporary stack buffer to ensure null-termination
        char line_buf[2048];
        if (line_len >= sizeof(line_buf)) {
            line_len = sizeof(line_buf) - 1;
        }
        memcpy(line_buf, line_start, line_len);
        line_buf[line_len] = '\0';

        // Parse log line (Step 2)
        LogEntry entry;
        parse_log_line(line_buf, &entry);

        // Accumulate thread-local metrics (Step 3)
        accumulate_metrics(&entry, &local_metrics);

        // Track IP address occurrences (Step 4)
        add_ip_to_tracker(&local_tracker, entry.ip_address);
    }

    // Merge local metrics into ThreadResult for coordination with Member 1's aggregator (Step 5)
    result->lines_processed = local_metrics.total_lines;
    result->error_count     = local_metrics.error_count;
    result->warn_count      = local_metrics.warn_count;
    result->info_count      = local_metrics.info_count;
    result->status_2xx      = local_metrics.status_2xx;
    result->status_4xx      = local_metrics.status_4xx;
    result->status_5xx      = local_metrics.status_5xx;

    result->ip_count = local_tracker.count;
    for (int i = 0; i < local_tracker.count; i++) {
        strncpy(result->top_ips[i].ip, local_tracker.entries[i].ip_address, 45);
        result->top_ips[i].ip[45] = '\0';
        result->top_ips[i].count  = local_tracker.entries[i].request_count;
        result->top_ips[i].is_anomaly = (local_tracker.entries[i].request_count > ANOMALY_LIMIT) ? 1 : 0;
    }

    printf("[worker %d] Done — lines=%ld errors=%ld warns=%ld 2xx=%ld 4xx=%ld 5xx=%ld\n",
           wargs->chunk.id, result->lines_processed,
           result->error_count, result->warn_count,
           result->status_2xx, result->status_4xx, result->status_5xx);

    // Coordinate with Member 1: uses Mutex Lock in merge_thread_result (Step 5)
    merge_thread_result(wargs->global, result);
    return NULL;
}
