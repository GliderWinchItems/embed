/************************ COPYRIGHT 2012 **************************************
* File Name          : p1_pushbuttontime_sd.h
* Generator          : deh
* Date First Issued  : 11/01/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Circular buffer of pushbutton times & SD card packet ids's
*******************************************************************************/
/*
The assignements of extra blocks in the SD card are done via a "chain of includes"
that contain assignments based on the previous include.

SD card extra block assignments --
"SDcard_fixed_assignments.h"



*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PBTIME_SD
#define __PBTIME_SD

#include "SDcard_fixed_assignments.h"


struct PBTIME
{
	unsigned long long ulltime;	// Extended linux time
	sdlog_seek_t	pid;		// unsigned long long with SD card packet ID
};

struct PBTIME_LOCATION
{
	int	blk;
	int	idx;
};



/* Number of launch times in each SD block */
#define NUM_PBTIME_PERBLOCK	(512/sizeof(struct LAUNCHTIME))	// Number of launches per SD block

/******************************************************************************/
void p1_pushbuttontime_init(void);
/* @brief	: Find latest entry in circular buffer of blocks
 ******************************************************************************/
void p1_pushbuttontime_add(unsigned long long ull);
/* @brief	: Add pushbutton entry
 * @param	: ull = linux time extended time for pushbutton event
 ******************************************************************************/
void p1_pushbuttontime_close(void);
/* @brief	: Detect the end of launch
 ******************************************************************************/
struct PBTIME p1_pushbuttontime_get_entry(int offset);
/* @brief	: Return the time in the entry that is 'offset' entries back in time
 * @param	: Offset (0 = current, +1 = next prior, etc.)
 * @return	: Time and PID of requested entry {0,0} returned when out-of-range
 ******************************************************************************/
int p1_pushbuttontime_set_start(struct PBTIME * plt, int offset);
/* @brief	: Position the SD readout to begin at the pushbuttontime entry time
 * @param	: plt = pointer to struct with time and pid
 * @param	: Offset (0 = current, +1 = next prior, etc.)
 * @return	: negative = seek error
 ******************************************************************************/


#endif 

