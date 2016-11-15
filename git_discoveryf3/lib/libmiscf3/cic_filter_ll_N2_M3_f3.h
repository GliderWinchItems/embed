/******************************************************************************
* File Name          : cic_filter_ll_N2_M3_f3.h
* Date First Issued  : 09/17/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : General purpose cic (long long) filter for N(delays)=2, M(sections)=3
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CIC_FILTER_LL_N2_M3_F3
#define __CIC_FILTER_LL_N2_M3_F3

/* One of these for each value filtered will is required */
struct CICLLN2M3
{
	unsigned short	usDecimateNum;	// Downsampling number
	unsigned short	usDiscard;	// Initial discard count
	int		nIn;		// New reading to be filtered
	long long	llIntegral[3];	// Three stages of Z^(-1) integrators
	long long	llDiff[3][2];	// Three stages of Z^(-2) delay storage
	long long	llout;		// Filtered/decimated data output
	unsigned short	usDecimateCt;	// Downsampling counter
	unsigned short	usFlag;		// Filtered/decimated data ready counter
};
/******************************************************************************/
unsigned short cic_filter_ll_N2_M3_f3 (struct CICLLN2M3 *strX);
/* @brief	: Do three sinc filters
 * @param	: Pointer to struct with all the stuff
 * @return	: 0 = filtered output not ready; 1 = new filtered output ready
*******************************************************************************/

#endif 
