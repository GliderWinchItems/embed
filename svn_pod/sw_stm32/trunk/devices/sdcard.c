/******************************************************************************/
/* File: sdcard.c -- Implementation for higher-level sdcard access.
 */
#include "sdcard.h"

/* Local variables and macros
 */
static int init_flag = (1==0);

static int v2_flag = (1==0);		/* false -> vers 1.X, true -> vers 2.00 */
static int hc_flag = (1==0);		/* true -> SDHC or SDXC */

/* The retry stuff.  Note that for "retries[n]", n==0 is the initial try count,
 * 0<n<(RETRY_LIMIT-1) are the nth retry counts, and n==(RETRY_LIMIT-1) is the
 * "retry failure" count.  Note that RETRY_LIMIT==2 means no retries. 
 */
static unsigned long retries[RETRY_LIMIT];	/* Try/retry/fail counters */

/* Debug stuff
 */
static volatile int HANG_flag = (1==1);
int caw_sdcard_HANG(int arg) { do {} while(HANG_flag); return arg;}
#define assert(bool) do if(!(bool)) caw_sdcard_HANG(__LINE__); while(1==0)

volatile int caw_saved_r;
int debug_save_r(int r, int err) { caw_saved_r = r; return err; }

/* Functions
 */
/******************************************************************************
* int sdcard_init(void *cid_buf, void *csd_buf);
* @brief	: Execute initialization sequence for SD Card
* @param	: char array to receive the CID data
* @param	: char array to receive the CSD data
* @return	: (1==1) for error, (1==0) for success
******************************************************************************/
int sdcard_init(void *cid_buf, void *csd_buf)
{
	#define	ACMD41_MAX	10000		/* How long to wait for ACMD41 to finish */
	int i, r;
	union {
		unsigned char cond[SDC_COND_SIZE];
		unsigned char ocr[SDC_OCR_SIZE];
		unsigned char cid[SDCARD_CID_SIZE];
		unsigned char csd[SDCARD_CSD_SIZE];
	} buf;	
	
	sdcard_ll_set_ccsf(1==0);						/* Init to reflect SDSC */

	r = sdcard_ll_init();							/* Init low level stuff */
	if(r != SDCARD_LL_OK)
		return debug_save_r(r, -1);
		
	for(i=0; i<RETRY_LIMIT-1; i+=1)					/* CMD0 gets retries */
	{
		retries[i] += 1;
		r = sdcard_ll_cmd(CMD0, 0, NULL);			/* GO_IDLE_STATE */
		if(r == R1_IN_IDLE_STATE)
			break;
	}
	if(i == RETRY_LIMIT-1)							/* Did we hit limit? */
	{
		retries[RETRY_LIMIT-1] += 1;				/* Yes! */
		return debug_save_r(r, -2);
	}

	r = sdcard_ll_cmd(CMD8, 0x1AA, buf.cond);		/* SEND_IF_COND */
	v2_flag = ((r & R1_ILLEGAL_CMD) == 0);			/* V2.00 spec.? */
	if((r & ~(R1_ILLEGAL_CMD | R1_IN_IDLE_STATE)) != R1_R2_R1X_OK)
		return debug_save_r(r, -3);

	r = sdcard_ll_cmd(CMD58, 0, buf.ocr);			/* READ_OCR */
	if(r != R1_IN_IDLE_STATE)
		return debug_save_r(r, -4);
		
	for(i=0; i<ACMD41_MAX; i+=1)
	{
		r = sdcard_ll_cmd(ACMD41, (1<<30), NULL);	/* SD_SEND_OP_COND w/ HCS */
		if((r & (R1X_IN_IDLE_STATE | R1_IN_IDLE_STATE)) == 0)
			break;
	}
	if(i == ACMD41_MAX)
		return debug_save_r(r, -5);

	r = sdcard_ll_cmd(CMD59, 1, NULL);				/* CRC_ON_OFF -- on */
	if(r != R1_R2_R1X_OK)
		return debug_save_r(r, -6);

	if(v2_flag)										/* V2.00 cards only */
	{
		r = sdcard_ll_cmd(CMD58, 0, buf.ocr);		/* READ_OCR again! */
		if(r != R1_R2_R1X_OK)
			return debug_save_r(r, -7);
		if((buf.ocr[0] & 0x40) != 0)				/* SDHC or SDXC? */
		{
			sdcard_ll_set_ccsf(1==1); 				/* Set CCS flag */
			hc_flag = (1==1);						/* Set local flag */
		}
	}

	if(csd_buf != NULL)								/* SEND_CSD */
	{
		r = sdcard_ll_cmd(CMD9, 0, csd_buf);
		if(r != SDCARD_LL_OK)
			return debug_save_r(r, -8);
	}

	if(cid_buf != NULL)								/* SEND_CID */
	{
		r = sdcard_ll_cmd(CMD10, 0, cid_buf);
		if(r != SDCARD_LL_OK)
			return debug_save_r(r, -9);
	}

	init_flag = (1==1);
	return 0;										/* SUCCESS! */
}

