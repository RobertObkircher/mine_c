#include "dynarray.h"

#include "allocorexit.h"

size_t realloc_if_too_small(void **array, size_t elem_size, size_t current_size, size_t desired_size) {
    if (current_size < desired_size) {
        if (!*array) {
            *array = calloc_or_exit(desired_size, elem_size);
        } else {
            *array = reallocarray_or_exit(*array, desired_size, elem_size);
        }
        return desired_size;
    } else {
        return current_size;
    }
}
