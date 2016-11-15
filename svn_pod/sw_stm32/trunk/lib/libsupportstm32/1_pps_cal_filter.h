/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : 1_pps_cal_filter.h
* Hackee             : deh
* Date First Issued  : 08/09/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Filter input capture time differences bewteen GPS 1_PPS & RTC 1 sec alarm
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PPS_CAL_FILTER
#define __PPS_CAL_FILTER

/* Down sampling ratio */
#define PPSDECIMATE	16	// Downsampling: "R" in the equations	

/* The first few readings maybe radically large, so discard them when starting out */
#define PPSDISCARDNUMBER	3	// Number of initial readings to discard

/* Variables contributed by this masterful set of routines */
struct PPSCALFILT
{
long 		lIntegral[3];		// Three stages of Z^(-1) integrators
long		lDiff[3][2];		// Three stages of Z^(-2) delay storage
long		l1PPS_out;		// Filtered/decimated data output
unsigned short	usDecimateCt;		// Downsampling counter
unsigned short	us1PPSfilterFlag;	// Incremented at each output time
unsigned short	us1PPSfilterFlagPrev;	// Up to user to update
unsigned short	sDiscard;		// Counter for discarding
};

/******************************************************************************/
unsigned int pps_cal_filter (struct PPSCALFILT *strX, int nInput);
/* @brief	: Do three sinc filters
 * @param	: strX: filters registers
 * @param	: nInput = reading
 * @return	: Ready flag: 0 = no; 1 = yes
*******************************************************************************/

#endif 
