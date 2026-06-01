#include <stdio.h>
#include <stddef.h>
#include "../include/log_analyzer.h"

static size_t find_next_newline(const char *data, size_t size, size_t pos)
{
    while (pos < size && data[pos] != '\n') pos++;
    if (pos < size) pos++;
    return pos;
}

int partition_file(const MappedFile *mf, int n_chunks, Chunk *chunks_out)
{
    if (!mf || !mf->data || mf->size == 0 || n_chunks <= 0) {
        fprintf(stderr, "[partitioner] Invalid arguments\n");
        return -1;
    }

    const char *data  = mf->data;
    size_t      total = mf->size;
    size_t      ideal = total / (size_t)n_chunks;
    size_t      chunk_start = 0;
    int         created = 0;

    for (int i = 0; i < n_chunks && chunk_start < total; i++) {
        size_t ideal_end = chunk_start + ideal;
        size_t chunk_end;

        if (i == n_chunks - 1 || ideal_end >= total)
            chunk_end = total;
        else
            chunk_end = find_next_newline(data, total, ideal_end);

        chunks_out[created].start  = data + chunk_start;
        chunks_out[created].length = chunk_end - chunk_start;
        chunks_out[created].id     = created;

        printf("[partitioner] Chunk %d: offset=%-10zu size=%-10zu bytes\n",
               created, chunk_start, chunks_out[created].length);

        chunk_start = chunk_end;
        created++;
    }

    printf("[partitioner] Created %d chunks from %.2f MB file\n",
           created, (double)total / (1024.0*1024.0));
    return created;
}
