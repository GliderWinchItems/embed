/************************ COPYRIGHT 2012 **************************************
* File Name          : p1_launchtime_sd.c
* Generator          : deh
* Date First Issued  : 10/23/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Circular buffer of launchtimte & SD card packet ids's
*******************************************************************************/

/*
The launch times & SD card id are stored in a circular buffer.  The first 2048 blocks
of the SD Card are set aside for this type of storage.  

Strategy--

_init
Read each block in the circular buffer and find the latest time.  Save the block number
and position within block in static memory.

-detect
Poll and when there is a new packet available.  Go through the 16 readings and see if any 
are above the high tension limit which signals a launch in progress.  Set a switch and 
then check new packets for tension below a low tension value which signals the end-of-launch.
When there is an end of launch, save the time and get the id of the last packet written.

When a block is filled it is written and block number advanced and read in.

--shutdown
If there has been any launch time additions the current block is written before 
shutdown.


*/

#include "p1_common.h"
#include "p1_launchtime_sd.h"
#include "SDcard_fixed_assignments.h"

/*
Key for finding routines, definitions, variables, and other clap-trap
@1 = yet to be done


*/




/*   When shutting down, new data is written to SD card.  If no changes skip writing. */
char cLaunchttimeChangeFlag;	// 0 = no change, 1= something was changed

/* This holds the current SD block of launchtimes */
static struct LAUNCHTIME launchblock[NUM_LAUNCHTIME_PERBLOCK];	// One SD block of launch times

/* This holds the SD block being used with the 'k' command readout */
static struct LAUNCHTIME launchblock1[NUM_LAUNCHTIME_PERBLOCK];	// One SD block of launch times


/* These have the current SD block number of the block in memory, and index within the block */
//static struct LAUNCHTIME_LOCATION ltCur; // Current SD block & index, time & pid

struct LAUNCHTIME_LOCATION ltCur; // Current SD block & index, time & pid

/* Count consecutive high tension readings needed to call it a launch */
static u32	launchtimectr = 0;	// 

/****************** DEBUG *****************************************************
 * static void printdetect(void);
 * @brief	: List position after detection
 ******************************************************************************/
static void printdetect(struct LAUNCHTIME lt)
{	
	int uitemp = (int)(lt.ulltime >> 11);
	uitemp += PODTIMEEPOCH;		// Adjust for shifted epoch
	printf(" Next LT pos: %2u %2u Last time: %11u %s\r", ltCur.blk,ltCur.idx,uitemp,ctime((const time_t*)&uitemp) ); USART1_txint_send();
	return;
}
/****************** DEBUG *****************************************************
 * static void clearblocks(void);
 * @brief	: DEBUGGING ROUTINE: Clear all launchtime blocks
 ******************************************************************************/
static void clearblocks(void)
{
	int i,j;
	struct LAUNCHTIME strX = {0,0}; 
	/* Go through whole entire buffer and find the latest launch time */
	for (i = 0; i < EXTRA_BLOCK_LAUNCHTIME_SIZE; i++)	// Go through all the blocks in the buffer
	{
		/* Read a SD card block that holds launch times v SD packet ID */
		sdlog_read_extra_block( (EXTRA_BLOCK_LAUNCHTIME_FIRST + i), (char *)&launchblock1[0] );

		for (j = 0; j < (int)NUM_LAUNCHTIME_PERBLOCK; j++) launchblock1[j] = strX;	// Go through entries within a block
				
		sdlog_write_extra_block((EXTRA_BLOCK_LAUNCHTIME_FIRST + i), (char*)launchblock1);
	}
	return;
}
/****************** DEBUG *****************************************************
 * static void dumpblocks(void);
 * @brief	: DEBUGGING ROUTINE: Dump non-zero launchtime blocks
 ******************************************************************************/
