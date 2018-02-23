//
// Created by root on 12/22/17.
//

#include <DynamicArray.h>

/*
 * insert elements to array
 */
void initArray(Array *a, size_t initialSize) {
    a->array = (unsigned char *) malloc(initialSize);
    a->used = 0;
    a->size = initialSize;
}

/*
 * insert elements to array
 */
void insertArray(Array *a, char element) {
    if (a->used == a->size) {
        a->size *= 2;
        a->array = (char *) realloc(a->array, a->size);
    }
    a->array[a->used++] = element;
}

/*
 * release array
 */
void freeArray(Array *a) {
    free(a->array);
    a->array = NULL;
    a->used = a->size = 0;
}

