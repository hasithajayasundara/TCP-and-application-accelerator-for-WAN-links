/*
 * deletedIdMIN_HEAP.h
 *
 *  Created on: Sep 12, 2017
 *      Author: deshanchathusanka
 */

#ifndef DELETEDIDMIN_HEAP_H_
#define DELETEDIDMIN_HEAP_H_



#endif /* DELETEDIDMIN_HEAP_H_ */

#ifndef hederIncluded
#include "headers.h"
#endif

#ifndef structIncluded
#include "types.h"
#endif


#define LCHILD(x) 2 * x + 1
#define RCHILD(x) 2 * x + 2
#define PARENT(x) (x-1)/2

/*heap initialization*/
void initMinHeap(MIN_HEAP* min_heap);

/*swap two heap nodes */
void swap(DELETED_ID_NODE* node1,DELETED_ID_NODE* node2);

/*heapify operation*/
void heapify(MIN_HEAP *min_heap, int i);

/*insert new deletedId to heap*/
void insertIdToDeletedHeap(MIN_HEAP* min_heap,int deletedId);

/*remove minimum deletedId from heap*/
int deleteIdFromDeletedHeap(MIN_HEAP* min_heap);

/*print heapArray*/
void printDeletedIdHeap(MIN_HEAP* min_heap);
