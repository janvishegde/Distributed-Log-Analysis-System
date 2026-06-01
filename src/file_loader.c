#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "../include/log_analyzer.h"

int mmap_open(const char *path, MappedFile *mf)
{
    struct stat st;
    mf->fd = open(path, O_RDONLY);
    if (mf->fd < 0) { perror("[file_loader] open() failed"); return -1; }

    if (fstat(mf->fd, &st) < 0) { perror("[file_loader] fstat() failed"); close(mf->fd); return -1; }

    mf->size = (size_t)st.st_size;
    if (mf->size == 0) { fprintf(stderr, "[file_loader] File is empty\n"); close(mf->fd); return -1; }

    mf->data = (char *)mmap(NULL, mf->size, PROT_READ, MAP_PRIVATE, mf->fd, 0);
    if (mf->data == MAP_FAILED) { perror("[file_loader] mmap() failed"); close(mf->fd); return -1; }

    madvise(mf->data, mf->size, MADV_SEQUENTIAL);
    printf("[file_loader] Mapped %.2f MB from '%s'\n", (double)mf->size / (1024.0*1024.0), path);
    return 0;
}

void mmap_close(MappedFile *mf)
{
    if (mf->data && mf->data != MAP_FAILED) { munmap(mf->data, mf->size); mf->data = NULL; }
    if (mf->fd >= 0) { close(mf->fd); mf->fd = -1; }
}
