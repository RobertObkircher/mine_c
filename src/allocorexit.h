#ifndef MINE_C_ALLOCOREXIT_H
#define MINE_C_ALLOCOREXIT_H

#include <stdlib.h>

void log_out_of_memory_and_exit();

void *malloc_or_exit(size_t size);

void *calloc_or_exit(size_t nmemb, size_t size);

void *realloc_or_exit(void *ptr, size_t size);

void *reallocarray_or_exit(void *ptr, size_t nmemb, size_t size);

#endif //MINE_C_ALLOCOREXIT_H
