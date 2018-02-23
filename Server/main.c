/*
 * main.c\
 *
 *  Created on: Jan 4, 2018
 *      Author: root
 */


/*with deduplication*/
#include <writeInLogger.h>

#ifndef headerIncluded
#include <headers.h>
#endif

#ifndef structIncluded
#include <types.h>
#endif

#ifndef SPOOF_H_
#include "spoof.h"
#endif

#ifndef constantsIncluded
#include <constants.h>
#endif

#ifndef LandataQueueIncluded
#include <LanDataQueue.h>
#endif

#include <DynamicArray.h>
#include <SHA1.h>
#include <xxhash.h>

/*Thread declarations*/
void* readFromWan(void *vargp);
void* process(void *vargp);
void* sendToGUI(void *vargp);

/*function declarations*/
char* concatCharArray(char* a, char*b, int size_a, int size_b);
void stringCopy(unsigned char* dest, unsigned char* src, int size);
int isSyn(unsigned char *buffer, int size);
int isFyn(unsigned char *buffer, int size);
int isRst(unsigned char *buffer, int size);
int isARP(unsigned char* buffer, int size);
int isAck(unsigned char* buffer, int size);
int isTcp(unsigned char* buffer, int size);
int isSynAck(unsigned char* buffer, int size);
int isFinAck(unsigned char* buffer, int size);
int getSeqNumber(unsigned char* buffer);
int getAckNumber(unsigned char* buffer);
int getRwnd(unsigned char* buffer);
void strCat(char* startPtr, char* sub, int size);
char* concatCharArray(char* a, char*b, int size_a, int size_b);
int chunkData(unsigned char *client_message, int n);
int comp(unsigned char *i, unsigned char *max);
int retNum(int num, int count);
int pow2(int num);
unsigned long long calcul_hash(const void* buffer, size_t length);
unsigned char* getTCPPayload(unsigned char* buffer, int size);
int getTCPPayloadSize(unsigned char* buffer, int size);
int getId(unsigned char *buffer, int size);

/*variable declarations*/
WAN_QUEUE* wanQueue;
int processInvoke = 0;
int NoOfElements = 0;
int totCount  = 0;

/*create session buffer*/
struct session sessionBuf[MAX_SESSIONS];

/*sessionId existance buffer*/
int sessionExists[MAX_SESSIONS] = { 0 };

char *device_eth = "eth0";
char *device_inet = "stack:eth1";
int dataCon = 0;

/*ethernet Thread parameters*/
#define MSS 1520
#define ITR 10000
#define MAX_PKT_LEN 1536
u_int32_t flags = 0;

bool g = true;
int accu;

/*assign mac addresses*/
char target_mac[6] = { 0xc4, 0x34, 0x6b, 0x56, 0xcd, 0xb4 };
char source_mac[6] = { 0x00, 0xe0, 0x4c, 0x68, 0x1e, 0xbc };

pthread_mutex_t wan_mutex;
pthread_mutex_t session_queue_mutex;


/*send to gui parameters*/
int guisock;
char numberSent[15];
char* hashName="HashTable_140";

int main(int argc, char *argv[]) {

	/*send to gui socket*/
	guisock=getWanSocket("10.8.145.228",9876);

	//int retcon = htonl(RET_CONST);

	// Write the number to the opened socket
	//write(guisock, &retcon, sizeof(retcon));



	/*WAN Queue Initialization*/
	wanQueue = (WAN_QUEUE*) malloc(sizeof(WAN_QUEUE));
	Init_wan_data(wanQueue, WAN_QUEUE_CAPACITY);

	/*set ids for threads*/
	pthread_t readFromWan_id, process_id, sendToLan_id;

	/*set attributes for threads*/
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	pthread_create(&readFromWan_id, &attr, &readFromWan, NULL);
	pthread_create(&process_id, &attr, &process, NULL);
	//pthread_create (&sendToLan_id, &attr,&sendToGUI, NULL);

	/*wait until all running thread exits*/
	pthread_join(readFromWan_id, NULL);
	pthread_join(process_id, NULL);
	//pthread_join(sendToLan_id, NULL);

	return 0;
}

