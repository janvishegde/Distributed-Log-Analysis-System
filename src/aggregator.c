#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/log_analyzer.h"

void global_result_init(GlobalResult *gr)
{
    memset(gr, 0, sizeof(GlobalResult));
    pthread_mutex_init(&gr->lock, NULL);
}

void global_result_destroy(GlobalResult *gr)
{
    pthread_mutex_destroy(&gr->lock);
}

static void merge_ip_tables(GlobalResult *gr, const ThreadResult *tr)
{
    for (int i = 0; i < tr->ip_count; i++) {
        const char *ip    = tr->top_ips[i].ip;
        int         count = tr->top_ips[i].count;
        int         found = 0;
        for (int j = 0; j < gr->ip_count; j++) {
            if (strcmp(gr->ip_table[j].ip, ip) == 0) { gr->ip_table[j].count += count; found = 1; break; }
        }
        if (!found && gr->ip_count < MAX_IP_ENTRIES) {
            strncpy(gr->ip_table[gr->ip_count].ip, ip, 45);
            gr->ip_table[gr->ip_count].count = count;
            gr->ip_count++;
        }
    }
}

void merge_thread_result(GlobalResult *gr, const ThreadResult *tr)
{
    pthread_mutex_lock(&gr->lock);
    gr->total_errors   += tr->error_count;
    gr->total_warnings += tr->warn_count;
    gr->total_info     += tr->info_count;
    gr->total_lines    += tr->lines_processed;
    merge_ip_tables(gr, tr);
    pthread_mutex_unlock(&gr->lock);
}
