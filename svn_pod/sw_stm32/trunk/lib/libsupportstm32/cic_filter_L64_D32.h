/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : cic_filter_L64_D32.h
* Author             : deh
* Date First Issued  : 09/08/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : general purpose filtering
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CICL64D32_FILTER
#define __CICL64D32_FILTER



/* One of these for each value filtered will is required */
struct CICL64D32
{
	int		nIn;		// New reading to be filtered
	long 		lIntegral[3];	// Three stages of Z^(-1) integrators
	long		lDiff[3][2];	// Three stages of Z^(-2) delay storage
	long		lout;		// Filtered/decimated data output
	unsigned short	usDecimateCt;	// Decimation counter
	unsigned short	usFlag;		// Filtered/decimated data ready counter
};

/******************************************************************************/
void cic_L64_D32_filter (struct CICL64D32 * strP);
/* @brief	: Do three sinc filters on the data passed in in the struct
 * @param	: pointer to struct with input, intermediate data, and flag
*******************************************************************************/

#endif 