static void dumpblocks(void)
{
	int i,j;
	unsigned int uitemp;

	/* Go through whole entire buffer and list launch times not zero */
	for (i = 0; i < EXTRA_BLOCK_LAUNCHTIME_SIZE; i++)	// Go through all the blocks in the buffer
	{
		/* Read a SD card block that holds launch times v SD packet ID */
		sdlog_read_extra_block( (EXTRA_BLOCK_LAUNCHTIME_FIRST + i), (char *)&launchblock1[0] );

		for (j = 0; j < (int)(NUM_LAUNCHTIME_PERBLOCK); j++)	// Go through entries within a block
		{
			if (launchblock1[j].ulltime != 0)
			{
				uitemp = launchblock1[j].ulltime >> 11;	// Extened linux time -> linux time (secs)
				uitemp += PODTIMEEPOCH;			// Adjust for shifted epoch
				// Block, Index, Linux time, Linux time converted to ASCII (not elegant, but beautiful...)
printf(" %2u %2u %11u %016x %016x %s\r", i,j,(int)(launchblock1[j].ulltime >> 11),(int)(launchblock1[j].pid >> 32),(int)(launchblock1[j].pid & 0xffffffff),ctime((const time_t*)&uitemp) ); 
USART1_txint_send();
			}
		}
	}
	return;
}
/******************************************************************************
 * void p1_launchtime_init(void);
 * @brief	: Find latest entry in circular buffer of blocks
 ******************************************************************************/
void p1_launchtime_init(void)
{
	int i;
	unsigned int j;
	unsigned long long ullX = 0;

	ltCur.idx = 0; 
	ltCur.blk = EXTRA_BLOCK_LAUNCHTIME_FIRST;

	/* Go through whole entire buffer and find the latest launch time */
	for (i = EXTRA_BLOCK_LAUNCHTIME_FIRST; i <= EXTRA_BLOCK_LAUNCHTIME_LAST; i++)	// Go through all the blocks in the buffer
	{
		/* Read a SD card block that holds launch times v SD packet ID */
		sdlog_read_extra_block( i, (char *)&launchblock[0] );

		/* Look for latest time and save block number and position */
		for (j = 0; j < NUM_LAUNCHTIME_PERBLOCK; j++)	// Go through entries within a block
		{
			if (launchblock[j].ulltime > ullX)	// Have we found a later time?
			{ // Here yes.
				ullX = launchblock[j].ulltime;	// Update latest time
				ltCur.idx = j; ltCur.blk = i; 	// SD block number & index within block
			}
		}
	}
	/* At this point 'ltCur.blk' and  'ltCur.idx' have the launchtime block number and index within block for the latest time */

	/* Advance to next entry, i.e. .blk & .idx hold the next available entry position */
	ltCur.idx += 1;
	if (ltCur.idx >= (int)(NUM_LAUNCHTIME_PERBLOCK) ) 	// At the end of the block?
	{
		ltCur.idx  = 0;					// Reset index to entry within SD block
		ltCur.blk += 1;					// Advance to next SD block number
		if (ltCur.blk > EXTRA_BLOCK_LAUNCHTIME_LAST) ltCur.blk = EXTRA_BLOCK_LAUNCHTIME_FIRST;	// Buffer wrap-around
	}

	/* Get SD block that contains the latest launchtime info */
	sdlog_read_extra_block(ltCur.blk, (char*)launchblock);
//	ltCur.lt.ulltime = launchblock[ltCur.idx].ulltime;
//	ltCur.lt.pid     = launchblock[ltCur.idx].pid;

// DEBUGGING
//clearblocks(); // Debug: clear the blocks 
//dumpblocks(); // Debug: dump non-zero blocks
//printf(" NEXT AVAILABLE POS: %u %u %u \n\r",ltCur.blk, ltCur.idx,(int)(launchblock[ltCur.idx].ulltime >> 11)); USART1_txint_send(); // Verify position
struct LAUNCHTIME ltx; ltx.ulltime = ullX;
printdetect(ltx);

	return;
clearblocks();dumpblocks(); // Get rid of compiler warning message when these routines are not used.
}
/******************************************************************************
 * void p1_launchtime_detect(void);
 * @brief	: Detect the end of launch
 ******************************************************************************/
static unsigned short state = 0;	// State machine
static short delayctr;			// Counter for delaying tagging at end-of-launch
static short cUpDn = 0;			// Up/down tension hysteresis switch

