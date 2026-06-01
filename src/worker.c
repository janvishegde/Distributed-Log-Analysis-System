#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/log_analyzer.h"

static int line_contains(const char *line, size_t len, const char *keyword)
{
    size_t klen = strlen(keyword);
    if (klen > len) return 0;
    for (size_t i = 0; i <= len - klen; i++)
        if (memcmp(line + i, keyword, klen) == 0) return 1;
    return 0;
}

static void extract_and_count_ip(const char *line, size_t len, ThreadResult *res)
{
    for (size_t i = 0; i < len; i++) {
        if (line[i] < '0' || line[i] > '9') continue;
        int a, b, c, d, consumed;
        char buf[32];
        size_t copy_len = len - i < 31 ? len - i : 31;
        memcpy(buf, line + i, copy_len);
        buf[copy_len] = '\0';
        if (sscanf(buf, "%d.%d.%d.%d%n", &a, &b, &c, &d, &consumed) == 4
            && a>=0 && a<=255 && b>=0 && b<=255
            && c>=0 && c<=255 && d>=0 && d<=255) {
            char ip[46];
            snprintf(ip, sizeof(ip), "%d.%d.%d.%d", a, b, c, d);
            int found = 0;
            for (int j = 0; j < res->ip_count; j++) {
                if (strcmp(res->top_ips[j].ip, ip) == 0) {
                    res->top_ips[j].count++;
                    found = 1;
                    break;
                }
            }
            if (!found && res->ip_count < MAX_IP_ENTRIES) {
                strncpy(res->top_ips[res->ip_count].ip, ip, 45);
                res->top_ips[res->ip_count].count = 1;
                res->ip_count++;
            }
            i += (size_t)consumed - 1;
        }
    }
}

static void detect_status_code(const char *line, size_t len, ThreadResult *res)
{
    /* Look for 3-digit status code pattern: space + 2xx/4xx/5xx + space */
    for (size_t i = 1; i + 3 < len; i++) {
        if (line[i-1] == ' ' && line[i+3] == ' ') {
            int code = 0;
            char buf[4] = {line[i], line[i+1], line[i+2], '\0'};
            code = atoi(buf);
            if      (code >= 200 && code < 300) { res->status_2xx++; return; }
            else if (code >= 400 && code < 500) { res->status_4xx++; return; }
            else if (code >= 500 && code < 600) { res->status_5xx++; return; }
        }
    }
}

void *worker_thread(void *arg)
{
    WorkerArgs   *wargs  = (WorkerArgs *)arg;
    const char   *ptr    = wargs->chunk.start;
    const char   *end    = ptr + wargs->chunk.length;
    ThreadResult *result = &wargs->result;
    memset(result, 0, sizeof(ThreadResult));

    while (ptr < end) {
        const char *line_start = ptr;
        while (ptr < end && *ptr != '\n') ptr++;
        size_t line_len = (size_t)(ptr - line_start);
        if (ptr < end) ptr++;
        if (line_len == 0) continue;

        result->lines_processed++;

        /* Log level */
        if      (line_contains(line_start, line_len, "ERROR")) result->error_count++;
        else if (line_contains(line_start, line_len, "WARN" )) result->warn_count++;
        else if (line_contains(line_start, line_len, "INFO" )) result->info_count++;

        /* HTTP status code */
        detect_status_code(line_start, line_len, result);

        /* IP extraction */
        extract_and_count_ip(line_start, line_len, result);
    }

    printf("[worker %d] Done — lines=%ld errors=%ld warns=%ld 2xx=%ld 4xx=%ld 5xx=%ld\n",
           wargs->chunk.id, result->lines_processed,
           result->error_count, result->warn_count,
           result->status_2xx, result->status_4xx, result->status_5xx);

    merge_thread_result(wargs->global, result);
    return NULL;
}
