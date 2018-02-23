/*

 + main.c
 *
 *  Created on: Aug 14, 2017
 *      Author: deshanchathusanka
 */

/*with deduplication*/
/*............................include custom header files....................................*/
#ifndef headerIncluded
#include <headers.h>
#endif

#include <writeInLogger.h>
#include <deletedIdMinHeap.h>
#include <deletedIdBST.h>

#ifndef SPOOF_H_
#include "spoof.h"
#endif

#ifndef structIncluded
#include <types.h>
#endif

#ifndef constantsIncluded
#include <constants.h>
#endif

#ifndef LandataQueueIncluded
#include <LanDataQueue.h>
#endif

#ifndef LanorderQueueIncluded
#include <LanOrderQueue.h>
#endif

#ifndef WANTCPHANDLING_H_
#include <wanTcpHandling.h>
#endif /* WANTCPHANDLING_H_ */

#ifndef ARP_STRUCT_H_
#include "arp_struct.h"
#endif

#ifndef ARP_REPLY_H_
#include "arp_reply.h"
#endif

#ifndef _NETINET_IN_H
#include <netinet/in.h>
#endif

#ifndef SERIALIZE_H_
#include <serialize.h>
#endif /* SERIALIZE_H_ */

#ifndef THRQUEUE_H
#include <thrqueue.h>
#endif

#include <SHA1.h>
#include <DynamicArray.h>
#include <xxhash.h>

/*......................prototypes.................................*/
/*functions*/
int isTcp(unsigned char* buffer, int size, pfring *ring_eth, FILE* f);
int sessionHandler(unsigned char* buffer, int size, struct iphdr *iph,
		struct tcphdr *tcph, pfring *ring_eth, FILE *f);
int getId(unsigned char* buffer, int size);
unsigned char* getTCPPayload(unsigned char* buffer, int size);
int getTCPPayloadSize(unsigned char* buffer, int size);
void stringCopy(unsigned char* dest, unsigned char* src, int size);
int isARP(unsigned char* buffer, int size);
char* concatCharArray(char* a, char*b, int size_a, int size_b);
int comp(unsigned char *i, unsigned char *max);
int chunkData(unsigned char *buffer, int n);
unsigned long long calcul_hash(const void* buffer, size_t length);
int retNum(int x, int count);
int pow2(int num);

/*threads*/
void *etherReadThread(void *vargp);
void *makeChunkThread(void *vargp);
void *processThread(void *vargp);
void *addHeaderThread(void *vargp);
void *sendToWanThread(void *vargp);

/*.....................variable definitions.........................*/
FILE *logfile, *queueLogfile_1, *queueLogfile_2, *orderQueueLogfile,
		*testLogfile, *testLogfile2, *sentPacketsFile;
struct dataRecord dataFromLanBuf[MAX_SESSIONS];
int firstRoundUpdate = -1, minimumId, freeSessionId;
struct deletedIdNode* root;
ORDER_QUEUE* orderQueue;
CHUNK_QUEUE* chunkQueue;
DEDUP_QUEUE* dedupQueue;
NETWORK_QUEUE* networkQueue;
MIN_HEAP* min_heap;
thrqueue_t* order;
char* ip;
int port;
int sock;
int cumSum;

//----LevelDB configuration----////
leveldb_t *db;
leveldb_options_t *options;
leveldb_readoptions_t *roptions;
leveldb_writeoptions_t *woptions;
char *err = NULL;
size_t read_len;

/*mutex parameters*/
pthread_mutex_t data_size_mutex;
pthread_mutex_t chunk_size_mutex;
pthread_mutex_t dedup_size_mutex;

/*create session buffer*/
struct session sessionBuf[MAX_SESSIONS];

/*create session exists buffer*/
int sessionExists[MAX_SESSIONS];

/*define constants*/
#define MAXBYTES2CAPTURE 2048

/*create fin list*/
int finList[MAX_SESSIONS] = { 0 };

/*ethernet Thread parameters*/
#define MSS 1520
#define ITR 10000
#define MAX_PKT_LEN 1536
u_int32_t flags = 0;

char *device_eth = "eth0";
char *device_inet = "stack:eth1";
char* hashName = "HashTable_140";

int main() {

	/*create chunk queue*/
	chunkQueue = (CHUNK_QUEUE*) malloc(sizeof(CHUNK_QUEUE));
	Init_chunk(chunkQueue, CHUNK_CAPACITY);

	/*create dedup queue*/
	dedupQueue = (DEDUP_QUEUE*) malloc(sizeof(DEDUP_QUEUE));
	Init_dedup(dedupQueue, DEDUP_CAPACITY);

	/*create network queue*/
	networkQueue = (NETWORK_QUEUE*) malloc(sizeof(NETWORK_QUEUE));
	Init_network(networkQueue, NETWORK_CAPACITY);

	/*configure WAN socket*/
	ip = "10.8.145.219";
	port = 10000;
	sock = getSocket(ip, port);

	//----LevelDB configuration----////
	time_t t;
	srand((unsigned) time(&t));
	int n = rand() % 1000000;
	char str[10] = { '\0' };
	sprintf(str, "%d", n);
	options = leveldb_options_create();
	leveldb_options_set_create_if_missing(options, 1);
	db = leveldb_open(options, str, &err);
	if (err != NULL ) {
		printf("Database initiation failed.\n");
	} else {
		printf("Database %s Initialized.\n", str);
		leveldb_free(err);
		err = NULL;
	}
	//Initilize writing
	woptions = leveldb_writeoptions_create();
	//Initializing reading
	roptions = leveldb_readoptions_create();
	//----LevelDB configuration End----////

	/*initialize order queue*/
	orderQueue = (ORDER_QUEUE*) malloc(sizeof(ORDER_QUEUE));
	Init_order(orderQueue, MAX_WAITING_PACKETS);

	/*set ids for threads*/
	pthread_t etherReadThread_id, makeChunkThread_id, processThread_id,
			addHeaderThread_id, sendToWanThread_id;

	/*set attributes for threads*/
	pthread_attr_t attr;
	cpu_set_t cpus;
	pthread_attr_init(&attr);

	/*CPU_ZERO(&cpus);
	 CPU_SET(0, &cpus);
	 pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);*/
	pthread_create(&etherReadThread_id, &attr, &etherReadThread, NULL );

	/*CPU_ZERO(&cpus);
	 CPU_SET(1, &cpus);
	 pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);*/
	pthread_create(&makeChunkThread_id, &attr, &makeChunkThread, NULL );

	CPU_ZERO(&cpus);
	CPU_SET(2, &cpus);
	pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);
	pthread_create(&processThread_id, &attr, &processThread, NULL );

	/*CPU_ZERO(&cpus);
	 CPU_SET(3, &cpus);
	 pthread_attr_setaffinity_np(&attr, sizeof(cpu_set_t), &cpus);*/
	pthread_create(&addHeaderThread_id, &attr, &addHeaderThread, NULL );

	//pthread_create(&sendToWanThread_id, &attr, &sendToWanThread, NULL );

	/*wait until ethernet thread exits*/
	pthread_join(etherReadThread_id, NULL );
	pthread_join(makeChunkThread_id, NULL );
	pthread_join(processThread_id, NULL );
	pthread_join(addHeaderThread_id, NULL );
	//pthread_join(sendToWanThread_id, NULL );

	return 0;
}
/*..............................Thread definitions............................................*/

