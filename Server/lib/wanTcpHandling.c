/*
 * wanTcpHandling.c
 *
 *  Created on: Oct 19, 2017
 *      Author: root
 */

#include <wanTcpHandling.h>


int getSocket(char* ip,int port,SESSION* session)
{
	int sock;
	struct sockaddr_in server;
	struct sockaddr_in local_address;
	int addr_size = sizeof(local_address);



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

	getsockname(sock, &local_address, &addr_size);
	session->sourceIP=local_address.sin_addr.s_addr;
	session->sourcePort=ntohs(local_address.sin_port);

	printf("\n############Connected:::::::localIp:%s  localport:%d####\n",inet_ntoa(local_address.sin_addr),ntohs(local_address.sin_port));

    return sock;
}

int getWanSocket(char* ip,int port)
{
	int sock;
	struct sockaddr_in server;
	struct sockaddr_in local_address;
	int addr_size = sizeof(local_address);



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

    return sock;
}


int sendTo(char *data,int size,int sock){
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
