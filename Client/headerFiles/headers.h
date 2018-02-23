
#define headerIncluded

#define _GNU_SOURCE

#include<netinet/in.h>
#include<errno.h>
#include<netdb.h>
#include<stdio.h>
#include<stdlib.h>    //malloc
#include<string.h>    //strlen
#include<stddef.h>
#include<stdint.h>
#include<memory.h>
#include<stdbool.h>
#include<limits.h>
#include<leveldb/c.h>
#include <xxhash.h>

#include<netinet/tcp.h>   //Provides declarations for tcp header
#include<netinet/ip.h>    //Provides declarations for ip header
#include<netinet/if_ether.h>  //For ETH_P_ALL
#include<net/ethernet.h>  //For ether_header
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include<sys/time.h>
#include<sys/types.h>
#include<unistd.h>

#include <pthread.h>
#include <limits.h>
#include <pcap.h>
#include <pfring.h>
#include <errno.h>

#include <errno.h>

#define handle_error_en(en, msg) \
	   do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)