void *etherReadThread(void *var) {

	printf(
			"\n########################### EtherReadThread Running #################################\n");

	/*create order queue*/
	/*orderQueue=(ORDER_QUEUE* )malloc(sizeof(ORDER_QUEUE));
	 Init_order(orderQueue, MAX_WAITING_PACKETS);*/

	/*packet read variables*/
	int saddr_size, data_size, k, tcpPayloadSize, sessionId, counter = 0;
	struct sockaddr saddr;
	unsigned char* tcpPayloadBuffer;
	DATA_RECORD* tempDataRecord;
	ORDER_RECORD* tempOrderRecord;
	//unsigned char *buffer = (unsigned char *) malloc(MSS);

	/*min heap initialization*/
	min_heap = (MIN_HEAP*) malloc(sizeof(MIN_HEAP));
	initMinHeap(min_heap);

	//log files
	//logfile = fopen("log.txt", "w");
	sentPacketsFile = fopen("sentPackets.txt", "w");
	//FILE* sendDataLogfile = fopen("sendDataLogfile.txt", "w");

	//check for log file open errors
	if (logfile == NULL ) {
		printf("Unable to create log.txt file.");
	}
	if (sentPacketsFile == NULL ) {
		printf("Unable to create sentPacketsFile.txt");
	}

	int packet_size = 0;
	char* pkb;
	int eth_ifindex, inet_ifindex;
	pfring *ring_eth, *ring_inet;
	int bind_core = -1;
	char *bpfFilter = NULL;
	u_int16_t watermark = 1;

	struct pfring_pkthdr hdr, fdr;
	ring_eth = pfring_open(device_eth, MAX_PKT_LEN,
			PF_RING_PROMISC | PF_RING_LONG_HEADER | 0);
	if (ring_eth == NULL ) {
		printf("error opening \n");
		fprintf(stderr,
				"pfring_open error [%s] (pf_ring not loaded or interface %s is down ?)\n",
				strerror(errno), device_eth);
		exit(0);
	}

	pfring_set_application_name(ring_eth, "pfbridge-eth");
	pfring_set_direction(ring_eth, 0);
	pfring_set_socket_mode(ring_eth, 0);
	pfring_set_poll_watermark(ring_eth, watermark);
	pfring_get_bound_device_ifindex(ring_eth, &eth_ifindex);

	// adding BPF filter

	if (bpfFilter != NULL ) {
		int rc = pfring_set_bpf_filter(ring_eth, bpfFilter);
		if (rc != 0)
			printf("pfring_set_bpf_filter(%s) returned %d\n", bpfFilter, rc);
		else
			printf("Successfully set BPF filter '%s'\n", bpfFilter);
	}

	ring_inet = pfring_open(device_inet, MAX_PKT_LEN,
			PF_RING_PROMISC | PF_RING_LONG_HEADER);
	if (ring_inet == NULL ) {
		printf("error opening \n");
		fprintf(stderr,
				"pfring_open error [%s] (pf_ring not loaded or interface %s is down ?)\n",
				strerror(errno), device_inet);
		pfring_close(ring_eth);
		exit(0);
	}

	pfring_set_application_name(ring_inet, "pfbridge-b");
	pfring_set_socket_mode(ring_inet, 0);
	pfring_get_bound_device_ifindex(ring_inet, &inet_ifindex);

	if (pfring_enable_ring(ring_eth) != 0) {
		printf("failed to enable reader ring  \n");
		pfring_close(ring_eth);
		pfring_close(ring_inet);
		exit(0);
	} else if (pfring_enable_ring(ring_inet) != 0) {
		printf("failed to enable writer ring \n");
		pfring_close(ring_eth);
		pfring_close(ring_inet);
		exit(0);
	} else {

		cumSum = 0;
		//char * syn = send_syn();
		while (1) {
			packet_size = pfring_recv(ring_eth, &pkb, 0, &hdr, 1);
			if (packet_size > 0) {
				struct ethhdr *ethH = (struct ethhdr *) pkb;
				//printf("Received ethernet frame \n");
				if (ethH->h_proto == 8) {
					struct iphdr *iph = (struct iphdr *) (pkb
							+ sizeof(struct ethhdr));
					int iphdrlen = iph->ihl * 4; //set ip header length
					struct tcphdr *tcph = (struct tcphdr*) (pkb + iphdrlen
							+ sizeof(struct ethhdr)); //set tcp header

					//check for ip_checksum

					if (verify_ip_checksum(iph)) {

						if (isTcp(pkb, data_size, ring_eth, sentPacketsFile)) //check whether the packet is TCP
								{
							/*continue loop is received packet is SYN or FYN*/
							if (isSyn(pkb, data_size)
									|| isFyn(pkb, data_size)) {
								//print_tcp_packet(pkb, data_size, logfile);
								continue;
							}

							tcpPayloadBuffer = getTCPPayload(pkb, data_size);
							tcpPayloadSize = getTCPPayloadSize(pkb, data_size);
							//print_tcp_packet(pkb, data_size, logfile);
							sessionId = getId(pkb, data_size);

							if (sessionBuf[sessionId].queue
									&& sessionExists[sessionId]) {
								//..................data record processing.......................//
								//create a new data Record
								tempDataRecord = (DATA_RECORD*) malloc(
										sizeof(DATA_RECORD));
								tempDataRecord->sessionId = sessionId;
								tempDataRecord->size = tcpPayloadSize;
								tempDataRecord->tcpPayload =
										(unsigned char*) malloc(
												(tcpPayloadSize + 1)
														* sizeof(unsigned char));

								memcpy(tempDataRecord->tcpPayload,
										tcpPayloadBuffer, tcpPayloadSize);
								*(tempDataRecord->tcpPayload + tcpPayloadSize) =
										'\0';

								//enqueue the data Record
								pthread_mutex_lock(&data_size_mutex);
								enqueue_data(sessionBuf[sessionId].queue,
										tempDataRecord);
								pthread_mutex_unlock(&data_size_mutex);

							}

						}

					}

					// else cpndition ip checksum mismatch discard packet

				}

			}

		}

		pfring_close(ring_eth);
		pfring_close(ring_inet);
	}

	pthread_exit(0);

}

