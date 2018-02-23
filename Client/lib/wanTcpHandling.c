/*
 * wanTcpHandling.c
 *
 *  Created on: Oct 19, 2017
 *      Author: root
 */

#include <wanTcpHandling.h>


int getSocket(char* ip,int port)
{
	int sock;
	struct sockaddr_in server;
	char message[1000] , server_reply[2000];

	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");

	server.sin_addr.s_addr = inet_addr(ip);
	server.sin_family = AF_INET;
	server.sin_port = htons( port );

	//Connect to remote server
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}

	puts("Connected\n");

    return sock;
}

int sendTo_ACNode(char *data,int size,int sock){
	//Send some data
	if( send(sock , data , size , 0) < 0)
	{
		puts("Send failed");
		return 0;
	}
	return 1;
}

void receiveFrom_ACNode(int sock,char* buffer){

	read( sock , buffer, 1024);
}


