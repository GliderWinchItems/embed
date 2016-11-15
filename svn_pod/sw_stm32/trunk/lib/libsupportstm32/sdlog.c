/******************************************************************************/
/* File: sdlog.c -- Implelentation of sdcard data logger.
 */
/* for debugging */
#include "libmiscstm32/printf.h"
/*end debugging */
 
#include <string.h>
#include "sdlog.h"
#include "libsupportstm32/sdcard_csd.h"

/* Global variables
 */
int sdlog_err;							/* Global error variable */

/* Local function prototypes
 */
void sdlog_fatal(void);					/* Local fatal error handling */

/* Macros, manifest constants, typedefs and such.
 */
#define	ERR(n) (-10000+(n))				/* Our errors are biased by -10K */

#define	EOB				0				/* End of block */
#define	SIZE_MIN		1				/* Minimum size byte value */
#define	SIZE_MAX		250				/* Maximum size byte value */

#define	SDLOG_EXTRA_BLOCKS	2048		/* Blocks reserved at top of SD-card */

#define	ASSERT(boolean) if(!(boolean)) { sdlog_fatal(); }

#ifndef NULL
	#define	NULL (void *)0
#endif

typedef sdlog_seek_t pid_t;				/* sdlog_seek returns a packet id */

/* Higher level structures & unions.
 */
#define	SDLOG_DATASIZE	(SDCARD_BLOCKSIZE-sizeof(pid_t))

typedef union							/* Union of raw & structured blocks */
{
	char raw[SDCARD_BLOCKSIZE];			/* Buffer for "sdcard.[ch]" calls */
	struct								/* Used for local structured access */
	{
		char data[SDLOG_DATASIZE];
		pid_t pid;
	} str;
} block_t;

typedef struct							/* "Stream" structure */
{
	int blocknum;						/* SD card block number */
	char *ptcc;							/* Pointer to "current character" */
	block_t block;						/* An SDCARD block */
} stream_t;

/* Local variables.
 */
static int inited = (1==0);				/* Has this package been inited? */
static int card_size_in_blocks;			/* 0 <= blocknum < card_size_in_blocks */
static int first_read_flag;				/* Force read of last written */
static int eof_flag = (1==0);			/* Persistent EOF flag */

static stream_t read;
static stream_t write;

/* More constants.
 */
#define	R_DAT_BOTTOM	(read.block.str.data + 0)
#define	R_DAT_TOP		(read.block.str.data + SDLOG_DATASIZE)
#define	W_DAT_BOTTOM	(write.block.str.data + 0)
#define	W_DAT_TOP		(write.block.str.data + SDLOG_DATASIZE)

/******************************************************************************/
/* Client API functions.
 */
