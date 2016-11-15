/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_calibration_sd.c
* Generator          : deh
* Date First Issued  : 10/27/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Calibration save/restore using SD card storage
*******************************************************************************/
/*
Calibrations peculiar to this part are stored in the SD Card.  The first 2048 blocks
of the SD Card are set aside for this type of storage.  

*/

#include "p1_common.h"

/*
Key for finding routines, definitions, variables, and other clap-trap
@1 = svn_pod/sw_stm32/trunk/lib/libsupportstm32/calibration.h


*/


/* First block with calibrations (and whatever else we might add later) */
union SDCALBLOCK0 sdcalblock0;



/* Change calibration flag --
   When shutting down, new data is written out.  If no changes skip writing. */
char cCalChangeFlag;	// 0 = no change, 1= something was changed

/* Reset SD calibration to default flag, i.e. force SD card calibrations to defaults */
char cCalDefaultflag;	// 0 = update with in-memory values; not-zero = reset to defaults


/******************************************************************************
 * static int datracheck (struct CALBLOCK * calp);
 * @brief	: Check the integrity of the calibration struct
 * @return	: 0 = SUCCESS; not-zero = something wrong.
 ******************************************************************************/
static int datacheck (struct CALBLOCK * calp)
{
	unsigned int i;
	unsigned int crc;
	unsigned int * p = (unsigned int *) calp;
	unsigned int temp = 0;

	/* First check if there is any data */
	for (i = 0; i < (sizeof (struct CALBLOCK))/(sizeof (int)); i++) // (@1)
	{
		if (*p++ != 0)
		{
			temp += 1;
		}
	}

	/* There should be at least a few non-zero entries! */
	if ( temp < 10) return 1;

	/* Check if crc is correct */
	if ( (crc = hwcrcgen( (unsigned int*)calp, (sizeof (struct CALBLOCK) )/(sizeof (int)) ) ) != 0) return crc;

	/* Check if battery calibrations are reasonable */
	if ( (calp->celllow > 3700) || (calp->celllow < 3100) ) return 2;

	return 0;	// SUCCESS!
}

/******************************************************************************
 * char p1_calibration_retrieve(void);
 * @brief	: Get the calibration for this unit & store in 'strDefaultCalib'
 * @return	: 0 = data from SD Card used; 1 = default calibration used
 ******************************************************************************/
/*
Calibration structs are saved in two SD blocks.  The first block is checked, and if that
doesn't pass the data checks, then the second block is checked.  If neither of the blocks
pass the test, the default calibration which was setup during 'p1_initialization basic' is 
left intact and will be used.  If the first block's calibration passes the tests then it is
copied to struct in static memory, and if it fails but the second SD block calibration passes
it is copied.  If either SD block calibration fails, then the flag is set so that when the
POD is placed in deepsleep the in-memory is written before shutdown.  (Well, at least that is
what I had in mind when I first set up this routine!)
*/
char p1_calibration_retrieve(void)
{
	union SDCALBLOCK0 sd;	// Space for one SD block containing the calibration struct

	/* Read one SD card block that holds the first calibration struct */
	sdlog_read_extra_block(EXTRA_BLOCK_CAL0, &sd.sdcalblock[0]);

	/* Check the integrity of the calibrations */
	if ( (datacheck(&sd.calblock)) != 0)
	{ // Here, it didn't past muster.  Try the another one.
		/* Read another SD card block that holds the calibration struct */
		sdlog_read_extra_block(EXTRA_BLOCK_CAL1, &sd.sdcalblock[0]);
		
		if ( (datacheck(&sd.calblock)) != 0)
		{ // Here, it didn't past muster.  Screwed!  Use the default...better than nothing!
			cCalChangeFlag = 1;	// Cause the shutdown 'close' to update the SD card calibrations
		}
		else
		{ // First one was bad, but second one is OK, so use it.
			strDefaultCalib = sd.calblock;	// Copy this SD calibration to one in static memory
		}

		/* Re-write the SD blocks that hold calibration struct copies */
		cCalChangeFlag = 1;	// Cause the shutdown 'close' to update the SD card calibrations
	}
	else
	{ // Here, first SD block calibration checked OK, so use it.  No need to update unless someone else tinkers with it.
		strDefaultCalib = sd.calblock;	// Copy this SD calibration to one in static memory		
	}

// Debugging
//caldump(&sd.calblock);
//caldump(&strDefaultCalib);

	return cCalChangeFlag;
}
/******************************************************************************
 * static void update_n(unsigned int blocknum);
 * @brief	: Read in SD extra block, copy calibration to it, and write block
 * @param	: Extra block, number
 ******************************************************************************/
