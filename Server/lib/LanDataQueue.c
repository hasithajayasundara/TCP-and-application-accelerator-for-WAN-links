
/*
 *
 *
 *  Created on: Sep 7, 2017
 *      Author: deshanchathusanka
 */

#include <writeInLogger.h>
#include <LanDataQueue.h>

void Init_data(DATA_QUEUE* queue, int capacity)
{
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = -1;
    queue->dataRecordArray = (DATA_RECORD**) malloc(queue->capacity * sizeof(DATA_RECORD*));
}

// Queue is full when size becomes equal to the capacity
int isFull_data(DATA_QUEUE* queue)
{
	return (queue->size == queue->capacity);
}

// Queue is empty when size is 0
int isEmpty_data(DATA_QUEUE* queue)
{
	return (queue->size == 0);
}

// Function to add an item to the queue.  It changes rear and size
void enqueue_data(DATA_QUEUE* queue, DATA_RECORD* item)
{

    if (isFull_data(queue))
        return;

    //pthread_mutex_lock(&queue->rearLock);
    queue->rear = (queue->rear+1)%(queue->capacity);
    //pthread_mutex_unlock(&queue->rearLock);
    queue->dataRecordArray[queue->rear] = item;

    //pthread_mutex_lock(&queue->sizeLock);
    ++queue->size;
    //pthread_mutex_unlock(&queue->sizeLock);
}

// Function to remove an item from queue.  It changes front and size
int dequeue_data(DATA_QUEUE* queue)
{
    if (isEmpty_data(queue))
        return INT_MIN;
    DATA_RECORD* item = queue->dataRecordArray[queue->front];
    //pthread_mutex_lock(&queue->frontLock);
    queue->front=(queue->front+1)%(queue->capacity);
    //pthread_mutex_unlock(&queue->frontLock);

    //pthread_mutex_lock(&queue->sizeLock);
    queue->size = queue->size - 1;
    //pthread_mutex_unlock(&queue->sizeLock);
    //free(item);
    return INT_MAX;
}

// Function to get front of queue
DATA_RECORD* front_data(DATA_QUEUE* queue)
{
    if (isEmpty_data(queue))
        return INT_MIN;
    return queue->dataRecordArray[queue->front];
}

// Function to get rear of queue
DATA_RECORD* rear_data(DATA_QUEUE* queue)
{
    if (isEmpty_data(queue))
        return INT_MIN;
    return queue->dataRecordArray[queue->rear];
}

void displayDataOnLogger(DATA_QUEUE* queue, FILE* logfile) {
	printf("\n...................queue print..............\n");
	printf("\nsize:%d\n",queue->size);
	int i = queue->front;
	if (queue->front <= queue->rear) {
		for (; i <= queue->rear; i++) {
			/*fprintf(logfile, "\n\ndata record : %d\n", i);
			fprintf(logfile, "sessionId : %d\n",
					queue->dataRecordArray[i]->sessionId);
			fprintf(logfile, "dataSize : %d\n",
					queue->dataRecordArray[i]->size);
			logHexData(queue->dataRecordArray[i]->tcpPayload,
					queue->dataRecordArray[i]->size, logfile);*/

			logRawData(queue->dataRecordArray[i]->tcpPayload,
								queue->dataRecordArray[i]->size, logfile);
		}
	} else {
		for (; (i % (queue->capacity)) != (queue->rear); ++i) {
			/*fprintf(logfile, "data record : %d\n", i % (queue->capacity));
			fprintf(logfile, "sessionId : %d\n",
					queue->dataRecordArray[i % (queue->capacity)]->sessionId);
			fprintf(logfile, "dataSize : %d\n",
					queue->dataRecordArray[i % (queue->capacity)]->size);
			logHexData(queue->dataRecordArray[i % (queue->capacity)]->tcpPayload,
					queue->dataRecordArray[i % (queue->capacity)]->size,
					logfile);*/

			logRawData(queue->dataRecordArray[i % (queue->capacity)]->tcpPayload,
								queue->dataRecordArray[i % (queue->capacity)]->size,
								logfile);
		}
	}
	return;
}
