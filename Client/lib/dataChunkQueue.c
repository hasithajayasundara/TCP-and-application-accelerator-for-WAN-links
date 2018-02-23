/*
 * dataChunkQueue.c

 *
 *  Created on: Jan 16, 2018
 *      Author: root
 */

#include <dataChunkQueue.h>

void Init_chunk(CHUNK_QUEUE* queue, int capacity)
{
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = -1;
    queue->dataChunkArray = (unsigned char**) malloc(queue->capacity * sizeof(unsigned char*));
}

// Queue is full when size becomes equal to the capacity
int isFull_chunk(CHUNK_QUEUE* queue)
{
	return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty_chunk(CHUNK_QUEUE* queue)
{
	return (queue->size == 0);
}

// Function to add an item to the queue.  It changes rear and size
int enqueue_chunk(CHUNK_QUEUE* queue, unsigned char* item)
{

    if (isFull_chunk(queue))
        return INT_MIN;

    queue->rear = (queue->rear+1)%(queue->capacity);
    queue->dataChunkArray[queue->rear] = item;


    //pthread_mutex_lock(&queue->sizeLock);
    ++queue->size;
    //pthread_mutex_unlock(&queue->sizeLock);

    return INT_MAX;
}

// Function to remove an item from queue.  It changes front and size
int dequeue_chunk(CHUNK_QUEUE* queue)
{
    if (isEmpty_chunk(queue))
        return INT_MIN;
    char* item = queue->dataChunkArray[queue->front];
    queue->front=(queue->front+1)%(queue->capacity);

    //pthread_mutex_lock(&queue->sizeLock);
    queue->size = queue->size - 1;
    //pthread_mutex_unlock(&queue->sizeLock);
    return INT_MAX;
}

// Function to get front of queue
unsigned char* front_chunk(CHUNK_QUEUE* queue)
{
    if (isEmpty_chunk(queue))
        return INT_MIN;
    return queue->dataChunkArray[queue->front];
}

// Function to get rear of queue
unsigned char* rear_chunk(CHUNK_QUEUE* queue)
{
    if (isEmpty_chunk(queue))
        return INT_MIN;
    return queue->dataChunkArray[queue->rear];
}

