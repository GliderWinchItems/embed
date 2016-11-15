typedef union
{
	int8_t int8[SD_BLOCKSIZE/sizeof(int8_t)];
	int16_t int16[SD_BLOCKSIZE/sizeof(int16_t)];
	int32_t int32[SD_BLOCKSIZE/sizeof(int32_t)];
	struct
	{
		char data[SD_BLOCKSIZE-sizeof(int64_t)];
		int64_t pid;
	} blk;
} sdcard_block_t;

typedef struct
{
	/* Error message returned or NULL for no errors */
	char *err_msg;

	/* Internally updated.  Caller can peek. */	
	int inited;
	int64_t this_pid, last_pid;
	int no_packets;
} sdcard_isok_t;

void block_dump(sdcard_block_t *b, int32_t blocknum);
int block_isze(sdcard_block_t *b);
char *block_isok(sdcard_block_t *b, sdcard_isok_t *k);
int32_t block_cksum(sdcard_block_t *b);

