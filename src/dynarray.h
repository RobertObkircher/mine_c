#ifndef MINE_C_DYNARRAY_H
#define MINE_C_DYNARRAY_H

#include <stdlib.h>

size_t realloc_if_too_small(void **array, size_t elem_size, size_t num_elements, size_t desired_num_elements);

#endif //MINE_C_DYNARRAY_H