void *makeChunkThread(void *vargp) {
	printf(
			"\n########################### makeChunkThread Running #################################\n");

	DATA_QUEUE* queue;
	DATA_RECORD* temp;
	bool isOver = false;
	unsigned char* tmpDataChunk;
	u_int32_t dataLen;
	unsigned char *dataLoad;
	int sessionId = 0;
	int bytesInChunk, counter = 0;

	/*create first chunk*/
	tmpDataChunk = (char*) malloc(FILE_CONST + 1);
	bytesInChunk = 0;

	START_CHUNK: while (1) {

		if (sessionExists[sessionId] == 0)
			continue;
		/*now session Exists*/

		queue = sessionBuf[sessionId].queue;

		if (isFull_chunk(chunkQueue))
			continue;

		/*now chunk queue is not full*/

		while (1) {
			if ((queue->size) > 0) {
				temp = queue->dataRecordArray[queue->front];
				dataLoad = temp->tcpPayload;
				dataLen = temp->size;
				int func;
				for (func = 0; func < dataLen; func++) {
					tmpDataChunk[bytesInChunk++] = dataLoad[func];

					if (bytesInChunk == FILE_CONST) {
						tmpDataChunk[bytesInChunk] = '\0';
						if (func < dataLen - 1) {
							/*there are charators that are not inserted to currect chunk*/
							pthread_mutex_lock(&chunk_size_mutex);
							enqueue_chunk(chunkQueue, tmpDataChunk);
							pthread_mutex_unlock(&chunk_size_mutex);
							tmpDataChunk = (char*) malloc(FILE_CONST);
							bytesInChunk = 0;
							int k;
							for (k = func + 1; k < dataLen; ++k) {
								tmpDataChunk[bytesInChunk++] = dataLoad[k];
							}
							pthread_mutex_lock(&data_size_mutex);
							dequeue_data(queue);
							pthread_mutex_unlock(&data_size_mutex);
							free(temp);
							goto START_CHUNK;
						}

						/*all the charators are inserted into current chunk*/
						pthread_mutex_lock(&chunk_size_mutex);
						enqueue_chunk(chunkQueue, tmpDataChunk);
						pthread_mutex_unlock(&chunk_size_mutex);

						pthread_mutex_lock(&data_size_mutex);
						dequeue_data(queue);
						pthread_mutex_unlock(&data_size_mutex);

						free(temp);
						tmpDataChunk = (char*) malloc(FILE_CONST);
						bytesInChunk = 0;
						goto START_CHUNK;
					}
				}

				/*dequeue and free data from raw queue*/
				pthread_mutex_lock(&data_size_mutex);
				dequeue_data(queue);
				pthread_mutex_unlock(&data_size_mutex);
				//free(dataLoad);
				free(temp);
			}

		}

	}
	pthread_exit(0);
}