/******************************************************************************
* int sdcard_read(int blocknum, void *buf);
* int sdcard_write(int blocknum, void *buf);
* @brief	: Read/write a block.
* @param	: Blocknumber (assume to be 512 byte block size)
* @param	: char array to receive/write data
* @return	: zero = OK. not zero = failed
******************************************************************************/
int sdcard_read(int blocknum, void *buf)
{
	int r, n;
	
	assert(init_flag);
	assert(blocknum >= 0);
	assert(buf != NULL);

	for(n=0; n<RETRY_LIMIT-1; n+=1)
	{
		retries[n] += 1;
		
		r = sdcard_ll_cmd(CMD17, blocknum, buf);
		if(r == SDCARD_LL_CHECKSUM)
			continue;
		if(r != R1_R2_R1X_OK)
			return r;	
		r = sdcard_ll_cmd(CMD13, blocknum, NULL);
		return r;
	}
	
	retries[RETRY_LIMIT-1] += 1;
	return SDCARD_LL_CHECKSUM;
}

int sdcard_write(int blocknum, void *buf)
{
	int r, n;
	
	assert(init_flag);
	assert(blocknum >= 0);
	assert(buf != NULL);

	for(n=0; n<RETRY_LIMIT-1; n+=1)
	{
		retries[n] += 1;
		
		r = sdcard_ll_cmd(CMD24, blocknum, buf);
		if(r == SDCARD_LL_CHECKSUM)
			continue;
		if(r != R1_R2_R1X_OK)
			return r;	
		r = sdcard_ll_cmd(CMD13, blocknum, NULL);
		if((r & R2_WP_VIOLATON) == R1_R2_R1X_OK)
			return r;
	
		/* WP-violation here.  Try to unprotect this block and retry the write.
		 */
		r = sdcard_ll_cmd(CMD29, blocknum, NULL);
		if(r != R1_R2_R1X_OK)
			return r;
		r = sdcard_ll_cmd(CMD24, blocknum, buf);
		if(r == SDCARD_LL_CHECKSUM)
			continue;
		if(r != R1_R2_R1X_OK)
			return r;	
		r = sdcard_ll_cmd(CMD13, blocknum, NULL);
		return r;
	}
	
	retries[RETRY_LIMIT-1] += 1;
	return SDCARD_LL_CHECKSUM;
}

/******************************************************************************
* int sdcard_erase(int first_block, int number_of_blocks);
* @brief	: Read/write one block.
* @param	: Blocknumber (assume to be 512 byte block size)
* @param	: Number of blocks
* @return	: zero = OK. not zero = failed
******************************************************************************/
int sdcard_erase(int first_block, int number_of_blocks)
{
	assert(first_block >= 0);
	assert(number_of_blocks >= 0);
	if(number_of_blocks == 0)
		return (1==0);

	sdcard_ll_cmd(CMD32, first_block+0, NULL);
	sdcard_ll_cmd(CMD33, first_block+number_of_blocks-1, NULL);
	return sdcard_ll_cmd(CMD38, 0, NULL);
}

/*******************************************************************************
 * Retry interface.
 */
void sdcard_retries_get(sdcard_retries_t *s)	/* Get a copy */
{
	unsigned long *p = retries;
	unsigned long *q = s->retries;
	unsigned long *r = s->retries+RETRY_LIMIT;
	
	while(q < r)
		*q++ = *p++;
}

void sdcard_retries_clr(void)					/* Clear internal counters */
{
	unsigned long *q = retries;
	unsigned long *r = retries+RETRY_LIMIT;
	
	while(q < r)
		*q++ = 0UL;
}

