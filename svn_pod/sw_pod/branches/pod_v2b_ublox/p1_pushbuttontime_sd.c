/************************ COPYRIGHT 2012 **************************************
* File Name          : p1_pushbuttontime_sd.c
* Generator          : deh
* Date First Issued  : 11/01/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Circular buffer of pushbutton times & SD card packet ids's
*******************************************************************************/

/*
The pushbutton times & SD card id are stored in a circular buffer.  The first 2048 blocks
of the SD Card are set aside for this type of storage.  

Strategy--

_init
Read each block in the circular buffer and find the latest time.  Save the block number
and position within block in static memory.

-detect
Poll and when there is a new packet available.  Go through the 16 readings and see if any 
are above the high tension limit which signals a pushbutton in progress.  Set a switch and 
then check new packets for tension below a low tension value which signals the end-of-pushbutton.
When there is an end of pushbutton, save the time and get the id of the last packet written.

When a block is filled it is written and block number advanced and read in.

--shutdown
If there has been any pushbutton time additions the current block is written before 
shutdown.


*/

#include "p1_common.h"
#include "p1_pushbuttontime_sd.h"
#include "SDcard_fixed_assignments.h"

/*
Key for finding routines, definitions, variables, and other clap-trap
@1 = yet to be done
*/


/*   When shutting down, new data is written to SD card.  If no changes skip writing. */
char cPBtimeChangeFlag;	// 0 = no change, 1= something was changed

/* This holds the current SD block of pushbuttontimes */
static struct PBTIME pbblock[NUM_PBTIME_PERBLOCK];	// One SD block of pushbutton times

/* This holds the SD block being used with the 'k' command readout */
static struct PBTIME pbblock1[NUM_PBTIME_PERBLOCK];	// One SD block of pushbutton times


/* These have the current SD block number of the block in memory, and index within the block */
//static struct PBTIME_LOCATION ptCur; // Current SD block & index, time & pid

struct PBTIME_LOCATION ptCur; // Current SD block & index, time & pid

/****************** DEBUG *****************************************************
 * static void printdetect(void);
 * @brief	: List position after detection
 ******************************************************************************/
static void printdetect(struct PBTIME lt)
{	
	int uitemp = (int)(lt.ulltime >> 11);
	uitemp += PODTIMEEPOCH;		// Adjust for shifted epoch
	printf(" Next PB pos: %2u %2u Last time: %11u %s\r", ptCur.blk,ptCur.idx,uitemp,ctime((const time_t*)&uitemp) ); USART1_txint_send();
	return;
}
/****************** DEBUG *****************************************************
 * static void clearblocks(void);
 * @brief	: DEBUGGING ROUTINE: Clear all pushbuttontime blocks
 ******************************************************************************/
static void clearblocks(void)
{
	int i,j;
	struct PBTIME strX = {0,0}; 
	/* Go through whole entire buffer and find the latest pushbutton time */
	for (i = 0; i < EXTRA_BLOCK_PBTIME_SIZE; i++)	// Go through all the blocks in the buffer
	{
		/* Read a SD card block that holds pushbutton times v SD packet ID */
		sdlog_read_extra_block( (EXTRA_BLOCK_PBTIME_FIRST + i), (char *)&pbblock1[0] );

		for (j = 0; j < (int)NUM_PBTIME_PERBLOCK; j++) pbblock1[j] = strX;	// Go through entries within a block
				
		sdlog_write_extra_block((EXTRA_BLOCK_PBTIME_FIRST + i), (char*)pbblock1);
	}
	return;
}
/****************** DEBUG *****************************************************
 * static void dumpblocks(void);
 * @brief	: DEBUGGING ROUTINE: Dump non-zero pushbuttontime blocks
 ******************************************************************************/
static void dumpblocks(void)
{
	int i,j;
	unsigned int uitemp;

	/* Go through whole entire buffer and list pushbutton times not zero */
	for (i = 0; i < EXTRA_BLOCK_PBTIME_SIZE; i++)	// Go through all the blocks in the buffer
	{
		/* Read a SD card block that holds pushbutton times v SD packet ID */
		sdlog_read_extra_block( (EXTRA_BLOCK_PBTIME_FIRST + i), (char *)&pbblock1[0] );

		for (j = 0; j < (int)(NUM_PBTIME_PERBLOCK); j++)	// Go through entries within a block
		{
			if (pbblock1[j].ulltime != 0)
			{
				uitemp = pbblock1[j].ulltime >> 11;	// Extened linux time -> linux time (secs)
				uitemp += PODTIMEEPOCH;			// Adjust for shifted epoch
				// Block, Index, Linux time, Linux time converted to ASCII (not elegant, but beautiful...)
printf("PBdump %2u %2u %11u %016x %016x %s\r", i,j,(int)(pbblock1[j].ulltime >> 11),(int)(pbblock1[j].pid >> 32),(int)(pbblock1[j].pid & 0xffffffff),ctime((const time_t*)&uitemp) ); 
USART1_txint_send();
			}
		}
	}
	return;
}
/******************************************************************************
 * void p1_pushbuttontime_init(void);
 * @brief	: Find latest entry in circular buffer of blocks
 ******************************************************************************/
