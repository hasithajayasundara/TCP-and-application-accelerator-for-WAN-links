#define constantsIncluded

#define MAX_SESSIONS 100
#define RET 20
#define MAX_RECORDS_PER_SESSION 3000000
#define WAN_QUEUE_CAPACITY 20000000
#define MSS 1460//define Maximum Segment Size

#define MAX_WAITING_PACKETS 1000

/*wanHandling constants*/
#define SERVER_PORT 9000

/**
 * de-duplication variables
 */
#define MAX_CHUNK_SIZE 30000
#define AVERAGE_CHUNK_SIZE 1024
#define E 2.718281828
#define STORAGE_CONST 1
#define FILE_CONST 102400
#define BUFFER_LENGTH 1024
#define HASH_LENGTH 20
#define RET_CONST 54


/**
 * AE window size
 */
static int windowSize = AVERAGE_CHUNK_SIZE / (E - 1);