/*#####################################Thread definitions###############################################*/

void* readFromWan(void *vargp) {
	printf(
			"\n#################################Read From WAN Thread#################################\n");

	FILE* receivedFromWanLog = fopen("receivedFromWanLog.txt", "a");

	int socket_desc, client_sock, c, read_size;
	struct sockaddr_in server, client;
	//unsigned char client_message[1024]={'\0'};

	//Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1) {
		printf("Could not create socket");
	}
	puts("Socket created");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(10000);

	//Bind
	if (bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");

	//Listen
	listen(socket_desc, 3);

	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);

	//accept connection from an incoming client
	client_sock = accept(socket_desc, (struct sockaddr *) &client,
			(socklen_t*) &c);
	if (client_sock < 0) {
		perror("accept failed");
		return 1;
	}
	puts("Connection accepted");

	//Receive a message from client
	WAN_RECORD* tmp;
	int totReadSize=0;
	int converted_number;
	while (1) {
		unsigned char client_message[1024]={'\0'};
		read_size = recv(client_sock, client_message, 1024, 0);
		if (read_size > 0) {
			totReadSize+=read_size;
			converted_number = htonl(totReadSize);

			// Write the number to the opened socket
			write(guisock, &converted_number, sizeof(converted_number));
			tmp = (WAN_RECORD*) malloc(sizeof(WAN_RECORD));
			tmp->data = (char*) malloc(1024 * sizeof(char));
			memcpy(tmp->data, client_message, 1024);
			*(tmp->data + read_size) = '\0';
			tmp->size = read_size;

			pthread_mutex_lock(&wan_mutex);
			enqueue_wan_data(wanQueue, tmp);
			pthread_mutex_unlock(&wan_mutex);
			logRawData(tmp->data, tmp->size, receivedFromWanLog);
		}
	}

	pthread_exit(0);
}

