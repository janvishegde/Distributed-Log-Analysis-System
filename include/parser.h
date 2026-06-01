#ifndef PARSER_H
#define PARSER_H

#define MAX_IP_TRACK      1024
#define ANOMALY_LIMIT     1000

typedef struct {
    char ip_address[46];   // Accommodates IPv4 and IPv6
    char timestamp[32];    // E.g., [01/Jun/2026:22:38:04]
    char log_level[8];     // INFO, WARN, ERROR
    int status_code;       // 200, 404, 500, etc.
} LogEntry;

typedef struct {
    long total_lines;
    long info_count;
    long warn_count;
    long error_count;
    long status_2xx;
    long status_4xx;
    long status_5xx;
} ThreadMetrics;

typedef struct {
    char ip_address[46];
    int request_count;
} IPCounter;

typedef struct {
    IPCounter entries[MAX_IP_TRACK];
    int count;
} IPTracker;

void parse_log_line(const char *line, LogEntry *entry);
void accumulate_metrics(const LogEntry *entry, ThreadMetrics *metrics);
void add_ip_to_tracker(IPTracker *tracker, const char *ip);
int  detect_anomalies(const IPTracker *tracker, IPCounter *anomalies_out, int max_anomalies, int threshold);

#endif // PARSER_H