void p1_launchtime_detect(void)
{
	struct PKT_AD7799 * pp_ad7799;		// Pointer to tension packet being examined
	struct PKT_PTR pp_launch;		// Ptr & ct to current
	long long lltemp;
	int ntemp;
	int i;

static struct LAUNCHTIME debug;

	/* Don't be making tags with time entry until the time-base has been established, otherwise it is possible to get
	   huge & bogus times, which always appear to be the latest!  (Guess how I found out.) */
	if (cGPS_ready == 0) return;	// Return we don't have good & stable GPS time established.

	switch (state)
	{
	case 0:	// Look through a packet for tension going up and down
		pp_launch = ad7799_packetize_get_launchtime();	// Check if packet ready
		if (pp_launch.ct == 0) return;			// Skip if no packets buffered
		pp_ad7799 = (struct PKT_AD7799 *)pp_launch.ptr;	// Convert Char pointer to struct pointer
		for (i = 0; i < PKT_AD7799_TENSION_SIZE; i++)	// Check readings within the packet
		{
			/* Convert raw reading into a calibrated reading to make life easy */
			/* Slight adjustment of the load_cell and ad7799 offset for zero */
			lltemp = pp_ad7799->tension[i] + strDefaultCalib.load_cell_zero;
			/* Convert to kgs * 1000 (which I supposed one might call "grams"!) */
			lltemp = lltemp*10000 / strDefaultCalib.load_cell;	// Gain scaling
			ntemp = (int)lltemp;	// Convert back to signed 32 bits
			/* ntemp is now exactly the same as would be sent to the PC upon readout */
			switch (cUpDn)	// Looking for tension increasing or decreasing
			{
			case 0:	 // Here, yes, looking for high tension signifying launch started			
				if (ntemp > LAUNCHTIME_TENSION_UP)
				{ // Here, early phase of launch detected
//printf("LTen00: %d\n\r", ntemp ); USART1_txint_send();

					if (ntemp > LAUNCHTIME_TENSION_OFFSCALE)	// Bogus reading?
					{ // Here, off-scale reading; a connector problem?
						cUpDn = 2;	// Wait for it to come down, then reset.
					}
					else
					{ // Here, reading above threshold, but not way out of whack
						launchtimectr += 1;	// Count 1/64th sec ticks
						if (launchtimectr > LAUNCHTIME_CTR)	// Enough time elapsed?
						{ // Here, it's been high long enough for it to be a legit launch.
							cUpDn = 1;	// Next, look for ending
						}
					}		
				}
				else
				{
					launchtimectr = 0;	// Reset above-threshold counter
				}
				break;

			case 1: // Here looking for tension to drop to a low value signifying end of launch
//printf("LTen01: %d\n\r", ntemp ); USART1_txint_send();

				if (ntemp < LAUNCHTIME_TENSION_DN)
				{ // Here, end-of-launch detected
					launchtimectr = 0;	// Reset above-threshold counter
					cUpDn = 0;		// Tension on the way down
					delayctr = 0;		// Delay counter at end of launch
					state = 1;		// Next, time delay to add packets by counting packets
				}
				break;

			case 2:	// Here we have an off-scale reading
//printf("LTen02: %d\n\r", ntemp ); USART1_txint_send();


				if (ntemp < LAUNCHTIME_TENSION_DN)
				{ // Here, readings are below the going-upward threshold.
					cUpDn = 0;		// Restart detection
					launchtimectr = 0;	// Reset above-threshold counter
				}
				break;

			} // End of 'switch(cUpDn)'
		}
		break;
		
	case 1:  // Delay tagging.  Here we are adding data at the end by counting tensions readings
//printf("LTen11a: %d\n\r", ntemp ); USART1_txint_send();


		pp_launch = ad7799_packetize_get_launchtime();	// Check if packet ready
		if (pp_launch.ct == 0) return;			// Skip if no packets buffered
		pp_ad7799 = (struct PKT_AD7799 *)pp_launch.ptr;	// Convert Char pointer to struct pointer

		delayctr += 1;	// Count tension packets (0.25 secs per packet (16 entries per packet))
		if (delayctr <= LAUNCHTIME_DELAY) break;
//printf("LTen11b: %d\n\r", ntemp ); USART1_txint_send();

		/* Add a "launchtime" entry into the current SD block number, current index within the block */
		launchblock[ltCur.idx].ulltime = pp_ad7799->U.ull;	// Add extended linux format time for this packet
		launchblock[ltCur.idx].pid     = sdlog_write_tell(); 	// Get ID of last SD packet
		cLaunchttimeChangeFlag = 1;				// Show that there has been an addition (to assure it is saved at shutdown)

debug = launchblock[ltCur.idx]; // Debug: save new entry data for printdetect();

		/* Advance to the next entry (from the current entry) */
		// Note: .blk & .idx point to the currently filled entry
		ltCur.idx += 1;						// Advance to next position in circular buffer
	/* Note: the SD card block is updated upon each launch time detection.  Several reasons--
		1) If the POD is not put into sleep mode before the PC is hooked up the 'kk' command to readout the table
		   of launchtime entries will not have the new data in the SD, yet the SD block and index will have been
		   updated.  The result is either zeros for time/pid or in the case of wrap-around old data.
		2) If the POD should fail the SD card table will have the latest entry.
		3) There are only 32 entries per block, so the re-write number for a block is not large.
	*/
		sdlog_write_extra_block(ltCur.blk, (char*)launchblock);	// Write out the latest 
		cLaunchttimeChangeFlag = 0;				// No need to write out block upon shutdown
		if (ltCur.idx >= (int)(NUM_LAUNCHTIME_PERBLOCK) ) 	// At the end of the block?
		{ // Here, yes.  The block has filled so we need to read in the next block
			state = 2;		// Upon next poll, read next SD block into memory
			break;
		}
		state = 0; 
printdetect(debug);	// Debug: display our handy work
		break;		
			
	case 2:	// This case is so that we don't tie up polling with consecutive write/reads of the SD card.
		ltCur.idx = 0;			// Reset index to entry within SD block
		ltCur.blk += 1;			// Advance to next SD block number
		if (ltCur.blk > EXTRA_BLOCK_LAUNCHTIME_LAST) ltCur.blk = EXTRA_BLOCK_LAUNCHTIME_FIRST;	// Buffer wrap-around
		sdlog_read_extra_block(ltCur.blk, (char*)launchblock);					// Read the block
		state = 0;
printdetect(debug);	// A new block is not so frequent
		break;

	} // End of 'switch(state)'
	return;
}	
/******************************************************************************
 * void p1_launchtime_close(void);
 * @brief	: Write current SD block of launchtimes if there has been a change
 ******************************************************************************/
