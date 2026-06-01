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
    fprintf(fp, "  \"ip_frequency\": [\n");
    for (int i = 0; i < n; i++) {
        fprintf(fp, "    { \"ip\": \"%s\", \"count\": %d }%s\n",
                sorted[i].ip, sorted[i].count, (i < n-1) ? "," : "");
    }
    fprintf(fp, "  ]\n}\n");

    fclose(fp);
    printf("[output] Results written to '%s'\n", path);
    return 0;
}