/******************************************************************************/
int sdlog_init(void)
{
	int low_block, high_block, mid_block;	/* Block numbers */
	pid_t id0;								/* ID for block 0 */
	char *p;
	
	first_read_flag = (1==1);				/* Always reset the next read */
	eof_flag = (1==0);						/* Always clear persistent eof */
	sdlog_err = 0;							/* Always clear sdlog_err */

	if(inited)								/* Already called returns OK */
		return 0;
	inited = (1==1);

	/* Some error checking
	 */
	ASSERT(sizeof(block_t) == SDCARD_BLOCKSIZE);

	/* Start with some internal initialization.
	 */
	sdlog_err = sdcard_init(NULL, write.block.raw); /* Borrow write.block.raw */
	if(sdlog_err < 0)
		return SDLOG_ERR;

	/* Set the card size in blocks (if not already set by the debugging API)
	 * and adjust for the extra blocks.  Note that the sdlog layer always
	 * assumes that blocks are a fixed length of 512 bytes, thus it doesn't
	 * handle 2GB cards (cards w/ 1024 byte blocks) correctly.
	 */
	if(card_size_in_blocks == 0)			/* Already non-0 means debug */
		card_size_in_blocks = sdcard_csd_memory_size(write.block.raw);
	card_size_in_blocks -= SDLOG_EXTRA_BLOCKS;
	ASSERT(card_size_in_blocks > 0);

	/* Setup to find the queue boundary.
	 */
	low_block = 0;
	high_block = card_size_in_blocks;
	
	sdlog_err = sdcard_read(0, write.block.raw);
	if(sdlog_err < 0)
		return SDLOG_ERR;

	id0 = write.block.str.pid;

	/* Go converge on the queue boundary.
	 */
	while(1==1)
	{
		int delta = high_block - low_block;

		if(delta == 1)
			break;

		ASSERT(delta > 1);
			
		mid_block = low_block + (delta / 2);

		sdlog_err = sdcard_read(mid_block, write.block.raw);
		if(sdlog_err < 0)
			return SDLOG_ERR;
			
		if(id0 >= write.block.str.pid)
			high_block = mid_block;				/* Bring down the high end */
		else
			low_block = mid_block;				/* Bring up the low end */
	}

	/* Setup current block to write and its pointer.  
	 */
	write.blocknum = 							/* Modulo for special case! */
		low_block % card_size_in_blocks;
		
	sdlog_err = sdcard_read(write.blocknum, write.block.raw);
	if(sdlog_err < 0)
		return SDLOG_ERR;

	p = W_DAT_TOP;
	
	while(p > W_DAT_BOTTOM)
		if(*--p != 0)
		{
			p += 1;
			break;
		}
	
	write.ptcc = p;

	return SDLOG_OK;
}

/******************************************************************************/
int sdlog_write(void *buf, int count)
{
	int remaining;
	
	ASSERT(inited);
	ASSERT(buf);
	ASSERT(count >= SIZE_MIN  &&  count <= SIZE_MAX);

	remaining = W_DAT_TOP - write.ptcc;
	
	if(remaining <= count)
	{
		if(remaining > 0)
			memset(write.ptcc, 0, remaining);
		
		sdlog_err = sdcard_write(write.blocknum, write.block.raw);
		if(sdlog_err != 0)
			return SDLOG_ERR;
		
		memset(W_DAT_BOTTOM, 0, SDLOG_DATASIZE);
		write.ptcc = W_DAT_BOTTOM;

		write.blocknum += 1;
		if(write.blocknum >= card_size_in_blocks)
			write.blocknum = 0;
			
		if(write.blocknum == read.blocknum)
			eof_flag = (1==1);
	}

	memcpy(write.ptcc, buf, count);			/* Copy out caller's packet */
	write.ptcc += count;					/* Skip packet */
	*write.ptcc++ = count;					/* Put out packet size */

	write.block.str.pid += 1;				/* Show next packet ID */

	return SDLOG_OK;
}

/******************************************************************************/
int sdlog_read_backwards(void *buf, int buf_size)
{
	int count;
	
	ASSERT(inited);
	ASSERT(buf);
	ASSERT(buf_size >= SIZE_MIN);

	/* If this is the first read, nail to read stream to the current values
	 * of the write stream.
	 */

	if(first_read_flag)
	{
		first_read_flag = (1==0);
		
		memcpy(&read, &write, sizeof(read));
		
		read.blocknum = write.blocknum;
		read.ptcc = R_DAT_BOTTOM + (write.ptcc - W_DAT_BOTTOM);
		
		eof_flag = (1==0);
	}
	
	if(eof_flag)
		return SDLOG_EOF;

	if(read.ptcc <= R_DAT_BOTTOM)
	{
		pid_t pid_save = read.block.str.pid;

		if(read.blocknum == 0)
			read.blocknum = card_size_in_blocks;
		read.blocknum -= 1;
		
		if(read.blocknum == write.blocknum)
		{
			eof_flag = (1==1);
			return SDLOG_EOF;
		}

		sdlog_err = sdcard_read(read.blocknum, read.block.raw);
		if(sdlog_err < 0)
			return SDLOG_ERR;

		if(pid_save != read.block.str.pid)
			return SDLOG_EOF;
		
		read.ptcc = R_DAT_TOP;
		
		while(read.ptcc > R_DAT_BOTTOM)
			if(*--read.ptcc != 0)
			{
				read.ptcc += 1;
				break;
			}
		
		if(read.ptcc == R_DAT_BOTTOM)
			return SDLOG_EOF;
	}

	read.block.str.pid -= 1;

	count = *--read.ptcc & 0xff;

	if(count < SIZE_MIN || count > SIZE_MAX)
		return ERR(-7);

	read.ptcc -= count;
	if(read.ptcc < R_DAT_BOTTOM)
		return ERR(-8);

	if(count > buf_size)
		count = buf_size;

	memcpy(buf, read.ptcc, count);

	return count;
}