void p1_pushbuttontime_init(void)
{
	int i;
	unsigned int j;
	unsigned long long ullX = 0;

	ptCur.idx = 0; 
	ptCur.blk = EXTRA_BLOCK_PBTIME_FIRST;

	/* Go through whole entire buffer and find the latest pushbutton time */
	for (i = EXTRA_BLOCK_PBTIME_FIRST; i <= EXTRA_BLOCK_PBTIME_LAST; i++)	// Go through all the blocks in the buffer
	{
		/* Read a SD card block that holds pushbutton times v SD packet ID */
		sdlog_read_extra_block( i, (char *)&pbblock[0] );

		/* Look for latest time and save block number and position */
		for (j = 0; j < NUM_PBTIME_PERBLOCK; j++)	// Go through entries within a block
		{
			if (pbblock[j].ulltime > ullX)	// Have we found a later time?
			{ // Here yes.
				ullX = pbblock[j].ulltime;	// Update latest time
				ptCur.idx = j; ptCur.blk = i; 	// SD block number & index within block
			}
		}
	}
	/* At this point 'ptCur.blk' and  'ptCur.idx' have the pushbuttontime block number and index within block for the latest time */

	/* Advance to next entry, i.e. .blk & .idx hold the next available entry position */
	ptCur.idx += 1;
	if (ptCur.idx >= (int)(NUM_PBTIME_PERBLOCK) ) 	// At the end of the block?
	{
		ptCur.idx  = 0;					// Reset index to entry within SD block
		ptCur.blk += 1;					// Advance to next SD block number
		if (ptCur.blk > EXTRA_BLOCK_PBTIME_LAST) ptCur.blk = EXTRA_BLOCK_PBTIME_FIRST;	// Buffer wrap-around
	}

	/* Get SD block that contains the latest pushbuttontime info */
	sdlog_read_extra_block(ptCur.blk, (char*)pbblock);
//	ptCur.lt.ulltime = pbblock[ptCur.idx].ulltime;
//	ptCur.lt.pid     = pbblock[ptCur.idx].pid;

// DEBUGGING
//clearblocks(); // Debug: clear the blocks 
//dumpblocks(); // Debug: dump non-zero blocks
//printf(" NEXT AVAILABLE PB POS: %u %u %u \n\r",ptCur.blk, ptCur.idx,(int)(pbblock[ptCur.idx].ulltime >> 11)); USART1_txint_send(); // Verify position
struct PBTIME ptx; ptx.ulltime = ullX;
printdetect(ptx);

	return;
clearblocks();dumpblocks(); // Get rid of compiler warning message when these routines are not used.
}
/******************************************************************************
 * void p1_pushbuttontime_add(unsigned long long ull);
 * @brief	: Add pushbutton entry
 * @param	: ull = linux time extended time for pushbutton event
 ******************************************************************************/
static unsigned short state = 0;	// State machine

