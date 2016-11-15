/******************************************************************************/
/* File: sdlog_test.c - Implementation for sdlog test appendage to sdcardtest4.
 *
 * This file contains the test code scraped out of sdcardtest4.c.
 */
#include <math.h>
#include <string.h>
#include "libmiscstm32/printf.h"
#include "sdcard.h"
#include "sdlog.h"

#include "sdlog_test.h"

/* Function prototypes
 */
void USART2_txint_send(void);

/* Local variables & constants
 */
#define	SDLOG_TEST_SIZE	20

static int inited = (1==0);
static int running = (1==0);
static int cmd_arg;

static void (*ptrwf)(void) = (void*)0;	/* Pointer to read/write test */
#define	NEXT_STATE(funct) do { void (funct)(void); ptrwf = funct; } while(1==0)

static unsigned long long random;

#define	REC_SIZE (0x50-1)
char rec[REC_SIZE];

#define	PR	USART2_txint_send();	/* Used after printf(...) */

/******************************************************************************/
/* Function implementations
 */
int sdlog_test_init(void)
{
	sdlog_debug_set_card_size_in_blocks(SDLOG_TEST_SIZE + SDLOG_EXTRA_BLOCKS);
	printf("\n\r"); PR;
	printf("sdlog_debug_get_card_size_in_blocks() returns %u\n\r", 
					sdlog_debug_get_card_size_in_blocks());
	PR;
	return 0;
}

/******************************************************************************/

int sdlog_test_cmd(int n)
{
	cmd_arg = n;
	running = (1==1);
	NEXT_STATE(sdlog_test_00);
	return 0;
}

/******************************************************************************/
int check(char *, int);
unsigned long long get_ull(char *p);
void put_ull(char *p, unsigned long long ull);

int sdlog_test_loop(void)
{
	random = random * 7919 + 0xdeadbeef;

	if(!running)
		return 0;

	if(!inited)
	{
		inited = (1==1);
		sdlog_init();
	}
	
	if(ptrwf)
		(*ptrwf)();

	return 0;
}

/******************************************************************************/
static sdlog_seek_t read_marker;
static int read_count;

void sdlog_test_00(void)		/* sdlog_write pass */
{
	int i, j;

	printf("## START of sdlog_test_00 (sdlog_write pass)\n\r"); PR;
	printf("cmd_arg is %d\r\n", cmd_arg);

	for(i=0; i<cmd_arg; i+=1)
	{
		put_ull(rec, sdlog_write_tell());
		for(j=8; j<REC_SIZE; j+=1)
			rec[j] = j;
		check("sdlog_write", sdlog_write(rec, REC_SIZE));
	}
	
	sdlog_flush();

	printf("sdlog_write_tell returns %d\n\r", (int)sdlog_write_tell());

	NEXT_STATE(sdlog_test_01);
	printf("\r\n");
}

void sdlog_test_01(void)		/* sdlog_read_backwards pass */
{
	int i, j, n;
	sdlog_seek_t here;
	unsigned long long ull;

	printf("## START of sdlog_test_01 (sdlog_read_backwards pass)\n\r"); PR;

	for(i=0; ; i+=1)
	{
		n = sdlog_read_backwards(rec, REC_SIZE);
		if(n == SDLOG_EOF)
			break;
		check("sdlog_read_backwards", n);

		if(n != REC_SIZE)
		{
			printf("# %d bytes read is not %d\r\n", n, REC_SIZE); PR;
		}
		ull = get_ull(rec);
		here = sdlog_read_tell();
		if(ull != here)
		{
			printf("[%d] %d == get_ull(rec), %d == sdlog_read_tell()\n\r",
															i, ull, here); PR;
		}
		
		for(j=8; j<REC_SIZE; j+=1)
			if(((rec[j] - j) & 0xff) != 0)
			{
				printf("# Bad data at %d\r\n", j); PR;
			}
	}

	read_marker = sdlog_read_tell();
	read_count = i;

	printf("sdlog_read_tell returns %d\n\r", (int)read_marker); PR;
	printf("Number of records read is %d\r\n", i); PR;

	NEXT_STATE(sdlog_test_02);
	printf("\r\n");
}

void sdlog_test_02(void)		/* sdlog_seek pass */
{
	int i, n;
	sdlog_seek_t target;
	unsigned long long ull;
	
	printf("## START of sdlog_test_02 (sdlog_seek forward pass)\n\r"); PR;
	
	target = sdlog_read_tell();

	for(i=0; i<read_count; i+=1)
	{
		n = sdlog_seek(target + 2);				/* Seek forwards 2 records */
		if(check("sdlog_seek", n))
			break;

		n = sdlog_read_backwards(rec, REC_SIZE);/* Read backwards 1 record */
		if(check("sdlog_read_backwards", n))
			break;

		ull = get_ull(rec);
		target = sdlog_read_tell();
		if(ull != target)
		{
			printf("pid is %d, read_tell returns %d \r\n", (int)ull, (int)target);
			PR;
			break;
		}
	}

	printf("Loop ran %d times\r\n", i);

	NEXT_STATE(sdlog_test_goto_idle);
	printf("\r\n");
}

void sdlog_test_goto_idle(void)
{
	printf("## START of sdlog_test_goto_idle\r\n"); PR;
	running = (1==0);

	NEXT_STATE(sdlog_test_00);	
	printf("\r\n");
}

/******************************************************************************/
extern int sdlog_err;

int check(char *name, int err)
{
	if(err < 0)
	{
		printf("# %s returns %d, sdlog_err is %d\r\n", name, err, sdlog_err);
		PR;
		return (1==1);
	}
	
	return (1==0);
}

unsigned long long get_ull(char *p)				/* Big endian! */
{
	unsigned long long acc = 0;
	int i;
	
	for(i=0; i<8; i+=1)
		acc = (acc << 8) | (*p++ & 0xff);
	
	return acc;
}

void put_ull(char *p, unsigned long long ull)	/* Big endian! */
{
	int i;
	
	for(i=7; i>=0; i-=1)
		*p++ = (char)((ull >> (8*i)) & 0xff);
}
