#include "dynarray.h"

#include "allocorexit.h"

size_t realloc_if_too_small(void **array, size_t elem_size, size_t num_elements, size_t desired_num_elements) {
    if (num_elements < desired_num_elements) {
        if (!*array) {
            *array = calloc_or_exit(desired_num_elements, elem_size);
        } else {
            *array = reallocarray_or_exit(*array, desired_num_elements, elem_size);
        }
        return desired_num_elements;
    } else {
        return num_elements;
    }
}
