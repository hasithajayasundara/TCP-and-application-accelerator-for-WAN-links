/*
 * deletedIdMinHeap.c
 *
 *  Created on: Sep 13, 2017
 *      Author: deshanchathusanka
 */

#include <deletedIdMinHeap.h>

void initMinHeap(MIN_HEAP* min_heap){
	min_heap->heapArray=(DELETED_ID_NODE*)malloc(1000*sizeof(DELETED_ID_NODE));
    min_heap->size=0;

}

void swap(DELETED_ID_NODE* node1,DELETED_ID_NODE* node2)
{
    DELETED_ID_NODE temp;
    temp=*node1;
    *node1=*node2;
    *node2=temp;

}

void heapify(MIN_HEAP *min_heap, int i) {
    int smallest = (LCHILD(i) < min_heap->size && min_heap->heapArray[LCHILD(i)].sessionId < min_heap->heapArray[i].sessionId) ? LCHILD(i) : i ;
    if(RCHILD(i) < min_heap->size && min_heap->heapArray[RCHILD(i)].sessionId < min_heap->heapArray[smallest].sessionId) {
        smallest = RCHILD(i) ;
    }
    if(smallest != i) {
        swap(&(min_heap->heapArray[i]), &(min_heap->heapArray[smallest])) ;
        heapify(min_heap, smallest) ;
    }
}

void insertIdToDeletedHeap(MIN_HEAP* min_heap,int deletedId)
{
	//re allocation of heap array according to size
	if(min_heap->size)
	{
		min_heap->heapArray=realloc(min_heap->heapArray,(min_heap->size+1)*sizeof(DELETED_ID_NODE));
	}else{
		min_heap->heapArray=malloc(sizeof(DELETED_ID_NODE));
	}

	//create new deleted session Id
	DELETED_ID_NODE newNode;
	newNode.sessionId=deletedId;

	int i=(min_heap->size)++;
	while(i && deletedId < min_heap->heapArray[PARENT(i)].sessionId )
	{
		min_heap->heapArray[i] = min_heap->heapArray[PARENT(i)];
		i=PARENT(i);
	}

	min_heap->heapArray[i]=newNode;


}

int deleteIdFromDeletedHeap(MIN_HEAP* min_heap){

	int minimum;
	if(min_heap->size){
		minimum=min_heap->heapArray[0].sessionId;
		printf("................delete sesion :%d\n",minimum);
		min_heap->heapArray[0] = min_heap->heapArray[--(min_heap->size)];
		min_heap->heapArray = realloc(min_heap->heapArray,(min_heap->size)*sizeof(DELETED_ID_NODE));
		heapify(min_heap,0);
	}else{
		printf("Heap Empty : CAN NOT DELETE ANY ITEM\n");
		minimum=-1;
	}

	return minimum;
}

void printDeletedIdHeap(MIN_HEAP* min_heap){

	int i=0;
	for(;i<min_heap->size;++i){
		printf("%d ",min_heap->heapArray[i]);
	}
}