void *processThread(void *vargp) {

	printf(
			"\n########################### processThread Running #################################\n");
	FILE* f = fopen("process.txt", "a");
	FILE* fQ = fopen("processQ.txt", "a");

	unsigned char *dupArray;
	unsigned char *freePtr;
	DEDUP_RECORD* dedupRecord;
	int func = 0;

	while (1) {

		if (isFull_dedup(dedupQueue))
			continue;

		/*now dedup queue is not full*/

		if (chunkQueue->size < 1)
			continue;

		/*now chunk queue is not empty*/

		dupArray = chunkQueue->dataChunkArray[chunkQueue->front];
		freePtr = chunkQueue->dataChunkArray[chunkQueue->front];

		/*create new dedup record*/
		dedupRecord = (DEDUP_RECORD*) malloc(sizeof(DEDUP_RECORD));

		int dataBoundaries = 0;
		Array cArray;
		initArray(&cArray, STORAGE_CONST);
		int dupCount = 0;
		int *positionsArray = (int *) malloc(sizeof(int) * 500);
		while (1) {
			int length = strlen(dupArray);
			int count = 0;
			if (length < (windowSize + 8)) {
				char xxHashBuf[HASH_LENGTH];
				sprintf(xxHashBuf, "%llu", calcul_hash(dupArray, length));
				int xxLen = strlen(xxHashBuf);
				while (xxLen != 20) {
					xxHashBuf[xxLen] = '*';
					xxLen++;
				}
				char *result = leveldb_get(db, roptions, xxHashBuf, HASH_LENGTH,
						&read_len, &err);
				if (err != NULL ) {
					printf("Read Hash Failed Phase 1.\n");
				} else {
					leveldb_free(err);
				}
				if (result == NULL ) {
					if (length > HASH_LENGTH + 4) {
						leveldb_put(db, woptions, xxHashBuf, SHA_LENGTH,dupArray, length, &err);
						if (err != NULL ) {
							printf("Write Hash Failed Phase 1.\n");
						} else {
							leveldb_free(err);
							err = NULL;
						}
					}
					int dataArrayCount = 0;
					for (dataArrayCount = 0; dataArrayCount < length;dataArrayCount++) {
						insertArray(&cArray, *(dupArray + dataArrayCount));
					}
				} else {
					int j = 0;
					*(positionsArray+dupCount) = dataBoundaries;
					dupCount++;
					for (j = 0; j < HASH_LENGTH; j++) {
						insertArray(&cArray, xxHashBuf[j]);
					}
				}
				int clean = 0;
				break;
			}

			//calculate boundary
			int boundary = chunkData(dupArray, length);
			char xxHashBuf[HASH_LENGTH];
			sprintf(xxHashBuf, "%llu", calcul_hash(dupArray, boundary + 1));
			int xxLen = strlen(xxHashBuf);
			while (xxLen != 20) {
				xxHashBuf[xxLen] = '*';
				xxLen++;
			}
			//check whether the hash exist
			char *result = leveldb_get(db, roptions, xxHashBuf, HASH_LENGTH,
					&read_len, &err);
			if (err != NULL ) {
				printf("Read Hash Failed Phase 2.\n");
			} else {
				leveldb_free(err);
			}
			if (result == NULL ) {
				if ((boundary + 1) > HASH_LENGTH + 4) {
					leveldb_put(db, woptions, xxHashBuf, HASH_LENGTH, dupArray,
							boundary + 1, &err);
					if (err != NULL ) {
						printf("Write Hash Failed Phase 2.\n");
					} else {
						leveldb_free(err);
					}
				}
				int k;
				for (k = 0; k < boundary + 1; k++) {
					insertArray(&cArray, *(dupArray + k));
				}
				dataBoundaries += boundary + 1;
			} else {
				int j = 0;
				*(positionsArray + dupCount) = dataBoundaries;
				dupCount++;
				for (j = 0; j < HASH_LENGTH; j++) {
					insertArray(&cArray, xxHashBuf[j]);
				}
				dataBoundaries += HASH_LENGTH;
			}

			//break condition
			if (boundary == length) {
				break;
			}

			//increment string pointer
			dupArray = dupArray + boundary + 1;
		}

		insertArray(&cArray, '\0');

		int finalDataLength = strlen(cArray.array) + dupCount * 4 + 6;
		int dedupLength = dupCount;

		/*if(dedupLength!=0)
		 func++;
		 printf("%d : %d : %d ",finalDataLength, dedupLength,func);
		 */

		dedupRecord->tcpPayload = cArray.array;
		dedupRecord->finalDataLength = finalDataLength;
		dedupRecord->dedupLength = dedupLength;
		dedupRecord->positionsArray = positionsArray;

		/*enqueue dedup record*/
		fprintf(f, cArray.array);
		pthread_mutex_lock(&dedup_size_mutex);
		if (enqueue_dedup(dedupQueue, dedupRecord) < 0) {
			printf("Queue full\n");
		}
		pthread_mutex_unlock(&dedup_size_mutex);

		/*dequeue and free data from chunk queue*/
		pthread_mutex_lock(&chunk_size_mutex);
		dequeue_chunk(chunkQueue);
		pthread_mutex_unlock(&chunk_size_mutex);

		//free(freePtr);

	}
	pthread_exit(0);
}
void *addHeaderThread(void *vargp) {

	printf(
			"\n########################### addHeaderThread Running #################################\n");
	FILE* addHeader = fopen("addHeader.txt", "a");

	DEDUP_RECORD* tmpDedupData;
	DEDUP_RECORD* freePtr;
	int finalDataLength;
	int dedupLength, dupcount;
	int* positionsArray;
	unsigned char* dedupCharArray;
	int cnt;

	int bclock = clock();
	bool start = false;
	int fuck =0;
	while (1) {
		if (isEmpty_dedup(dedupQueue))
			continue;

		/*bclock = clock();
		 start = true;*/
		/*now chunk queue is not empty*/
		tmpDedupData = dedupQueue->dedupArray[dedupQueue->front];

		finalDataLength = tmpDedupData->finalDataLength;
		dedupLength = tmpDedupData->dedupLength;
		dupcount = dedupLength;
		positionsArray = tmpDedupData->positionsArray;
		dedupCharArray = tmpDedupData->tcpPayload;

		//if(strlen(dedupCharArray) != 102400)
		//printf("%d %d\n",finalDataLength,dedupLength);

		//data to send
		Array networkBuffer;
		initArray(&networkBuffer, STORAGE_CONST);
		insertArray(&networkBuffer, '$');
		insertArray(&networkBuffer, '$');
		int binData[4] = { 0, 0, 0, 0 };
		int binCount = 0;
		int powCount = 0;

		while (finalDataLength != 0) {
			binData[binCount] += (finalDataLength % 2) * power(powCount++);
			finalDataLength /= 2;
			if (powCount == 8) {
				powCount = 0;
				binCount++;
			}
		}

		int t;
		for (t = 3; t >= 0; t--) {
			insertArray(&networkBuffer, (unsigned char) binData[t]);
		}

		binData[0] = 0;
		binData[1] = 0;
		binData[2] = 0;
		binData[3] = 0;
		binCount = 0;
		powCount = 0;

		while (dedupLength != 0) {
			binData[binCount] += (dedupLength % 2) * power(powCount++);
			dedupLength /= 2;
			if (powCount == 8) {
				powCount = 0;
				binCount++;
			}
		}

		for (t = 1; t >= 0; t--) {
			insertArray(&networkBuffer, (unsigned char) binData[t]);

		}

		int m;
		int kake = 0;
		//printf("Dedup Length %d \n",dupcount);
		for (m = 0; m < dupcount; m++) {
			int initNum = positionsArray[m];
			//printf(" %d - ",initNum);
			binData[0] = 0;
			binData[1] = 0;
			binData[2] = 0;
			binData[3] = 0;
			binCount = 0;
			powCount = 0;
			while (initNum != 0) {
				binData[binCount] += (initNum % 2) * power(powCount++);
				initNum /= 2;
				if (powCount == 8) {
					powCount = 0;
					binCount++;
				}
			}
			int y = 0;
			for (t = 3; t >= 0; t--) {
				insertArray(&networkBuffer, (unsigned char) binData[t]);
				kake += retNum(binData[t], y++);
			}

		}
		//printf("%d\n", cnt++);
		int arrayCount = 0;
		while (*(dedupCharArray + arrayCount) != '\0') {
			insertArray(&networkBuffer, *(dedupCharArray + arrayCount));
			arrayCount++;
		}
		insertArray(&networkBuffer, '\0');
		//printf("cnt: %d \n",cnt++);
		//printf("chu:%d dup:%d add:%d\n",chunkQueue->size,dedupQueue->size,cnt++);
		logRawData(networkBuffer.array, networkBuffer.used, addHeader);
		sendTo_ACNode(networkBuffer.array, networkBuffer.used, sock);
		printf("%d\n",fuck++);
		//sleep(1);
		/*while (1) {
		 if (!isFull_network(networkQueue)) {
		 enqueue_network(networkQueue, &networkBuffer);
		 logRawData(networkBuffer.array,networkBuffer.used,addHeader);
		 printf("%d %d %d %d\n", sessionBuf[0].queue->size,chunkQueue->size,dedupQueue->size,networkQueue->size);
		 break;
		 }
		 }*/

		/*dequeue and free data from dedup queue*/
		pthread_mutex_lock(&dedup_size_mutex);
		dequeue_dedup(dedupQueue);
		pthread_mutex_unlock(&dedup_size_mutex);
		freeArray(&networkBuffer);
		free(tmpDedupData);
		free(positionsArray);

	}

	pthread_exit(0);
}