void* process(void *vargp) {

	printf("\n#################################Process Thread#################################\n");

	FILE* processData = fopen("processData.txt", "a");
	FILE* processData1 = fopen("processData1.txt", "a");
	FILE* log = fopen("log.txt", "a");
	FILE* log1 = fopen("log1.txt", "a");

	int sessionId = NULL;
	u_int32_t destIP;
	u_int16_t destPort;
	char* destIpStr;


	DATA_RECORD* tempDataRecord;
	WAN_RECORD* tempWanRecord;

	/*..........extracted from received data header...............*/
	struct sockaddr_in dest;
	/*..........extracted from received data header...............*/
	sessionId=0;//session Id
	destIpStr="192.168.2.2";
	dest.sin_addr.s_addr = destIpStr;
	dest.sin_family = AF_INET;
	dest.sin_port = htons( 1234 );
	destIP=dest.sin_addr.s_addr;//destination IP
	destPort=dest.sin_port;//destination port

	int charCount = 0;
	u_int16_t dedupLength = 0;
	u_int32_t dataLength = 0;
	bool lengthCalculated = false;
	bool dedupCalculated = false;
	Array messagetoProcess;
	initArray(&messagetoProcess, STORAGE_CONST);
	bool endReached = false;
	bool initFound = false;
	int initPoint = 0;
	int fuck = 0;
	int fuck1 = 0;
	int i = 0;
	FILE *logfile = fopen("log.txt", "a");
	bool secDolFound = false;
	bool dolFound = false;
	bool caome = true;

	time_t t;
	srand((unsigned) time(&t));
	int n = rand() % 1000000;
	char str[10] = { '\0' };
	sprintf(str, "%d", n);

	//----LevelDB configuration----//
	leveldb_t *db;
	leveldb_options_t *options;
	leveldb_readoptions_t *roptions;
	leveldb_writeoptions_t *woptions;
	char *err = NULL;
	size_t read_len;
	options = leveldb_options_create();
	leveldb_options_set_create_if_missing(options, 1);
	db = leveldb_open(options, str, &err);
	if (err != NULL) {
		printf("Database initiation failed.\n");
	} else {
		printf("Database Initialized.\n");
		leveldb_free(err);
		err = NULL;
	}
	//Initilize writing
	woptions = leveldb_writeoptions_create();
	//Initializing reading
	roptions = leveldb_readoptions_create();
	//----LevelDB configuration End----////

	//Test
	bool before = false;
	int retDataCount = 0;
	int bClock = clock();
	while (1) {

		if(wanQueue->size==0){
				continue;
		}
		if(sessionExists[sessionId]==0){
			/*add new session to session Buffer*/
			sessionBuf[sessionId].destIP = destIP;
			sessionBuf[sessionId].destPort = ntohs(destPort);
			sessionBuf[sessionId].sessionId = sessionId;
			sessionBuf[sessionId].sock=getSocket(destIpStr,sessionBuf[sessionId].destPort,&sessionBuf[sessionId]);

			/*create a data queue for created session*/
			sessionBuf[sessionId].queue = (DATA_QUEUE *) malloc(sizeof(DATA_QUEUE));
			Init_data(sessionBuf[sessionId].queue, MAX_RECORDS_PER_SESSION);
			//sessionBuf[sessionId].dqueue=thrqueue_new(MAX_RECORDS_PER_SESSION);

			/*add opposite session to session Buffer*/
			sessionBuf[(MAX_SESSIONS-1)-sessionId].destIP = sessionBuf[sessionId].sourceIP;
			sessionBuf[(MAX_SESSIONS-1)-sessionId].sourceIP = destIP;
			sessionBuf[(MAX_SESSIONS-1)-sessionId].sourcePort = ntohs(destPort);
			sessionBuf[(MAX_SESSIONS-1)-sessionId].destPort = sessionBuf[sessionId].sourcePort;
			sessionBuf[(MAX_SESSIONS-1)-sessionId].sessionId = (MAX_SESSIONS-1)-sessionId;

			/*create a data queue for created opposite session*/
			sessionBuf[(MAX_SESSIONS-1)-sessionId].queue = (DATA_QUEUE *) malloc(sizeof(DATA_QUEUE));
			Init_data(sessionBuf[(MAX_SESSIONS-1)-sessionId].queue, MAX_RECORDS_PER_SESSION);
			//sessionBuf[(MAX_SESSIONS-1)-sessionId].dqueue=thrqueue_new(MAX_RECORDS_PER_SESSION);


			/*notice that session exists in the buffer*/
			sessionExists[sessionId]=1;
			sessionExists[(MAX_SESSIONS-1)-sessionId]=1;
		}

		if (wanQueue->size > 0) {
			tempWanRecord = wanQueue->dataArray[wanQueue->front];
			pthread_mutex_lock(&wan_mutex);
			dequeue_wan_data(wanQueue);
			pthread_mutex_unlock(&wan_mutex);
			int read_size = tempWanRecord->size;
			unsigned char *client_message = tempWanRecord->data;
			logRawData(client_message, read_size, log1);
			if (read_size > 0) {
				for (i = 0; i < read_size; i++) {
					if (!initFound) {
						if (dolFound && !secDolFound && client_message[0] == '$') {
							secDolFound = true;
							initFound = true;
							i++;
						} else {
							dolFound = false;
						}
						if (!secDolFound) {
							while (1) {
								if (i == read_size - 1) {
									break;
								} else if (client_message[i] == '$'
										&& client_message[i + 1] == '$') {
									initFound = true;
									dolFound = true;
									secDolFound = true;
									i = i + 2;
									initPoint = i;
									break;
								}
								i++;
							}
						}

					}
					if (initFound && i < read_size) {
						insertArray(&messagetoProcess, client_message[i]);
						charCount++;
						if (charCount == 4 && !lengthCalculated) {
							int k;
							for (k = 0; k < 4; k++) {
								int a = (int) messagetoProcess.array[k];
								dataLength += retNum(((a < 0) ? (256 + a) : a),k);
							}
							if (dataLength > 102406 || dataLength == 0) {
								charCount = 0;
								dataLength = 0;
								dedupLength = 0;
								initFound = false;
								break;
							}
							lengthCalculated = true;
						}
						if (charCount == 6 && !dedupCalculated) {
							int k;
							for (k = 0; k < 2; k++) {
								int a = (int) messagetoProcess.array[k + 4];
								dedupLength += retNum(((a < 0) ? (256 + a) : a),k + 2);
							}
							dedupCalculated = true;
						}
						if (charCount == dataLength) {
							insertArray(&messagetoProcess, '\0');
							i++;
							//printf("%d\n", fuck++);
							endReached = true;
							initFound = false;
							break;
						}
					}
				}
			}
			if (endReached) {
				if (dedupLength > 0) {
					int positions[dedupLength];
					int deCount = 0;
					int collec = 0;
					for (deCount = 0; deCount < dedupLength; deCount++) {
						int inner = 0;
						int reNum = 0;
						for (inner = 0; inner < 4; inner++) {
							int instChar = *(messagetoProcess.array + 6+ collec++);
							int insertNum =(instChar < 0) ?(256 + instChar) : instChar;
							reNum += retNum(insertNum, inner);
						}
						positions[deCount] = reNum;
						reNum = 0;
					}
					unsigned char *modifiedArray = messagetoProcess.array+ dedupLength * 4 + 6;
					int cake = 0;
					int dupTrace = 0;
					Array cArray;
					unsigned char *dedupArray;
					int trace = 0;
					initArray(&cArray, STORAGE_CONST);
					int x;
					for (x = 0; x < dedupLength; x++) {
						int c1 = 0;
						int v;
						int newBufLength = positions[x] - trace;
						if (newBufLength > 0) {
							dedupArray = (unsigned char *) malloc(
									positions[x] - trace);
						}
						for (v = trace; v < positions[x]; v++) {
							insertArray(&cArray, *(modifiedArray + dupTrace));
							*(dedupArray + c1) = *(modifiedArray + dupTrace);
							c1++;
							dupTrace++;
						}
						if (newBufLength > 0) {
							int dataBoundaries = 0;
							unsigned char *dupArray = dedupArray;
							while (1) {
								int length = strlen(dupArray);
								int count = 0;
								if (length < (windowSize + 8)) {
									char xxHashBuf[HASH_LENGTH];
									sprintf(xxHashBuf, "%llu",
											calcul_hash(dupArray, length));
									int xxLen = strlen(xxHashBuf);
									while (xxLen != 20) {
										xxHashBuf[xxLen] = '*';
										xxLen++;
									}
									//printf("In  -- %s\n",xxHashBuf);
									char *result = leveldb_get(db, roptions,
											xxHashBuf, HASH_LENGTH, &read_len,
											&err);
									if (err != NULL) {
										printf("Read Hash Failed Phase 2.\n");
									} else {
										leveldb_free(err);
										err = NULL;
									}
									if (result == NULL) {
										leveldb_put(db, woptions, xxHashBuf,
												HASH_LENGTH, dupArray, length,
												&err);
										if (err != NULL) {
											printf(
													"Write Hash Failed Phase 2.\n");
										} else {
											leveldb_free(err);
											err = NULL;
										}
									}
									break;
								}

								int boundary = chunkData(dupArray, length);
								char xxHashBuf[HASH_LENGTH];
								sprintf(xxHashBuf, "%llu",
										calcul_hash(dupArray, boundary + 1));
								int xxLen = strlen(xxHashBuf);
								while (xxLen != 20) {
									xxHashBuf[xxLen] = '*';
									xxLen++;
								}
								//printf("In  -- %s\n",xxHashBuf);
								//check whether the hash exist
								char *result = leveldb_get(db, roptions,xxHashBuf, HASH_LENGTH, &read_len,&err);
								if (result == NULL) {
									leveldb_put(db, woptions, xxHashBuf,
											HASH_LENGTH, dupArray, boundary + 1,
											&err);
									if (err != NULL) {
										printf("Write Hash Fail\n");
									} else {
										leveldb_free(err);
									}
								}
								//break condition
								if (boundary == length) {
									break;
								}
								//increment string pointer
								dupArray = dupArray + boundary + 1;
							}
							free(dedupArray);
						}
						char key[HASH_LENGTH];
						int s;
						for (s = 0; s < HASH_LENGTH; s++) {
							key[s] = *(modifiedArray + dupTrace);
							dupTrace++;
						}
						if (strlen(key) > HASH_LENGTH) {
							key[HASH_LENGTH] = '\0';
						}
						//printf("KEY %s\n", key);
						char *result = leveldb_get(db, roptions, key,HASH_LENGTH, &read_len, &err);
						if (err != NULL) {
							printf("Read Hash Failed Phase 1.\n");
						} else {
							leveldb_free(err);
						}
						if (result != NULL) {
							//printf("KEY: %s\n", key);
							int retLen = strlen(result);
							int n;
							for (n = 0; n < retLen; n++) {
								insertArray(&cArray, *(result + n));
							}
						}
						trace = dupTrace;
					}
					int totLen = strlen(modifiedArray);
					if (trace < totLen) {
						int y;
						for (y = trace; y < totLen; y++) {
							insertArray(&cArray, *(modifiedArray + y));
						}
					}
					insertArray(&cArray, '\0');
					sendTo(cArray.array, cArray.used,sessionBuf[sessionId].sock);
					totCount += cArray.used;
					freeArray(&cArray);
				} else {
					int dataBoundaries = 0;
					unsigned char *dupArray = messagetoProcess.array + 6;
					logRawData(messagetoProcess.array + 6,messagetoProcess.used, processData);
					sendTo(messagetoProcess.array+6, messagetoProcess.used,sessionBuf[sessionId].sock);
					totCount += messagetoProcess.used - 6;
					while (1) {
						int length = strlen(dupArray);
						int count = 0;
						if (length < (windowSize + 8)) {
							char xxHashBuf[HASH_LENGTH];
							sprintf(xxHashBuf, "%llu",calcul_hash(dupArray, length));
							int xxLen = strlen(xxHashBuf);
							while (xxLen != 20) {
								xxHashBuf[xxLen] = '*';
								xxLen++;
							}
							//check whether the hash exist
							char *result = leveldb_get(db, roptions, xxHashBuf,
							HASH_LENGTH, &read_len, &err);
							if (err != NULL) {
								printf("Read Hash Failed Phase 2.\n");
							} else {
								leveldb_free(err);
								err = NULL;
							}
							if (result == NULL) {
								leveldb_put(db, woptions, xxHashBuf,HASH_LENGTH, dupArray, length, &err);
								if (err != NULL) {
									printf("Write Hash Failed Phase 2.\n");
								} else {
									leveldb_free(err);
									err = NULL;
								}
							}
							break;
						}

						//if length is more than window size
						int boundary = chunkData(dupArray, length);

						char xxHashBuf[HASH_LENGTH];
						sprintf(xxHashBuf, "%llu",calcul_hash(dupArray, boundary + 1));
						int xxLen = strlen(xxHashBuf);
						while (xxLen != 20) {
							xxHashBuf[xxLen] = '*';
							xxLen++;
						}

						//check whether the hash exist
						char *result = leveldb_get(db, roptions, xxHashBuf,
						HASH_LENGTH, &read_len, &err);
						if (result == NULL) {
							leveldb_put(db, woptions, xxHashBuf, HASH_LENGTH,dupArray, boundary + 1, &err);
							if (err != NULL) {
								printf("Write Hash Fail\n");
							} else {
								leveldb_free(err);
							}
							dataBoundaries += boundary + 1;
						} else {
							dataBoundaries += HASH_LENGTH;
						}

						//break condition
						if (boundary == length) {
							break;
						}
						//increment string pointer
						dupArray = dupArray + boundary + 1;
					}
				}
				charCount = 0;
				dataLength = 0;
				dedupLength = 0;
				lengthCalculated = false;
				dedupCalculated = false;
				initPoint = 0;
				dolFound = false;
				secDolFound = false;
				if (i < read_size) {
					while (1) {
						if (i == read_size - 1) {
							break;
						} else if (client_message[i] == '$'
								&& client_message[i + 1] == '$') {
							initFound = true;
							i = i + 2;
							break;
						}
						i++;
					}
					if (!initFound) {
						if (client_message[read_size - 1] == '$') {
							dolFound = true;
						}
					}
					if (initFound) {
						if ((read_size - i) > 3) {
							int k;
							for (k = 0; k < 4; k++) {
								int a = (int) client_message[k + i];
								dataLength += retNum(((a < 0) ? (256 + a) : a),
										k);
							}
							lengthCalculated = true;
						}
						if ((read_size - i) > 5) {
							int k;
							for (k = 0; k < 2; k++) {
								char c = client_message[k + i + 4];
								int a = (int) client_message[k + i + 4];
								dedupLength += retNum(((a < 0) ? (256 + a) : a),k + 2);
							}
							dedupCalculated = true;
						}
						freeArray(&messagetoProcess);
						initArray(&messagetoProcess, STORAGE_CONST);
						int l;
						for (l = i; l < read_size; l++) {
							insertArray(&messagetoProcess, client_message[l]);
							charCount++;
						}
					} else {
						freeArray(&messagetoProcess);
						initArray(&messagetoProcess, STORAGE_CONST);
					}

				} else {
					freeArray(&messagetoProcess);
					initArray(&messagetoProcess, STORAGE_CONST);
				}
				endReached = false;
			}
			free(tempWanRecord);
		}
	}
	pthread_exit(0);
}

