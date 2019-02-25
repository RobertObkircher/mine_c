#ifndef MINE_C_LIST_H
#define MINE_C_LIST_H

#include "allocorexit.h"

#define list(ElementType)   \
    struct {                \
        size_t length;      \
        size_t capacity;    \
        ElementType* data;  \
    }

// 1
#define list_ensure_capacity(List, Capacity)                                                              \
    do {                                                                                                  \
        __typeof__(List) *list_1 = &(List);                                                               \
        size_t desired_capacity_1 = (Capacity);                                                           \
        if (list_1->capacity < desired_capacity_1) {                                                      \
            list_1->data = reallocarray_or_exit(list_1->data, desired_capacity_1, sizeof(*list_1->data)); \
            list_1->capacity = desired_capacity_1;                                                        \
         }                                                                                                \
    } while (0)

// 2
#define list_ensure_free_capacity(List, FreeCapacity)                   \
    do {                                                                \
        __typeof__(List) *list_2 = &(List);                             \
        list_ensure_capacity(*list_2, list_2->length + (FreeCapacity)); \
    } while (0)

// 3
#define list_add(List, Element)                \
    do {                                       \
        __typeof__(List) *list_3 = &(List);    \
        list_ensure_free_capacity(*list_3, 1); \
        list_add_unsafe(*list_3, (Element));   \
    } while (0)

// 4
#define list_add_unsafe(List, Element)              \
    do {                                            \
        __typeof__(List) *list_4 = &(List);         \
        list_4->data[list_4->length++] = (Element); \
    } while (0)


#endif //MINE_C_LIST_H
