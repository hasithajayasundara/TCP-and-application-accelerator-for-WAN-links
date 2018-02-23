/*
 * WanDataQueue.c
 *
 *  Created on: Jan 4, 2018
 *      Author: root
 */

#include <writeInLogger.h>
#include <WanDataQueue.h>

void Init_wan_data(WAN_QUEUE* queue, int capacity)
{
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = -1;
    queue->dataArray = (WAN_RECORD**) malloc(queue->capacity * sizeof(WAN_RECORD*));
}

// Queue is full when size becomes equal to the capacity
int isFull_wan_data(WAN_QUEUE* queue)
{
	return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty_wan_data(WAN_QUEUE* queue)
{
	return (queue->size == 0);
}

// Function to add an item to the queue.  It changes rear and size
void enqueue_wan_data(WAN_QUEUE* queue, WAN_RECORD* item)
{

    if (isFull_data(queue))
        return;

    //pthread_mutex_lock(&queue->rearLock);
    queue->rear = (queue->rear+1)%(queue->capacity);
    //pthread_mutex_unlock(&queue->rearLock);
    queue->dataArray[queue->rear] = item;

    //pthread_mutex_lock(&queue->sizeLock);
    ++queue->size;
    //pthread_mutex_unlock(&queue->sizeLock);
}

// Function to remove an item from queue.  It changes front and size
WAN_RECORD* dequeue_wan_data(WAN_QUEUE* queue)
{
    if (isEmpty_data(queue))
        return INT_MIN;
    DATA_RECORD* item = queue->dataArray[queue->front];

    //pthread_mutex_lock(&queue->frontLock);
    queue->front=(queue->front+1)%(queue->capacity);
    //pthread_mutex_unlock(&queue->frontLock);

    //pthread_mutex_lock(&queue->sizeLock);
    queue->size = queue->size - 1;
    //pthread_mutex_unlock(&queue->sizeLock);
    return item;
}

// Function to get front of queue
WAN_RECORD* front_wan_data(WAN_QUEUE* queue)
{
    if (isEmpty_data(queue))
        return INT_MIN;
    return queue->dataArray[queue->front];
}

// Function to get rear of queue
WAN_RECORD* rear_wan_data(WAN_QUEUE* queue)
{
    if (isEmpty_data(queue))
        return INT_MIN;
    return queue->dataArray[queue->rear];
}

void displayWanDataOnLogger(WAN_QUEUE* queue, FILE* logfile) {
	printf("\n...................queue print..............\n");
	printf("\nsize:%d\n",queue->size);
	int i = queue->front;
	if (queue->front <= queue->rear) {
		for (; i <= queue->rear; i++) {
			logRawData(queue->dataArray[i]->data,
					queue->dataArray[i]->size, logfile);
		}
	} else {
		for (; (i % (queue->capacity)) != (queue->rear); ++i) {
			logRawData(queue->dataArray[i % (queue->capacity)]->data,
					queue->dataArray[i % (queue->capacity)]->size,
					logfile);
		}
	}

	return;
}
