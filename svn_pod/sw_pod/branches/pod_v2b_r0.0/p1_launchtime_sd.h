/************************ COPYRIGHT 2012 **************************************
* File Name          : p1_launchtime_sd.h
* Generator          : deh
* Date First Issued  : 10/23/2012
* Board              : STM32F103VxT6_pod_mm
* Description        : Circular buffer of launchtimte & SD card packet ids's
*******************************************************************************/
/*
The assignements of extra blocks in the SD card are done via a "chain of includes"
that contain assignments based on the previous include.

SD card extra block priority of assignments--
p1_calibration_sd.h	Absolute definition.
p1_launchtime_sd.h	Relative to "calibration"

*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LAUNCHTIME_SD
#define __LAUNCHTIME_SD

#include "SDcard_fixed_assignments.h"


/* Delay tagging the end-of-launch after the packet with low tension is encountered, by counting packets */
#define LAUNCHTIME_DELAY	12	// Each count is 0.250 seconds

struct LAUNCHTIME
{
	unsigned long long ulltime;	// Extended linux time
	sdlog_seek_t	pid;		// unsigned long long with SD card packet ID
};

struct LAUNCHTIME_LOCATION
{
	int	blk;
	int	idx;
};



/* Number of launch times in each SD block */
#define NUM_LAUNCHTIME_PERBLOCK	(512/sizeof(struct LAUNCHTIME))	// Number of launches per SD block

/******************************************************************************/
void p1_launchtime_init(void);
/* @brief	: Find latest entry in circular buffer of blocks
 ******************************************************************************/
void p1_launchtime_detect(void);
/* @brief	: Detect the end of launch
 ******************************************************************************/
void p1_launchtime_close(void);
/* @brief	: Detect the end of launch
 ******************************************************************************/
struct LAUNCHTIME p1_launchtime_get_entry(int offset);
/* @brief	: Return the time in the entry that is 'offset' entries back in time
 * @param	: Offset (0 = current, +1 = next prior, etc.)
 * @return	: Time and PID of requested entry {0,0} returned when out-of-range
 ******************************************************************************/
int p1_launchtime_set_start(struct LAUNCHTIME * plt, int offset);
/* @brief	: Position the SD readout to begin at the launchtime entry time
 * @param	: plt = pointer to struct with time and pid
 * @param	: Offset (0 = current, +1 = next prior, etc.)
 * @return	: negative = seek error
 ******************************************************************************/


#endif 

