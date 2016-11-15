/******************************************************************************/
/* File: sdcard.h -- Interface for higher-level sdcard access.
 */
#ifndef __SDCARD_H_
#define	__SDCARD_H_

#include "sdcard_ll.h"

/* Use the following for both read and write buffers.
 */
#define	SDCARD_BLOCKSIZE (SDC_DATA_SIZE)
#define	SDCARD_CSD_SIZE  (SDC_CSD_SIZE)
#define	SDCARD_CID_SIZE  (SDC_CID_SIZE)

/* NOTE:
 * sdcard_init(...) must be the first function called.  All other functions
 * return an error code if called before sdcard_init(...).  Note that the
 * arguments can be either a pointer to a buffer that gets the metadata or
 * NULL, in which case the metadata is not returned.
 *
 * All calls are synchronous.
 */

/******************************************************************************/
int sdcard_init(void *cid_buf, void *csd_buf);
/*@brief	: Execute initialization sequence for SD Card
* @param	: char array to receive the CID data
* @param	: char array to receive the CSD data
* @return	: zero = success. non-zero = failed.
******************************************************************************/
int sdcard_read(int blocknum, void *buf);
int sdcard_write(int blocknum, void *buf);
/*@brief	: Read/write a block.
* @param	: Blocknumber (assume to be 512 byte block size)
* @param	: char array to receive/write data
* @return	: zero = success. non-zero = failed.
******************************************************************************/
int sdcard_erase(int first_block, int number_of_blocks);
/*@brief	: Erase a sequence of blocks.
* @param	: Blocknumber (assume to be 512 byte block size)
* @param	: Number of blocks
* @return	: zero = success. non-zero = failed.
******************************************************************************/
/* Retry interface:
 */
#define	RETRY_LIMIT		8						/* 1 try + 6 retries + 1 fail */

typedef struct
{
	unsigned long retries[RETRY_LIMIT];			/* Try/retry/fail counters */
} sdcard_retries_t;

void sdcard_retries_get(sdcard_retries_t *s);	/* Get a copy */
void sdcard_retries_clr(void);					/* Clear internal counters */

#endif

