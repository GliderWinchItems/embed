/************************ COPYRIGHT 2011 **************************************
* File Name          : p1_calibration_sd.h
* Generator          : deh
* Date First Issued  : 10/27/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Calibration save/restore using SD card storage
*******************************************************************************/
/*

*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CALIBRATION_SD
#define __CALIBRATION_SD

#define EXTRA_BLOCK_CAL0	0	// Extra block number for calibration, 1st copy
#define EXTRA_BLOCK_CAL1	1	// Extra block number for calibration, 2nd copy

/******************************************************************************/
char p1_calibration_retrieve(void);
/* @brief	: Get the calibration for this unit & store in 'strDefaultCalib'
 * @return	: 0 = data from SD Card used; 1 = default calibration used
 ******************************************************************************/
void p1_calibration_close0(void);
/* @brief	: Write the calibration to SD if there was a change
 ******************************************************************************/

/******************************************************************************/
/* ==== DEBUGGING ===== */
void caldump(struct CALBLOCK * s);
/* @brief	: Dump tables
 ******************************************************************************/


/* Change calibration flag --
   When shutting down, new data is written out.  If no changes skip writing. */
extern char cCalChangeFlag;	// 0 = no change, 1= something was changed

/* Update calibration in SD with current in-memory, versus reset to default values */
extern char cCalDefaultflag;	// 0 = update with in-memory values; not-zero = reset to defaults


#endif 

