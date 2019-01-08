#ifndef MINE_C_DYNARRAY_H
#define MINE_C_DYNARRAY_H

#include <stdlib.h>

size_t realloc_if_too_small(void **array, size_t elem_size, size_t current_size, size_t desired_size);

#endif //MINE_C_DYNARRAY_H