/*void *sendToWanThread(void *vargp) {

 printf(
 "\n########################### sendToWanThread Running #################################\n");

 FILE* sendToWan = fopen("sendToWan.txt", "a");

 Array *networkArray;

 while (1) {
 if (isEmpty_network(networkQueue))
 continue;

 now network queue is not empty

 networkArray = networkQueue->networkArray[networkQueue->front];
 //logRawData(networkArray->array, networkArray->used, sendToWan);
 sendTo_ACNode(networkArray->array, networkArray->used, sock);

 dequeue and free data from network queue
 dequeue_network(networkQueue);
 freeArray(networkArray);

 }
 pthread_exit(0);
 }*/

/*..............................function definitions............................................*/

void arrcmp(unsigned char arr1[], unsigned char arr2[], int size) {
	int i = 0;
	int match = 0;

	for (i = 0; i < size; ++i) {
		if (arr1[i] != arr2[i]) {
			match = 1;
			break;
		}
	}

}

int isTcp(unsigned char* buffer, int size, pfring *ring_eth, FILE* f) {
	int iphdrlen;
	struct tcphdr *tcph;
	struct iphdr *iph = (struct iphdr*) (buffer + sizeof(struct ethhdr));
	iphdrlen = iph->ihl * 4;						//set ip header length

	//check for tcp checksum if incorrect discard packet

	if (iph->protocol == 6) //Check the Protocol and do accordingly.....
			{
		tcph = (struct tcphdr*) (buffer + iphdrlen + sizeof(struct ethhdr)); //set tcp header
		//if(check_csum(buffer)){
		if (sessionHandler(buffer, size, iph, tcph, ring_eth, f)) {
			return 1;
		}
		//}

		// else{ //discard packet};
	}

	return 0;
}

int isSyn(unsigned char *buffer, int size) {
	struct iphdr *iph = (struct iphdr *) (buffer + sizeof(struct ethhdr));
	int iphdrlen = iph->ihl * 4;    	//set ip header length
	struct tcphdr *tcph = (struct tcphdr *) (buffer + iphdrlen
			+ sizeof(struct ethhdr));    	//set tcp header

	if (tcph->syn == 1)
		return 1;
	return 0;

}

int isFyn(unsigned char *buffer, int size) {
	struct iphdr *iph = (struct iphdr *) (buffer + sizeof(struct ethhdr));
	int iphdrlen = iph->ihl * 4;    	//set ip header length
	struct tcphdr *tcph = (struct tcphdr *) (buffer + iphdrlen
			+ sizeof(struct ethhdr));    	//set tcp header

	if (tcph->fin == 1)
		return 1;
	return 0;

}

int isARP(unsigned char* buffer, int size) {

	struct ethhdr *eth = (struct ethhdr *) buffer;

	if (eth->h_proto == 1544) //Check the Protocol and do accordingly.....
		return 1;

	return 0;
}

int isAck(unsigned char* buffer, int size) {
	struct iphdr *iph = (struct iphdr *) (buffer + sizeof(struct ethhdr));
	int iphdrlen = iph->ihl * 4; //set ip header length
	struct tcphdr *tcph = (struct tcphdr *) (buffer + iphdrlen
			+ sizeof(struct ethhdr)); //set tcp header

	if (tcph->ack == 1 && tcph->syn == 0 && tcph->rst == 0)
		return 1;
	return 0;
}

int isRST(unsigned char* buffer, int size) {
	struct iphdr *iph = (struct iphdr *) (buffer + sizeof(struct ethhdr));
	int iphdrlen = iph->ihl * 4; //set ip header length
	struct tcphdr *tcph = (struct tcphdr *) (buffer + iphdrlen
			+ sizeof(struct ethhdr)); //set tcp header

	if (tcph->rst == 1)
		return 1;
	return 0;
}

