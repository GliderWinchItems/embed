/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : cic_filter_l_N2_M3.h
* Author             : deh
* Date First Issued  : 09/17/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : General purpose cic (long) filter for N(delays)=2, M(sections)=3
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CIC_FILTER_L_N2_M3
#define __CIC_FILTER_L_N2_M3


/* One of these for each value filtered will is required */
struct CICLN2M3
{
	unsigned short	usDecimateNum;	// Downsampling number
	unsigned short	usDiscard;	// Initial discard count
	int		nIn;		// New reading to be filtered
	long 		lIntegral[3];	// Three stages of Z^(-1) integrators
	long		lDiff[3][2];	// Three stages of Z^(-2) delay storage
	long		lout;		// Filtered/decimated data output
	unsigned short	usDecimateCt;	// Downsampling counter
	unsigned short	usFlag;		// Filtered/decimated data ready counter
};

/******************************************************************************/
unsigned short cic_filter_l_N2_M3 (struct CICLN2M3 *strP);
/* @brief	: Do three sinc filters
 * @param	: Pointer to struct with all the stuff
 * @return	: 0 = filtered output not ready; 1 = new filtered output ready
*******************************************************************************/

#endif 
