/*
 * wanTcpHandling.h
 *
 *  Created on: Oct 19, 2017
 *      Author: root
 */

#ifndef WANTCPHANDLING_H_
#define WANTCPHANDLING_H_



#endif /* WANTCPHANDLING_H_ */

#ifndef headerIncluded
#include "headers.h"
#endif

#ifndef constantsIncluded
#include <constants.h>
#endif

#ifndef structIncluded
#include "types.h"
#endif




int getSocket(char* ip,int port,SESSION* session);

int sendTo(char *data,int size,int sock);

void receiveFrom_ACNode(int sock,char* buffer);

int getWanSocket(char* ip,int port);



