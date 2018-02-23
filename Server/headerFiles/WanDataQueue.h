/*
 * WanDataQueue.h
 *
 *  Created on: Jan 4, 2018
 *      Author: root
 */

#ifndef WANWANQUEUE_H_
#define WANWANQUEUE_H_

#ifndef headerIncluded
#include "headers.h"
#endif

#ifndef structIncluded
#include "types.h"
#endif

#ifndef constantsIncluded
#include "constants.h"
#endif

void Init_data(WAN_QUEUE* queue, int capacity);


// Queue is full when size becomes equal to the capacity
int isFull_data(WAN_QUEUE* queue);

// Queue is empty when size is 0
int isEmpty_data(WAN_QUEUE* queue);


// Function to add an item to the queue.  It changes rear and size
void enqueue_data(WAN_QUEUE* queue, WAN_RECORD* item);

// Function to remove an item from queue.  It changes front and size
WAN_RECORD* dequeue_data(WAN_QUEUE* queue);

// Function to get front of queue
WAN_RECORD* front_data(WAN_QUEUE* queue);


// Function to get rear of queue
WAN_RECORD* rear_data(WAN_QUEUE* queue);

//display data in queue
void displayDataOnLogger(WAN_QUEUE* queue, FILE* logfile);


#endif /* WANWANQUEUE_H_ */


