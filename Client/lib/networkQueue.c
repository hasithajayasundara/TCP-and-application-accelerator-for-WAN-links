/*
 * networkQueue.h
 *
 *  Created on: Jan 16, 2018
 *      Author: root
 */

/*
 * dataChunkQueue.c

 *
 *  Created on: Jan 16, 2018
 *      Author: root
 */

#include <networkQueue.h>

void Init_network(NETWORK_QUEUE* queue, int capacity)
{
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = -1;
    queue->networkArray = (Array**) malloc(queue->capacity * sizeof(Array*));
}

// Queue is full when size becomes equal to the capacity
int isFull_network(NETWORK_QUEUE* queue)
{
	return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty_network(NETWORK_QUEUE* queue)
{
	return (queue->size == 0);
}

// Function to add an item to the queue.  It changes rear and size
int enqueue_network(NETWORK_QUEUE* queue, Array* item)
{

    if (isFull_network(queue))
        return INT_MIN;

    queue->rear = (queue->rear+1)%(queue->capacity);
    queue->networkArray[queue->rear] = item;


    //pthread_mutex_lock(&queue->sizeLock);
    ++queue->size;
    //pthread_mutex_unlock(&queue->sizeLock);

    return INT_MAX;
}

// Function to remove an item from queue.  It changes front and size
int dequeue_network(NETWORK_QUEUE* queue)
{
    if (isEmpty_network(queue))
        return INT_MIN;
    Array* item = queue->networkArray[queue->front];
    queue->front=(queue->front+1)%(queue->capacity);

    //pthread_mutex_lock(&queue->sizeLock);
    queue->size = queue->size - 1;
    //pthread_mutex_unlock(&queue->sizeLock);
    return INT_MAX;
}

// Function to get front of queue
Array* front_network(NETWORK_QUEUE* queue)
{
    if (isEmpty_network(queue))
        return INT_MIN;
    return queue->networkArray[queue->front];
}

// Function to get rear of queue
Array* rear_network(NETWORK_QUEUE* queue)
{
    if (isEmpty_network(queue))
        return INT_MIN;
    return queue->networkArray[queue->rear];
}