int sessionHandler(unsigned char *buffer, int size, struct iphdr *iph,
		struct tcphdr *tcph, pfring *ring_eth, FILE* f) {
	int freeId, sessionId;
	DATA_RECORD* tempDataRecord;
	ORDER_RECORD tempOrderRecord;
	DEST_SOCK_ADDR* tempSockAddr;

	//filter only incoming packets from LAN//
	memset(&source, 0, sizeof(source));
	source.sin_addr.s_addr = iph->saddr;
	char* ip = inet_ntoa(source.sin_addr);
	if (strcmp(ip, "192.168.1.2") != 0)
		return 0;

	if (isSyn(buffer, size)) { /*#####################################SYN##########################################*/
		int calculatedId = getId(buffer, size);
		printf("\n---syn--- calculated:%d", calculatedId);
		if (calculatedId == -1) {

			/*...................create new session..................................*/

			//minimumId = getMinimumIdFromDeletedSessions();
			minimumId = deleteIdFromDeletedHeap(min_heap);
			printf("\nminimum:%d", minimumId);
			if (minimumId == -1) {
				/*there is no deleted sessions upto now*/
				if (firstRoundUpdate + 1 < MAX_SESSIONS) {
					/*session buffer is not full in first round*/
					++firstRoundUpdate;

					/*define session Id*/
					sessionId = firstRoundUpdate;

					/*add new session to session Buffer*/
					sessionBuf[firstRoundUpdate].destIP = iph->daddr;
					sessionBuf[firstRoundUpdate].sourceIP = iph->saddr;
					sessionBuf[firstRoundUpdate].sourcePort = tcph->source;
					sessionBuf[firstRoundUpdate].destPort = tcph->dest;
					sessionBuf[firstRoundUpdate].sessionId = firstRoundUpdate;

					/*create a data queue for created session*/
					sessionBuf[firstRoundUpdate].queue = (DATA_QUEUE *) malloc(
							sizeof(DATA_QUEUE));
					Init_data(sessionBuf[firstRoundUpdate].queue,
							MAX_RECORDS_PER_SESSION);
					//sessionBuf[firstRoundUpdate].dqueue=thrqueue_new(MAX_RECORDS_PER_SESSION);

					/*add opposite session to session Buffer*/
					sessionBuf[(MAX_SESSIONS - 1) - firstRoundUpdate].destIP =
							iph->saddr;
					sessionBuf[(MAX_SESSIONS - 1) - firstRoundUpdate].sourceIP =
							iph->daddr;
					sessionBuf[(MAX_SESSIONS - 1) - firstRoundUpdate].sourcePort =
							tcph->dest;
					sessionBuf[(MAX_SESSIONS - 1) - firstRoundUpdate].destPort =
							tcph->source;
					sessionBuf[(MAX_SESSIONS - 1) - firstRoundUpdate].sessionId =
							(MAX_SESSIONS - 1) - firstRoundUpdate;

					/*create a data queue for created opposite session*/
					sessionBuf[(MAX_SESSIONS - 1) - firstRoundUpdate].queue =
							(DATA_QUEUE *) malloc(sizeof(DATA_QUEUE));
					Init_data(
							sessionBuf[(MAX_SESSIONS - 1) - firstRoundUpdate].queue,
							MAX_RECORDS_PER_SESSION);
					//sessionBuf[(MAX_SESSIONS-1)-firstRoundUpdate].dqueue=thrqueue_new(MAX_RECORDS_PER_SESSION);

				} else {
					/*session buffer is full*/
					printf("No of sessions that can be handled is exceeded\n");
				}

			} else {

				/*.............there are deleted sessions upto now.........................*/

				/*remove new session from fin list*/
				finList[minimumId] = 0;

				/*define session Id*/
				sessionId = minimumId;

				/*add new session to session Buffer*/
				sessionBuf[minimumId].destIP = iph->daddr;
				sessionBuf[minimumId].sourceIP = iph->saddr;
				sessionBuf[minimumId].sourcePort = tcph->source;
				sessionBuf[minimumId].destPort = tcph->dest;
				sessionBuf[minimumId].sessionId = minimumId;

				/*create a data queue for created session*/
				sessionBuf[minimumId].queue = (DATA_QUEUE *) malloc(
						sizeof(DATA_QUEUE));
				Init_data(sessionBuf[minimumId].queue, MAX_RECORDS_PER_SESSION);
				//sessionBuf[minimumId].dqueue=thrqueue_new(MAX_RECORDS_PER_SESSION);

				/*add opposite session to session Buffer*/
				sessionBuf[(MAX_SESSIONS - 1) - minimumId].destIP = iph->saddr;
				sessionBuf[(MAX_SESSIONS - 1) - minimumId].sourceIP =
						iph->daddr;
				sessionBuf[(MAX_SESSIONS - 1) - minimumId].sourcePort =
						tcph->dest;
				sessionBuf[(MAX_SESSIONS - 1) - minimumId].destPort =
						tcph->source;
				sessionBuf[(MAX_SESSIONS - 1) - minimumId].sessionId =
						(MAX_SESSIONS - 1) - minimumId;

				/*create a data queue for created opposite session*/
				sessionBuf[(MAX_SESSIONS - 1) - minimumId].queue =
						(DATA_QUEUE *) malloc(sizeof(DATA_QUEUE));
				Init_data(sessionBuf[(MAX_SESSIONS - 1) - minimumId].queue,
						MAX_RECORDS_PER_SESSION);
				//sessionBuf[(MAX_SESSIONS-1)-minimumId].dqueue=thrqueue_new(MAX_RECORDS_PER_SESSION);

				/*...............delete element from deletedBST....................*/
				//root = deleteNode(root, minimumId);
			}

			/*.............send reply to SYN.......................*/
			if (tcph->ack == 0) {
				sessionBuf[sessionId].seqNum = ntohl(tcph->seq);
				sessionBuf[sessionId].ackNum = ntohl(tcph->ack_seq);
				u_int32_t seq = 1000000 + rand() % 90000;
				char * synack = syn_ack(buffer, seq);
				pfring_send(ring_eth, synack,
						sizeof(struct ethhdr) + sizeof(struct iphdr)
								+ sizeof(struct tcphdr), 0);
				free(synack);

				sessionBuf[(MAX_SESSIONS - 1) - sessionId].seqNum = seq;
				sessionBuf[(MAX_SESSIONS - 1) - sessionId].ackNum = ntohl(
						tcph->seq) + 1;

				//update client seq_num and ack_num
				sessionBuf[sessionId].seqNum++;
				sessionBuf[sessionId].ackNum = seq + 1;
				sessionBuf[sessionId].HS_uNum = sessionBuf[sessionId].seqNum
						+ sessionBuf[sessionId].ackNum;
				print_tcp_packet(synack,
						sizeof(struct ethhdr) + sizeof(struct iphdr)
								+ sizeof(struct tcphdr), sentPacketsFile);

				/*change TCP state to SYN_RECIEVED */
				sessionBuf[sessionId].state = SYN_RECEIVED;
				sessionBuf[(MAX_SESSIONS - 1) - sessionId].state = SYN_RECEIVED;
			} else {
				// still need to work on this one
				sessionBuf[sessionId].seqNum = tcph->seq; // assume weh have already assigned our seq of syn to ack of cient struct
				sessionBuf[(MAX_SESSIONS - 1) - sessionId].ackNum = tcph->seq;
				sessionBuf[(MAX_SESSIONS - 1) - sessionId].seqNum++;

				sessionBuf[(MAX_SESSIONS - 1) - sessionId].ackNum++;
				char * ack = send_ack(buffer,
						sessionBuf[(MAX_SESSIONS - 1) - sessionId].seqNum,
						sessionBuf[(MAX_SESSIONS - 1) - sessionId].ackNum);
				pfring_send(ring_eth, ack,
						sizeof(struct ethhdr) + sizeof(struct iphdr)
								+ sizeof(struct tcphdr), 0);
				free(ack);

				/*change state to ESTABLISHED */
				sessionBuf[sessionId].state = ESTABLISHED;
				sessionBuf[(MAX_SESSIONS - 1) - sessionId].state = ESTABLISHED;
			}

			printf("\nsession %d is created\n", sessionId);
			sessionExists[sessionId] = 1;

		}

		/*do nothing for duplicate acks*/

	} else if (isFyn(buffer, size)) { /*#####################################FYN##########################################*/
		freeId = getId(buffer, size);
		if (freeId == -1)
			return 0;

		printf("\n################## Fin:%d ##################\n", freeId);
		//finList[freeId] = 1;
		//sessionExists[sessionId] = 0;

		if (sessionBuf[freeId].state == ESTABLISHED) {

			sessionBuf[(MAX_SESSIONS - 1) - freeId].ackNum++; //assume FIN,ACK has zero length of data
			sessionBuf[(MAX_SESSIONS - 1) - freeId].seqNum++; // assuming we are not sending data

			char * ack = send_ack(buffer,
					sessionBuf[(MAX_SESSIONS - 1) - freeId].seqNum,
					sessionBuf[(MAX_SESSIONS - 1) - freeId].ackNum);
			pfring_send(ring_eth, ack,
					sizeof(struct ethhdr) + sizeof(struct iphdr)
							+ sizeof(struct tcphdr), 0);
			/*print_tcp_packet(ack,
			 sizeof(struct ethhdr) + sizeof(struct iphdr)
			 + sizeof(struct tcphdr), logfile);*/
			free(ack);

			sessionBuf[freeId].seqNum++; // assume FIN,ACK has zero length of data
			sessionBuf[freeId].ackNum++;

			/* change state to CLOSE_WAIT when ACK is sent after recieving FIN */
			sessionBuf[freeId].state = CLOSE_WAIT;
			sessionBuf[(MAX_SESSIONS - 1) - freeId].state = CLOSE_WAIT;

			/*
			 * Time wait
			 *
			 *
			 */

			char * fin_ack = send_fin_ack(buffer,
					sessionBuf[(MAX_SESSIONS - 1) - freeId].seqNum,
					sessionBuf[(MAX_SESSIONS - 1) - freeId].ackNum);
			pfring_send(ring_eth, fin_ack,
					sizeof(struct ethhdr) + sizeof(struct iphdr)
							+ sizeof(struct tcphdr), 0);
			/*print_tcp_packet(fin_ack,
			 sizeof(struct ethhdr) + sizeof(struct iphdr)
			 + sizeof(struct tcphdr), logfile);*/
			free(fin_ack);

			sessionBuf[freeId].seqNum++;
			sessionBuf[freeId].ackNum++;

			/*change state toLAST_ACK */
			sessionBuf[freeId].state = LAST_ACK;
			sessionBuf[(MAX_SESSIONS - 1) - freeId].state = LAST_ACK;

		}

		if (sessionBuf[freeId].state == FIN_WAIT_1) {

			printf("within_fin_wait");
			sessionBuf[(MAX_SESSIONS - 1) - freeId].ackNum++; //assume FIN,ACK has zero length of data
			sessionBuf[(MAX_SESSIONS - 1) - freeId].seqNum++; // assuming we are not sending data

			//char * ack_for_fin = send_ack(buffer,sessionBuf[(MAX_SESSIONS-1)-freeId].seqNum,sessionBuf[(MAX_SESSIONS-1)-freeId].ackNum);
			//pfring_send(ring_eth,ack2,sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(struct tcphdr),0);
			//print_tcp_packet(ack2, sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(struct tcphdr), sentPacketsFile);

			//sessionBuf[freeId].seqNum++; // assume FIN,ACK has zero length of data
			//sessionBuf[freeId].ackNum++;

			/* change state to CLOSING when FIN is received after sending a  FIN ourselves */
			//sessionBuf[freeId].state = CLOSING;
			//sessionBuf[(MAX_SESSIONS-1)-freeId].state = CLOSING;
			/* receive ACK for this FIN and close connection
			 not implemented here ......................*/
		}

	} else if (isRST(buffer, size)) { /*#####################################RESET##########################################*/
		printf("RESET");
		/* send a Fin */
		//drop session
	} else { /*#####################################ACK##########################################*/
		// check for checksum
		u_int16_t len = getTCPPayloadSize(buffer, size); //iph-> tot_len-iph->ihl*4-tcph->doff*4;

		if (ntohl(tcph->seq) == sessionBuf[sessionId].seqNum) {
			// we are sure now that the packet is not an out of order packet

			//if(ntohl(tcph->ack_seq) == sessionBuf[sessionId].ackNum ){
			//we are now sure that the ack wasn't a selective ack

			if ((len + ntohl(tcph->seq) + ntohl(tcph->ack_seq))
					== sessionBuf[sessionId].HS_uNum) {
				//do nothing .. this is an ACK for the synack we sent
				sessionBuf[sessionId].state = ESTABLISHED;
				sessionBuf[(MAX_SESSIONS - 1) - sessionId].state = ESTABLISHED;
			} else {
				//sessionBuf[sessionId].ackNum+;
				sessionBuf[(MAX_SESSIONS - 1) - sessionId].ackNum += len;
				//sessionBuf[(MAX_SESSIONS-1)-sessionId].seqNum++;  // assuming we are not sending data
				sessionBuf[(MAX_SESSIONS - 1) - sessionId].seqNum = ntohl(
						tcph->ack_seq); // mind the data

				// can be a restransmission request

				char * ack_first = send_ack(buffer,
						sessionBuf[(MAX_SESSIONS - 1) - sessionId].seqNum,
						sessionBuf[(MAX_SESSIONS - 1) - sessionId].ackNum);
				pfring_send(ring_eth, ack_first,
						sizeof(struct ethhdr) + sizeof(struct iphdr)
								+ sizeof(struct tcphdr), 0);
				print_tcp_packet(ack_first,
						sizeof(struct ethhdr) + sizeof(struct iphdr)
								+ sizeof(struct tcphdr), f);
				free(ack_first);

				/*if(check_csum(buffer)){
				 fprintf(logfile,"\n packet checksum correct \n");
				 }
				 else{
				 fprintf(logfile,"\n packet checksum incorrect \n");
				 }*/

				sessionBuf[sessionId].seqNum += len;
				sessionBuf[sessionId].ackNum = ntohl(tcph->ack_seq) + 1;

			}
			/*else{
			 //SACK enabled
			 // ....still need work here
			 }*/
		} else {

			// poll for retransmission now
			if (ntohl(tcph->seq) > sessionBuf[sessionId].seqNum) {
				char *retrans = send_ack(buffer,
						sessionBuf[(MAX_SESSIONS - 1) - sessionId].seqNum,
						sessionBuf[(MAX_SESSIONS - 1) - sessionId].ackNum);
				pfring_send(ring_eth, retrans,
						sizeof(struct ethhdr) + sizeof(struct iphdr)
								+ sizeof(struct tcphdr), 0);
				print_tcp_packet(retrans,
						sizeof(struct ethhdr) + sizeof(struct iphdr)
								+ sizeof(struct tcphdr), f);
				free(retrans);
			}

		}

		//poll for retrnsmit error checksum
		/*else{
		 char *retransc = send_ack(buffer,sessionBuf[(MAX_SESSIONS-1)-sessionId].seqNum,sessionBuf[(MAX_SESSIONS-1)-sessionId].ackNum);
		 pfring_send(ring_eth,retransc,sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(struct tcphdr),0);
		 print_tcp_packet(retransc, sizeof(struct ethhdr)+sizeof(struct iphdr)+sizeof(struct tcphdr), f);
		 }*/

	}

	return 1;

}