/******************************************************************************/
/* sdlog's helper macros for mapping block numbers the run backwards and are 
 * relative to write.blocknum.
 */
static int map(int blocknum);
#define a2r(absolute_blocknum) map(absolute_blocknum)
#define r2a(relative_blocknum) map(relative_blocknum)
static pid_t oldest_pid(block_t *b);

#define TX do {void USART1_txint_send(void); USART1_txint_send(); } while(1==0)
void wf(int i) { printf("wf(%d)\r\n", i); TX; }
void wf2(int i, int j) { printf("wf2(%d, %d)\r\n", i, j); TX; }
void wf3(int i, int j, int k) { printf("wf3(%d, %d, %d)\r\n", i, j, k); TX; }
void wfs(char *p) { printf("wfs(\"%s\")\r\n", p); TX; }

int sdlog_seek(unsigned long long pid)
{
	int old_rblock, new_rblock, read_rblock;/* Realtive block numbers */
	int n;

	if(!inited)
		return ERR(-1);
		
	sdlog_flush();							/* Write cached write block */

	/* Very large pid (e.g., ~(0ULL)) resets to most recent written record
	 */
	if(pid >= write.block.str.pid)
	{
		first_read_flag = (1==1);			/* Always reset the next read */
		return SDLOG_OK;
	}

	/* Setup for binary search.  Start by reading oldest data block.
	 */
	new_rblock = 0;							/* Newest is relative 0 */
	old_rblock = card_size_in_blocks - 1;	/* Oldest is relative max-1 */
	read_rblock = 0;						/* Avoid bogus compiler warning */

	sdlog_err = sdcard_read(r2a(old_rblock), read.block.raw);
	if(sdlog_err < 0)						/* Can't read */
		return SDLOG_ERR;
	if(pid < oldest_pid(&read.block))
		return SDLOG_EOF;

	/* Do binary search.  The if-statement at the bottom of the loop requires
	 * some study.
	 */
	while(new_rblock < old_rblock)
	{
		/* Calculate the middle rblock.
		 */
		read_rblock = new_rblock + ((old_rblock - new_rblock) / 2);

		sdlog_err = sdcard_read(r2a(read_rblock), read.block.raw);
		if(sdlog_err < 0)
			return SDLOG_ERR;

		/* I apologize in advance for the following nested if statements, but
		 * they all do seem to be necessary.  If the special case where
		 * new_rblock == read_rblock isn't handled, this loop hangs forever.
		 */
		if(pid < oldest_pid(&read.block))		/* pid older than this block? */
		{
			if(new_rblock == read_rblock)		/* Special case? */
				new_rblock += 1;				/* Older special case */
			else
				new_rblock = read_rblock;		/* Older normal case */
		}
		else if(pid > read.block.str.pid)		/* pid newer than this block? */
			old_rblock = read_rblock;			/* Newer normal case */
		else
			break;								/* pid in this block */
	}

	read.blocknum = r2a(read_rblock);

	/* Find the last record in this block
	 */
	read.ptcc = R_DAT_TOP;
	while(R_DAT_BOTTOM < read.ptcc)
		if(*--read.ptcc != 0)
		{
			read.ptcc++;
			break;
		}

	/* Find the exact record we're looking for
	 */
	while(R_DAT_BOTTOM < read.ptcc  &&  pid != read.block.str.pid)
	{
		read.block.str.pid -= 1;
		n = *--read.ptcc & 0xff;
		read.ptcc -= n;
	}

	eof_flag = (1==0);						/* Successful seek resets eof */
	return SDLOG_OK;
}

