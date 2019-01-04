#include "allocorexit.h"
#include <log.h>
#include <malloc.h>

void log_out_of_memory_and_exit() {
    log_error("out of memory");
    exit(42);
}

void *malloc_or_exit(size_t size) {
    void *result = malloc(size);
    if (!result)
        log_out_of_memory_and_exit();
    return result;
}

void *calloc_or_exit(size_t nmemb, size_t size) {
    void *result = calloc(nmemb, size);
    if (!result)
        log_out_of_memory_and_exit();
    return result;
}

void *realloc_or_exit(void *ptr, size_t size) {
    void *result = realloc(ptr, size);
    if (!result)
        log_out_of_memory_and_exit();
    return result;
}

void *reallocarray_or_exit(void *ptr, size_t nmemb, size_t size) {
    void *result = reallocarray(ptr, nmemb, size);
    if (!result)
        log_out_of_memory_and_exit();
    return result;
}
