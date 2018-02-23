/*
 * dataChunkQueue.h
 *
 *  Created on: Jan 16, 2018
 *      Author: root
 */

#ifndef DATACHUNKQUEUE_H_
#define DATACHUNKQUEUE_H_
#define LandataQueueIncluded

#ifndef headerIncluded
#include "headers.h"
#endif

#ifndef structIncluded
#include "types.h"
#endif

#ifndef constantsIncluded
#include "constants.h"
#endif

void Init_chunk(CHUNK_QUEUE* queue, int capacity);


// Queue is full when size becomes equal to the capacity
int isFull_chunk(CHUNK_QUEUE* queue);

// Queue is empty when size is 0
int isEmpty_chunk(CHUNK_QUEUE* queue);


// Function to add an item to the queue.  It changes rear and size
int enqueue_chunk(CHUNK_QUEUE* queue,unsigned char* item);

// Function to remove an item from queue.  It changes front and size
int dequeue_chunk(CHUNK_QUEUE* queue);

// Function to get front of queue
unsigned char* front_chunk(CHUNK_QUEUE* queue);


// Function to get rear of queue
unsigned char* rear_chunk(CHUNK_QUEUE* queue);






#endif /* DATACHUNKQUEUE_H_ */
