/*
 * dedupDataQueue.h
 *
 *  Created on: Jan 16, 2018
 *      Author: root
 */

#ifndef DEDUPDATAQUEUE_H_
#define DEDUPDATAQUEUE_H_

#ifndef headerIncluded
#include "headers.h"
#endif

#ifndef structIncluded
#include "types.h"
#endif

#ifndef constantsIncluded
#include "constants.h"
#endif

void Init_dedup(DEDUP_QUEUE* queue, int capacity);


// Queue is full when size becomes equal to the capacity
int isFull_dedup(DEDUP_QUEUE* queue);

// Queue is empty when size is 0
int isEmpty_dedup(DEDUP_QUEUE* queue);


// Function to add an item to the queue.  It changes rear and size
int enqueue_dedup(DEDUP_QUEUE* queue,DEDUP_RECORD* item);

// Function to remove an item from queue.  It changes front and size
int dequeue_dedup(DEDUP_QUEUE* queue);

// Function to get front of queue
DEDUP_RECORD* front_dedup(DEDUP_QUEUE* queue);


// Function to get rear of queue
DEDUP_RECORD* rear_dedup(DEDUP_QUEUE* queue);



#endif /* DEDUPDATAQUEUE_H_ */
