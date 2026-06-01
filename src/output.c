#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/log_analyzer.h"

static int ip_compare(const void *a, const void *b)
{
    const IPEntry *ea = (const IPEntry *)a;
    const IPEntry *eb = (const IPEntry *)b;
    return eb->count - ea->count;
}

int write_json(const GlobalResult *gr, const char *path)
{
    FILE *fp = fopen(path, "w");
    if (!fp) { perror("[output] fopen() failed"); return -1; }

    IPEntry sorted[MAX_IP_ENTRIES];
    int     n = gr->ip_count;
    memcpy(sorted, gr->ip_table, (size_t)n * sizeof(IPEntry));
    qsort(sorted, (size_t)n, sizeof(IPEntry), ip_compare);

    fprintf(fp, "{\n");
    fprintf(fp, "  \"total_lines\":    %ld,\n", gr->total_lines);
    fprintf(fp, "  \"total_errors\":   %ld,\n", gr->total_errors);
    fprintf(fp, "  \"total_warnings\": %ld,\n", gr->total_warnings);
    fprintf(fp, "  \"total_info\":     %ld,\n", gr->total_info);
    fprintf(fp, "  \"status_2xx\":     %ld,\n", gr->total_2xx);
    fprintf(fp, "  \"status_4xx\":     %ld,\n", gr->total_4xx);
    fprintf(fp, "  \"status_5xx\":     %ld,\n", gr->total_5xx);

    /* Anomaly summary */
    int anomaly_count = 0;
    for (int i = 0; i < n; i++)
        if (sorted[i].is_anomaly) anomaly_count++;

    fprintf(fp, "  \"anomaly_count\":  %d,\n", anomaly_count);
    fprintf(fp, "  \"ip_frequency\": [\n");
    for (int i = 0; i < n; i++) {
        fprintf(fp, "    { \"ip\": \"%s\", \"count\": %d, \"anomaly\": %s }%s\n",
                sorted[i].ip,
                sorted[i].count,
                sorted[i].is_anomaly ? "true" : "false",
                (i < n-1) ? "," : "");
    }
    fprintf(fp, "  ]\n}\n");

    fclose(fp);

    /* Print anomaly warning to terminal */
    printf("[output] Results written to '%s'\n", path);
    if (anomaly_count > 0) {
        printf("\n⚠️  ANOMALY ALERT — %d suspicious IPs exceeded %d requests:\n",
               anomaly_count, ANOMALY_THRESHOLD);
        for (int i = 0; i < n; i++) {
            if (sorted[i].is_anomaly)
                printf("   🚨 %s — %d requests\n", sorted[i].ip, sorted[i].count);
        }
    } else {
        printf("[output] No anomalies detected.\n");
    }

    return 0;
}