/* This function maps absolute to relative block numbers as well as relative to
 * absolute block numbers.  Relative block numbers are relative to write.blocknum
 * and run backwards. The fact that this function is its own inverse (i.e., 
 * map(map(i)) == i) seems to be serendipity.
 */
static int map(int blocknum)
{
//	ASSERT(blocknum >= 0);
//	ASSERT(blocknum < card_size_in_blocks);

	blocknum = write.blocknum - blocknum;

	if(blocknum < 0)
		blocknum += card_size_in_blocks;

	return blocknum;
}

/* This function finds the oldest packet ID in a block.  The pid at the end of
 * the block is actually the pid assiciated with the first packet in the next
 * newer block.
 */
static pid_t oldest_pid(block_t *b)
{
	pid_t pid = b->str.pid;
	unsigned char *bot = (unsigned char *)b->str.data+0;
	unsigned char *top = (unsigned char *)b->str.data+SDLOG_DATASIZE;
	int n = 0;
	unsigned char uch;
	
	do { uch = *--top; } while(uch == 0 && bot < top);
	
	if(bot < top)
		for(top++; bot < top; )
		{
			n += 1;
			uch = *--top;
			top -= uch;
		}

	return pid - n;
}
/******************************************************************************/
unsigned long long sdlog_read_tell(void)
{
	if(!inited)
		return ERR(-1);
	
	return read.block.str.pid;
}

/******************************************************************************/
unsigned long long sdlog_write_tell(void)
{
	if(!inited)
		return ERR(-1);

	return write.block.str.pid;
}

/******************************************************************************/
int sdlog_flush(void)
{
	if(!inited)
		return ERR(-1);
		
	sdlog_err = sdcard_write(write.blocknum, write.block.raw);
	return (sdlog_err < 0) ? SDLOG_ERR : SDLOG_OK;
}

/******************************************************************************/
int sdlog_keepalive(void)
{
	return SDLOG_OK;
}

/******************************************************************************/
/* Fatal error handling
 */
volatile int sdlog_fatal_counter = 0;

void sdlog_fatal(void)
{
	do {sdlog_fatal_counter += 1; } while(1==1);
}

/******************************************************************************/
/* "Extra Blocks" implementation.
 */
int sdlog_read_extra_block(int blocknum, char buf[SDCARD_BLOCKSIZE])
{
	if(!inited)
		return ERR(-1);

	ASSERT(blocknum >= 0);
	ASSERT(blocknum < SDLOG_EXTRA_BLOCKS);
		
	sdlog_err = sdcard_read(card_size_in_blocks + blocknum, buf);
	return (sdlog_err < 0) ? SDLOG_ERR : SDLOG_OK;
}

int sdlog_write_extra_block(int blocknum, char buf[SDCARD_BLOCKSIZE])
{
	if(!inited)
		return ERR(-1);
		
	ASSERT(blocknum >= 0);
	ASSERT(blocknum < SDLOG_EXTRA_BLOCKS);
		
	sdlog_err = sdcard_write(card_size_in_blocks + blocknum, buf);
	return (sdlog_err < 0) ? SDLOG_ERR : SDLOG_OK;
}

/******************************************************************************/
/* Debugging only
 */
int sdlog_debug_get_card_size_in_blocks(void)
{
	return (unsigned long)card_size_in_blocks;
}

void sdlog_debug_set_card_size_in_blocks(int new_size)
{
	if(card_size_in_blocks == 0)
		card_size_in_blocks = new_size;
}

/******************************************************************************/
