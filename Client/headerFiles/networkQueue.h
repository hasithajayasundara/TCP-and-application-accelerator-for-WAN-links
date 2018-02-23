/*
 * networkQueue.h
 *
 *  Created on: Jan 16, 2018
 *      Author: root
 */

#ifndef NETWORKQUEUE_H_
#define NETWORKQUEUE_H_


#ifndef headerIncluded
#include "headers.h"
#endif

#ifndef structIncluded
#include "types.h"
#endif

#ifndef constantsIncluded
#include "constants.h"
#endif

void Init_network(NETWORK_QUEUE* queue, int capacity);


// Queue is full when size becomes equal to the capacity
int isFull_network(NETWORK_QUEUE* queue);

// Queue is empty when size is 0
int isEmpty_network(NETWORK_QUEUE* queue);


// Function to add an item to the queue.  It changes rear and size
int enqueue_network(NETWORK_QUEUE* queue,Array* item);

// Function to remove an item from queue.  It changes front and size
int dequeue_network(NETWORK_QUEUE* queue);

// Function to get front of queue
Array* front_network(NETWORK_QUEUE* queue);


// Function to get rear of queue
Array* rear_network(NETWORK_QUEUE* queue);



#endif /* NETWORKQUEUE_H_ */