/*#####################################function definitions###############################################*/

int isSyn(unsigned char *buffer, int size) {
	struct iphdr *iph = (struct iphdr *) (buffer + sizeof(struct ethhdr));
	int iphdrlen = iph->ihl * 4; //set ip header length
	struct tcphdr *tcph = (struct tcphdr *) (buffer + iphdrlen
			+ sizeof(struct ethhdr)); //set tcp header

	if (tcph->syn == 1)
		return 1;
	return 0;

}

int isRst(unsigned char *buffer, int size) {
	struct iphdr *iph = (struct iphdr *) (buffer + sizeof(struct ethhdr));
	int iphdrlen = iph->ihl * 4; //set ip header length
	struct tcphdr *tcph = (struct tcphdr *) (buffer + iphdrlen
			+ sizeof(struct ethhdr)); //set tcp header

	if (tcph->rst == 1)
		return 1;
	return 0;

}

int isFyn(unsigned char *buffer, int size) {
	struct iphdr *iph = (struct iphdr *) (buffer + sizeof(struct ethhdr));
	int iphdrlen = iph->ihl * 4; //set ip header length
	struct tcphdr *tcph = (struct tcphdr *) (buffer + iphdrlen
			+ sizeof(struct ethhdr)); //set tcp header

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

int isSynAck(unsigned char* buffer, int size) {
	struct iphdr *iph = (struct iphdr *) (buffer + sizeof(struct ethhdr));
	int iphdrlen = iph->ihl * 4; //set ip header length
	struct tcphdr *tcph = (struct tcphdr *) (buffer + iphdrlen
			+ sizeof(struct ethhdr)); //set tcp header

	if (tcph->ack == 1 && tcph->syn == 1 && tcph->rst == 0)
		return 1;
	return 0;
}

int isFinAck(unsigned char* buffer, int size) {
	struct iphdr *iph = (struct iphdr *) (buffer + sizeof(struct ethhdr));
	int iphdrlen = iph->ihl * 4; //set ip header length
	struct tcphdr *tcph = (struct tcphdr *) (buffer + iphdrlen
			+ sizeof(struct ethhdr)); //set tcp header

	if (tcph->ack == 1 && tcph->fin == 1 && tcph->rst == 0)
		return 1;
	return 0;
}

int isTcp(unsigned char* buffer, int size) {
	int iphdrlen;
	struct tcphdr *tcph;
	struct iphdr *iph = (struct iphdr*) (buffer + sizeof(struct ethhdr));
	iphdrlen = iph->ihl * 4; //set ip header length

//check for tcp checksum if incorrect discard packet

	if (iph->protocol == 6) //Check the Protocol and do accordingly.....
		return 1;

	return 0;
}

int getSeqNumber(unsigned char* buffer) {

	struct tcphdr *tcphs = (struct tcphdr *) (buffer + sizeof(struct iphdr)
			+ sizeof(struct ethhdr)); // ip header of spoofed packet
	return tcphs->seq;
}

int getAckNumber(unsigned char* buffer) {

	struct tcphdr *tcphs = (struct tcphdr *) (buffer + sizeof(struct iphdr)
			+ sizeof(struct ethhdr)); // ip header of spoofed packet
	return tcphs->ack_seq;
}

int getRwnd(unsigned char* buffer) {

	struct tcphdr *tcphs = (struct tcphdr *) (buffer + sizeof(struct iphdr)
			+ sizeof(struct ethhdr)); // ip header of spoofed packet
	return tcphs->window;
}

char* concatCharArray(char* a, char*b, int size_a, int size_b) {

	char* result = (char*) malloc(size_a + size_b);

	int i;
	for (i = 0; i < size_a; ++i) {
		if (*(a + i) == NULL) {
			result[i] = '0';
		} else {
			result[i] = *(a + i);
		}

	}

	for (i = 0; i < size_b; ++i) {
		if (*(b + i) == NULL) {
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
int chunkData(unsigned char *client_message, int n) {

	unsigned char *copy;
	unsigned char *max = client_message, *end = client_message + n - 8;
	int i = 0;
	for (copy = client_message + 1; copy <= end; copy++) {
		int comp_res = comp(copy, max);
		if (comp_res < 0) {
			max = copy;
			continue;
		}
		if (copy == max + windowSize || copy == client_message + MAX_CHUNK_SIZE) { //chunk max size
			return copy - client_message;
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

/**
 * This method retrieves the seralized numbers
 */
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

//calculate xxhash
unsigned long long calcul_hash(const void* buffer, size_t length) {
	unsigned long long const seed = 0; /* or any other value */
	unsigned long long const hash = XXH64(buffer, length, seed);
	return hash;
}

unsigned char* getTCPPayload(unsigned char* buffer, int size)
{

	struct iphdr *iph = (struct iphdr*)buffer;
	int iphdrlen = iph->ihl*4;//set ip header length
	struct tcphdr *tcph=(struct tcphdr*)(buffer + iphdrlen);//set tcp header
	int tcphdrlen = tcph->doff*4;//set TCP header length
	unsigned char* tcpPayload=(unsigned char*)(buffer + iphdrlen+tcphdrlen);

	return tcpPayload;
}

int getTCPPayloadSize(unsigned char* buffer, int size)
{
	struct iphdr *iph = (struct iphdr*)buffer;
	int totLength=ntohs(iph->tot_len);
	int iphdrlen = iph->ihl*4;//set ip header length
	struct tcphdr *tcph=(struct tcphdr*)(buffer + iphdrlen);//set tcp header
	int tcphdrlen = tcph->doff*4;//set TCP header length
	int totHeaderLen=iphdrlen+tcphdrlen;
	int applicationDataLength=totLength-totHeaderLen;

	return applicationDataLength;
}

int getId(unsigned char *buffer, int size) {
    int iphdrlen;
    struct tcphdr *tcph;
    struct iphdr *iph = (struct iphdr *)buffer;
    iphdrlen = iph->ihl * 4;//set ip header length
    tcph = (struct tcphdr *) (buffer + iphdrlen );//set tcp header

    int sessionId = MAX_SESSIONS-1;

    for (; sessionId>=0; --sessionId) {
        if (sessionBuf[sessionId].sourceIP != iph->saddr) {
            continue;
        } else if (sessionBuf[sessionId].destIP != iph->daddr) {
            continue;
        } else if (sessionBuf[sessionId].sourcePort != tcph->source) {
            continue;
        } else if (sessionBuf[sessionId].destPort != tcph->dest) {
            continue;
        }else{
            return sessionId;
        }
    }

    return -1;
}

void tostring(char str[], int num)

{

    int i, rem, len = 0, n;



    n = num;

    while (n != 0)

    {

        len++;

        n /= 10;

    }

    for (i = 0; i < len; i++)

    {

        rem = num % 10;

        num = num / 10;

        str[len - (i + 1)] = rem + '0';

    }

    str[len] = '\0';

}