static void update_n(unsigned int blocknum)
{
	union SDCALBLOCK0 sd;	// One SD block, with calibratrion struct inside 

	/* Read in one block with the old copy of calibration and whatever else is in the block */
	sdlog_read_extra_block(blocknum, sd.sdcalblock);
	
	/* If default flag is on, then set SD card calibration with default values */
// NOTE: no command currently implemented to set this flag (01/16/2012)
	if (cCalDefaultflag != 0)	// Reset to default?
	{ // Here, yes.  Re-initialize value to those compliled into the code
		calibration_init_default(&sd.calblock);

		/* Generate the CRC for the calibration struct */
		// hwcrcgen (pointer to calibration, count of ints for crc generation)
		sd.calblock.crc = hwcrcgen( (unsigned int*)&sd.calblock, (sizeof (struct CALBLOCK) - (sizeof (int)))/(sizeof (int)) );

		/* Write out block containing calibration */
		sdlog_write_extra_block(blocknum, sd.sdcalblock);
	}
	else
	{
		/* If no changes to calibration, then skip updating SD card (don't wear it out) */
		if (cCalChangeFlag != 0)
		{
			/* Copy in-memory calibration into struct area in block */
			sd.calblock = strDefaultCalib ;	// Copy

			/* Generate the CRC for the calibration struct */
			// hwcrcgen (pointer to calibration, count of ints for crc generation)
			sd.calblock.crc = hwcrcgen( (unsigned int*)&sd.calblock, (sizeof (struct CALBLOCK) - (sizeof (int)))/(sizeof (int)) );

			/* Write out block containing  of calibration */
			sdlog_write_extra_block(blocknum, sd.sdcalblock);
		}
	}
	return;
}
/******************************************************************************
 *  void p1_calibration_close0(void);
 * @brief	: Update multiple copies of the calibration values
 ******************************************************************************/
void p1_calibration_close0(void)
{
	/* Update multiple copies of the calibration values */
	update_n (EXTRA_BLOCK_CAL0);
	update_n (EXTRA_BLOCK_CAL1);

	return;
}
/******************************************************************************
 * DEBUGGING
 *  void caldump(struct CALBLOCK *s);
 * @brief	: Dump tables
 ******************************************************************************/
/******************************************************************************/
/* static char* hexchar (char* pout, char pin);                                */
/******************************************************************************/
static const char hextbl[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
static char* hexchar (char* pout, char in)
{
	*pout++ = hextbl[ ( (in >> 4) & 0x0f )];
	*pout++ = hextbl[  (in & 0x0f) ];
	return pout;

}

void caldump(struct CALBLOCK * s )
{
	unsigned int i;
	char * p = (char *)s;
	char vv[64];
	char * pout = &vv[0];


	for (i = 0; i < sizeof (struct CALBLOCK); i++)
	{
		pout = hexchar(pout,*p++); *pout++ = ' ';
		if ((i & 0x0f) == 0x0f) 
		{
			*pout = 0;
			USART1_txint_puts(vv);
			USART1_txint_puts("\n\r"); 
			USART1_txint_send();
			pout = vv;
		}
	}
	*pout = 0; USART1_txint_puts(vv); USART1_txint_puts("\n\r"); USART1_txint_send();

	return;
}