int getId(unsigned char *buffer, int size) {
	int iphdrlen;
	struct tcphdr *tcph;
	struct iphdr *iph = (struct iphdr *) (buffer + sizeof(struct ethhdr));
	iphdrlen = iph->ihl * 4; //set ip header length
	tcph = (struct tcphdr *) (buffer + iphdrlen + sizeof(struct ethhdr)); //set tcp header

	int sessionId = 0;

	for (; sessionId < MAX_SESSIONS; ++sessionId) {
		if (sessionBuf[sessionId].sourceIP != iph->saddr) {
			continue;
		} else if (sessionBuf[sessionId].destIP != iph->daddr) {
			continue;
		} else if (sessionBuf[sessionId].sourcePort != tcph->source) {
			continue;
		} else if (sessionBuf[sessionId].destPort != tcph->dest) {
			continue;
		} else if (finList[sessionId] == 1) {
			continue;
		} else {
			return sessionId;
		}
	}

	return -1;
}

int getMinimumIdFromDeletedSessions() {
	if (root == NULL )
		return -1;
	else
		return minValueNode(root)->id;
}

unsigned char* getTCPPayload(unsigned char* buffer, int size) {

	struct iphdr *iph = (struct iphdr*) (buffer + sizeof(struct ethhdr));
	int iphdrlen = iph->ihl * 4; //set ip header length
	struct tcphdr *tcph = (struct tcphdr*) (buffer + sizeof(struct ethhdr)
			+ iphdrlen); //set tcp header
	int tcphdrlen = tcph->doff * 4; //set TCP header length
	unsigned char* tcpPayload = (unsigned char*) (buffer + sizeof(struct ethhdr)
			+ iphdrlen + tcphdrlen);

	return tcpPayload;
}

