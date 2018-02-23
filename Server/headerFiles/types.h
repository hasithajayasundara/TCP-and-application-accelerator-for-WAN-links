#ifndef headerIncluded
#include "headers.h"
#endif

#define structIncluded

enum FSM{CLOSED,LISTEN,SYN_RECEIVED,SYN_SENT,ESTABLISHED,FIN_WAIT_1,FIN_WAIT_2,CLOSING,TIME_WAIT,CLOSE_WAIT,LAST_ACK};

/*...................define session type.........................*/
struct thrqueue {
	void **data;
	size_t sz, n;
	size_t in, out;
	unsigned int block_enqueue : 1;
	unsigned int block_dequeue : 1;
	pthread_mutex_t mutex;
	pthread_cond_t notempty;
	pthread_cond_t notfull;
};

typedef struct thrqueue thrqueue_t;

typedef struct session {
	u_int32_t sourceIP;
	u_int32_t destIP;
	u_int16_t sourcePort;
	u_int16_t destPort;
	u_int32_t sessionId;
	u_int32_t seqNum;
	u_int32_t ackNum;
	struct dataQueue* queue;
	u_int32_t HS_uNum; // handshake unique number = handshake ACK seq_num + ack_seq
	u_int16_t rwnd;
	enum FSM state ;
	thrqueue_t* dqueue;
	u_int8_t sock;

} SESSION;

/*.................define destination socket type.................*/
typedef struct destSockAddr{
    u_int32_t destIP;
    u_int16_t destPort;
}DEST_SOCK_ADDR;

/*session data details type*/
typedef struct sessionDetail{
	u_int32_t sessionId;
	u_int16_t size;
	u_int8_t noOfSessions;

}SESSION_DETAIL;

typedef struct details{
	u_int32_t sessionId;
	u_int8_t noOfPackets;

}DETAIL;



/*.......................define record types...........................*/
typedef struct orderRecord {
	u_int32_t sessionId;
	u_int16_t location;
} ORDER_RECORD;

typedef struct dataRecord {
	u_int32_t sessionId;
	u_int16_t size;
	unsigned char *tcpPayload;

} DATA_RECORD;

typedef struct WanDataRecord {
	u_int16_t size;
	char *data;

} WAN_RECORD;



/*..........................define data types for BSTs..............................*/
//define deletedID node for deleted BST
typedef struct idNode{
    int id;
    struct node* left;
    struct node* right;
} ID_NODE;


/*.........................define queues..................................*/
//define LAN data order queue parameters
typedef struct orderQueue{
	int front, rear, capacity, size;
	struct orderRecord** orderRecordArray;
	pthread_mutex_t frontLock,rearLock,sizeLock;
} ORDER_QUEUE;

//define LAN data queue parameters
typedef struct dataQueue{
	int front, rear, capacity, size;
	struct dataRecord** dataRecordArray;
	pthread_mutex_t frontLock,rearLock,sizeLock;
} DATA_QUEUE;

//define WAN data queue parameters
typedef struct wanDataQueue{
	int front, rear, capacity, size;
	WAN_RECORD** dataArray;
	pthread_mutex_t frontLock,rearLock,sizeLock;
} WAN_QUEUE;

/*..........................define heap structure types...............................*/

typedef struct deletedIdNode{
    u_int16_t sessionId;
}DELETED_ID_NODE;

typedef struct MIN_HEAP{
    int size;
    DELETED_ID_NODE* heapArray;
} MIN_HEAP;


/**
 * Dynamic array
 */
typedef struct {
    unsigned char *array;
    size_t used;
    size_t size;
} Array;

struct nlist {
    struct nlist *next;
    char *name;
    char *defn;
};

typedef struct header{
	u_int32_t sessionId;
	u_int16_t size;
	u_int32_t destIP;
	u_int16_t destPort;
}HEADER;



