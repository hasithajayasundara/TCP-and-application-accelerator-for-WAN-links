#ifndef headerIncluded
#include "headers.h"
#endif

#ifndef structIncluded
#include "types.h"
#endif

/*define variables*/
struct sockaddr_in source,dest;

void print_tcp_packet(unsigned char* Buffer, int Size, FILE *logfile);


void print_ip_header(unsigned char* Buffer, int Size,FILE *logfile);


void print_ethernet_header(unsigned char* Buffer, int Size,FILE *logfile);


void logHexData (unsigned char* data , int Size,FILE *logfile);

void logRawData (unsigned char* data , int Size,FILE *logfile);

void printHexData (unsigned char* data , int Size);

void print_arp(FILE *logfile,unsigned char *Buffer);


