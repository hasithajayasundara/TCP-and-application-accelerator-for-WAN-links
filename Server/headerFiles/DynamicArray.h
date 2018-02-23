//
// Created by root on 12/22/17.
//

#define TCPACCELERATORNEW_DYNAMICARRAY_H

#ifndef headerIncluded
#include <headers.h>
#endif

#ifndef structIncluded
#include <types.h>
#endif

#ifndef constantsIncluded
#include <constants.h>
#endif

/*
 * insert elements to array
 */
void initArray(Array *a, size_t initialSize) ;

/*
 * insert elements to array
 */
void insertArray(Array *a, char element);

/*
 * release array
 */
void freeArray(Array *a);



