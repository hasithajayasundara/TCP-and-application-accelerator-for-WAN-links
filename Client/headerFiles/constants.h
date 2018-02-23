#define constantsIncluded

#define MAX_SESSIONS 100
#define MAX_RECORDS_PER_SESSION 100
#define CHUNK_CAPACITY 15000
#define DEDUP_CAPACITY 1500000
#define NETWORK_CAPACITY 50000
#define KB_PER_CHUNK 100
#define MSS 1460//define Maximum Segment Size

#define MAX_WAITING_PACKETS 1000

/*wanHandling constants*/
#define SERVER_PORT 10000

/**
 * de-duplication variables
 */
#define MAX_CHUNK_SIZE 30000 //65536
#define AVERAGE_CHUNK_SIZE 1024
#define E 2.718281828
#define STORAGE_CONST 1
#define FILE_CONST 102400
#define SHA_LENGTH 20
#define HASH_LENGTH 20
#define ALT_HASH 7
#define PACKET_SIZE 1024


/**
 * AE window size
 */
static int windowSize = AVERAGE_CHUNK_SIZE / (E - 1);