int getTCPPayloadSize(unsigned char* buffer, int size) {
	struct iphdr *iph = (struct iphdr*) (buffer + sizeof(struct ethhdr));
	int totLength = ntohs(iph->tot_len);
	int iphdrlen = iph->ihl * 4; //set ip header length
	struct tcphdr *tcph = (struct tcphdr*) (buffer + sizeof(struct ethhdr)
			+ iphdrlen); //set tcp header
	int tcphdrlen = tcph->doff * 4; //set TCP header length
	int totHeaderLen = iphdrlen + tcphdrlen;
	int applicationDataLength = totLength - totHeaderLen;

	return applicationDataLength;
}

void stringCopy(unsigned char* dest, unsigned char* src, int size) {
	int i = 0;
	for (; i < size; ++i) {
		*(dest + i) = src[i];
	}

}

char* concatCharArray(char* a, char*b, int size_a, int size_b) {

	char* result = (char*) malloc(size_a + size_b);

	int i;
	for (i = 0; i < size_a; ++i) {
		if (*(a + i) == NULL ) {
			result[i] = '0';
		} else {
			result[i] = *(a + i);
		}

	}

	for (i = 0; i < size_b; ++i) {
		if (*(b + i) == NULL ) {
			result[i + size_a] = '0';
		} else {
			result[i + size_a] = *(b + i);
		}
	}

	return result;
}

/**
 * this method chunks the given data stream
 */
int chunkData(unsigned char *buffer, int n) {
	unsigned char *copy;
	unsigned char *max = buffer, *end = buffer + n - 8;
	int i = 0;
	for (copy = buffer + 1; copy <= end; copy++) {
		int comp_res = comp(copy, max);
		if (comp_res < 0) {
			max = copy;
			continue;
		}
		if (copy == max + windowSize || copy == buffer + MAX_CHUNK_SIZE) { //chunk max size
			return copy - buffer;
		}
		i++;
	}
	return n;
}

/**
 * This method compares the given pointers
 */
int comp(unsigned char *i, unsigned char *max) {
	uint64_t a = __builtin_bswap64(*((uint64_t *) i));
	uint64_t b = __builtin_bswap64(*((uint64_t *) max));
	if (a > b) {
		return 1;
	}
	return -1;
}

int power(int n) {
	int res = 1;
	if (n == 0) {
		return 1;
	} else {
		int k;
		for (k = 0; k < n; k++) {
			res *= 2;
		}
	}
	return res;
}

//calculate xxhash
unsigned long long calcul_hash(const void* buffer, size_t length) {
	unsigned long long const seed = 0; /* or any other value */
	unsigned long long const hash = XXH64(buffer, length, seed);
	return hash;
}

int retNum(int x, int count) {
	int init = 0;
	u_int32_t result = 0;
	switch (count) {
	case 0:
		init = 24;
		break;
	case 1:
		init = 16;
		break;
	case 2:
		init = 8;
		break;
	case 3:
		init = 0;
		break;
	}
	while (x != 0) {
		result += (x % 2) * pow2(init++);
		x = x / 2;
	}
	return result;
}

int pow2(int num) {
	u_int32_t multi = 1;
	if (num == 0)
		return 1;
	int i;
	for (i = 0; i < num; i++) {
		multi *= 2;
	}
	return multi;
}