void p1_pushbuttontime_add(unsigned long long ull)
{
static struct PBTIME debug;

	/* Don't be making tags with time entry until the time-base has been established, otherwise it is possible to get
	   huge & bogus times, which always appear to be the latest!  (Guess how I found out.) */
	if (cGPS_ready == 0) return;	// Return we don't have good & stable GPS time established.

	switch (state)
	{
	case 0: // Add a "pbtime" entry into the current SD block number, current index within the block
		pbblock[ptCur.idx].ulltime = ull;		// Add extended linux format time for this packet
		pbblock[ptCur.idx].pid = sdlog_write_tell(); 	// Get ID of last SD packet
		cPBtimeChangeFlag = 1;				// Show that there has been an addition (to assure it is saved at shutdown)

debug = pbblock[ptCur.idx]; // Debug: save new entry data for printdetect();

		/* Advance to the next entry (from the current entry) */
		// Note: .blk & .idx point to the currently filled entry
		ptCur.idx += 1;					// Advance to next position in circular buffer
	/* Note: the SD card block is update upon each pushbutton time detection.  Several reasons--
		1) If the POD is not put into sleep mode before the PC is hooked up the 'qq' command to readout the table
		   of pushbuttontime entries will not have the new data in the SD, yet the SD block and index will have been
		   updated.  The result is either zeros for time/pid or in the case of wrap-around old data.
		2) If the POD should fail the SD card table will have the latest entry.
		3) There are only 32 entries per block, so the re-write number for a block is not large.
	*/
		sdlog_write_extra_block(ptCur.blk, (char*)pbblock);	// Write out the latest 
		cPBtimeChangeFlag = 0;					// No need to write out block upon shutdown
		if (ptCur.idx >= (int)(NUM_PBTIME_PERBLOCK) ) 		// At the end of the block?
		{ // Here, yes.  The block has filled so we need to read in the next block
			state = 1;			// Upon next poll, read next SD block into memory
			break;
		}
		state = 0; 
printdetect(debug);	// Debug: display our handy work
		break;		
			
	case 1:	// This case is so that we don't tie up polling with consecutive write/reads of the SD card.
		ptCur.idx = 0;			// Reset index to entry within SD block
		ptCur.blk += 1;			// Advance to next SD block number
		if (ptCur.blk > EXTRA_BLOCK_PBTIME_LAST) ptCur.blk = EXTRA_BLOCK_PBTIME_FIRST;	// Buffer wrap-around
		sdlog_read_extra_block(ptCur.blk, (char*)pbblock);				// Read the block
		state = 0;
printdetect(debug);	// A new block is not so frequent
		break;

	} // End of 'switch(state)'
	return;
}	
/******************************************************************************
 * void p1_pushbuttontime_close(void);
 * @brief	: Write current SD block of pushbuttontimes if there has been a change
 ******************************************************************************/
void p1_pushbuttontime_close(void)
{
	if (cPBtimeChangeFlag == 0) return;
	sdlog_write_extra_block(ptCur.blk, (char*)pbblock);
	return;
}

/******************************************************************************
 * struct PBTIME p1_pushbuttontime_get_entry(int offset);
 * @brief	: Return the time in the entry that is 'offset' entries back in time
 * @param	: Offset (0 = current, +1 = next prior, etc.)
 * @return	: Time and PID of requested entry {0,0} returned when out-of-range
 ******************************************************************************/
/*
Note: The routine caller looks at entries as being relative to the current entry without
regard to blocking.   
*/
struct PBTIME p1_pushbuttontime_get_entry(int offset)
{
	struct PBTIME_LOCATION ptTmp = ptCur;	// Local copy of current blk & idx

	offset += 1;	// Since we point to next available, a zero offset backs us up one.

	/* Check for out-range-offset */
	if ( (offset/NUM_PBTIME_PERBLOCK) >= EXTRA_BLOCK_PBTIME_SIZE) return (struct PBTIME){0,0};

	/* Go backwards the number of blocks */
	ptTmp.blk -= (offset/NUM_PBTIME_PERBLOCK);
	
	/* Back up the number of entries within the block */
	ptTmp.idx -= (offset % NUM_PBTIME_PERBLOCK);
	if (ptTmp.idx < 0) 
	{ // Here, we went over a block boundary
		ptTmp.idx += NUM_PBTIME_PERBLOCK;	// Wrap around
		ptTmp.blk -= 1;				// Back up one block
	}
	if ( ptTmp.blk < EXTRA_BLOCK_PBTIME_FIRST ) ptTmp.blk += EXTRA_BLOCK_PBTIME_SIZE; // Wrap around

	/* Get the block with the requested entry */
	sdlog_read_extra_block(ptTmp.blk, (char*)pbblock1);	// Read the block

	/* Return the time & pid in the requrested entry */
	return pbblock1[ptTmp.idx];
}
/******************************************************************************
 * int p1_pushbuttontime_set_start(struct PBTIME * plt, int offset);
 * @brief	: Position the SD readout to begin at the pushbuttontime entry time
 * @param	: plt = pointer to struct with time and pid
 * @param	: Offset (0 = current, +1 = next prior, etc.)
 * @return	: negative = seek error
 ******************************************************************************/
int p1_pushbuttontime_set_start(struct PBTIME * plt, int offset)
{
	*plt = p1_pushbuttontime_get_entry(offset);	// Get location
printf (" PID %016x %016x \n\r",(int)(plt->pid>>32),(int)(plt->pid & 0x00ffffffff) ); USART1_txint_send();
	return sdlog_seek(plt->pid);		// Reset readout location of end-of-pb entry
	
}

