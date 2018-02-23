/*
 * dedupDataQueue.c
 *
 *  Created on: Jan 16, 2018
 *      Author: root
 */


#include <dedupDataQueue.h>

void Init_dedup(DEDUP_QUEUE* queue, int capacity)
{
    queue->capacity = capacity;
    queue->front = 0;
    queue->size = 0;
    queue->rear = -1;
    queue->dedupArray = (DEDUP_RECORD**) malloc(queue->capacity * sizeof(DEDUP_RECORD*));
}

// Queue is full when size becomes equal to the capacity
int isFull_dedup(DEDUP_QUEUE* queue)
{
	return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty_dedup(DEDUP_QUEUE* queue)
{
	return (queue->size == 0);
}

// Function to add an item to the queue.  It changes rear and size
int enqueue_dedup(DEDUP_QUEUE* queue, DEDUP_RECORD* item)
{

    if (isFull_dedup(queue))
        return INT_MIN;

    queue->rear = (queue->rear+1)%(queue->capacity);
    queue->dedupArray[queue->rear] = item;


    //pthread_mutex_lock(&queue->sizeLock);
    ++queue->size;
    //pthread_mutex_unlock(&queue->sizeLock);

    return INT_MAX;
}

// Function to remove an item from queue.  It changes front and size
int dequeue_dedup(DEDUP_QUEUE* queue)
{
    if (isEmpty_dedup(queue))
        return INT_MIN;
    DEDUP_RECORD* item = queue->dedupArray[queue->front];
    queue->front=(queue->front+1)%(queue->capacity);

    //pthread_mutex_lock(&queue->sizeLock);
    queue->size = queue->size - 1;
    //pthread_mutex_unlock(&queue->sizeLock);
    return INT_MAX;
}

// Function to get front of queue
DEDUP_RECORD* front_dedup(DEDUP_QUEUE* queue)
{
    if (isEmpty_dedup(queue))
        return INT_MIN;
    return queue->dedupArray[queue->front];
}

// Function to get rear of queue
DEDUP_RECORD* rear_dedup(DEDUP_QUEUE* queue)
{
    if (isEmpty_dedup(queue))
        return INT_MIN;
    return queue->dedupArray[queue->rear];
}
