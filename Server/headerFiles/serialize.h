/*
 * serialize.h
 *
 *  Created on: Dec 27, 2017
 *      Author: root
 */

#ifndef SERIALIZE_H_
#define SERIALIZE_H_



#endif /* SERIALIZE_H_ */

#ifndef headerIncluded
#include "headers.h"
#endif

#include <writeInLogger.h>

int* getIntArray(int  intValue);

char* getHeaderInCharArray(HEADER* header,char* i){

	int count = 0;
	int y = 0;
	int* binData;
	u_int32_t n_32;
	u_int16_t n_16;

	int binCount = 0;
	int powCount = 0;
	n_32 = header->sessionId;
	binData=getIntArray(n_32);
	for (y = 3; y >= 0; y--) {
		if (binData[y] == 0) {
			 i[count++] = '0';
		} else {
			 i[count++] = (char)binData[y];
		}
	}

	binCount = 0;
	powCount = 0;
	n_16 = header->size;
	binData=getIntArray(n_16);
	for (y = 1; y >= 0; y--) {
		if (binData[y] == 0) {
			 i[count++] = '0';
		} else {
			 i[count++] = (char)binData[y];
		}
	}

	binCount = 0;
	powCount = 0;
	n_32 = header->destIP;
	binData=getIntArray(n_32);
	for (y = 3; y >= 0; y--) {
		if (binData[y] == 0) {
			 i[count++] = '0';
		} else {
			 i[count++] = (char)binData[y];
		}
	}

	binCount = 0;
	powCount = 0;
	n_16 = header->destPort;
	binData=getIntArray(n_16);
	for (y = 1; y >= 0; y--) {
		if (binData[y] == 0) {
			 i[count++] = '0';
		} else {
			 i[count++] = (char)binData[y];
		}
	}


	return i;
}


int* getIntArray(int  intValue){
	int binData[4] = {0,0,0,0};
	int binCount = 0;
	int powCount = 0;

	while (intValue != 0) {
	   binData[binCount] += (intValue % 2) * power(powCount++);
	   intValue = intValue / 2;
	   if (powCount == 8) {

				powCount = 0;
				binCount++;
		}
	}
	return binData;
}