void p1_launchtime_close(void)
{
	if (cLaunchttimeChangeFlag == 0) return;
	sdlog_write_extra_block(ltCur.blk, (char*)launchblock);
	return;
}

/******************************************************************************
 * struct LAUNCHTIME p1_launchtime_get_entry(int offset);
 * @brief	: Return the time in the entry that is 'offset' entries back in time
 * @param	: Offset (0 = current, +1 = next prior, etc.)
 * @return	: Time and PID of requested entry {0,0} returned when out-of-range
 ******************************************************************************/
/*
Note: The routine caller looks at entries as being relative to the current entry without
regard to blocking.   
*/
struct LAUNCHTIME p1_launchtime_get_entry(int offset)
{
	struct LAUNCHTIME_LOCATION ltTmp = ltCur;	// Local copy of current blk & idx

	offset += 1;	// Since we point to next available, a zero offset backs us up one.

	/* Check for out-range-offset */
	if ( (offset/NUM_LAUNCHTIME_PERBLOCK) >= EXTRA_BLOCK_LAUNCHTIME_SIZE) return (struct LAUNCHTIME){0,0};

	/* Go backwards the number of blocks */
	ltTmp.blk -= (offset/NUM_LAUNCHTIME_PERBLOCK);
	
	/* Back up the number of entries within the block */
	ltTmp.idx -= (offset % NUM_LAUNCHTIME_PERBLOCK);
	if (ltTmp.idx < 0) 
	{ // Here, we went over a block boundary
		ltTmp.idx += NUM_LAUNCHTIME_PERBLOCK;	// Wrap around
		ltTmp.blk -= 1;				// Back up one block
	}
	if ( ltTmp.blk < EXTRA_BLOCK_LAUNCHTIME_FIRST ) ltTmp.blk += EXTRA_BLOCK_LAUNCHTIME_SIZE; // Wrap around

	/* Get the block with the requested entry */
	sdlog_read_extra_block(ltTmp.blk, (char*)launchblock1);	// Read the block

	/* Return the time & pid in the requrested entry */
	return launchblock1[ltTmp.idx];
}
/******************************************************************************
 * int p1_launchtime_set_start(struct LAUNCHTIME * plt, int offset);
 * @brief	: Position the SD readout to begin at the launchtime entry time
 * @param	: plt = pointer to struct with time and pid
 * @param	: Offset (0 = current, +1 = next prior, etc.)
 * @return	: negative = seek error
 ******************************************************************************/
int p1_launchtime_set_start(struct LAUNCHTIME * plt, int offset)
{
	*plt = p1_launchtime_get_entry(offset);	// Get location
printf (" PID %016x %016x \n\r",(int)(plt->pid>>32),(int)(plt->pid & 0x00ffffffff) ); USART1_txint_send();
	return sdlog_seek(plt->pid);		// Reset readout location of end-of-launch entry
	
}

