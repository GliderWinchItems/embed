/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : SDcard_fixed_assignments.h
* Author             : deh
* Date First Issued  : 11/01/2012
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Extra block (fixed address) assignments for SD Card
*******************************************************************************/
/*
11/01/2012
*/

#ifndef __SDFIXEDASSIGNMENT
#define __SDFIXEDASSIGNMENT

/* Calibration blocks */
#define EXTRA_BLOCK_CAL0	0	// Extra block number for calibration, 1st copy
#define EXTRA_BLOCK_CAL1	1	// Extra block number for calibration, 2nd copy
#define EXTRA_BLOCK_CAL_LAST	9	// Last block reserved for calibrations

/* Launchtime blocks */
#define	EXTRA_BLOCK_LAUNCHTIME_SIZE	10	// Number of blocks of launchtimes|pid
#define EXTRA_BLOCK_LAUNCHTIME_FIRST	(EXTRA_BLOCK_CAL_LAST + 1)	// Extra block number, first of many
#define EXTRA_BLOCK_LAUNCHTIME_LAST	(EXTRA_BLOCK_LAUNCHTIME_FIRST + EXTRA_BLOCK_LAUNCHTIME_SIZE - 1)	// Extra block number, last of many

/* Pushbuttontime blocks */
#define	EXTRA_BLOCK_PBTIME_SIZE	10	// Number of blocks of pushbutton times|pid
#define EXTRA_BLOCK_PBTIME_FIRST	(EXTRA_BLOCK_LAUNCHTIME_LAST + 1)	// Extra block number, first of many
#define EXTRA_BLOCK_PBTIME_LAST		(EXTRA_BLOCK_PBTIME_FIRST + EXTRA_BLOCK_PBTIME_SIZE - 1)	// Extra block number, last of many



#endif


