/******************************************************************************
* File Name          : cic_filter_l_N8_M3_f3.h
* Date First Issued  : 03/27/2016
* Board              : STM32F373 filtering
* Description        : General purpose cic long filter for N(delays)=8, M(sections)=3
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CIC_FILTER_L_N8_M3_F3
#define __CIC_FILTER_L_N8_M3_F3

#define CIC1M		3	// Number of stages
#define	CIC1N		8	// Number of delays
#define CIC1DECIMATE	8	// CIC downsampling number ('R')

/* One of these for each value filtered will is required */
struct CICLN8M3
{
	long	lIntegral[CIC1M];	// Three stages of Z^(-1) integrators
	long	lDiff[CIC1N][CIC1M];	// Three stages of Z^(-2) delay storage
	long	lout;			// Filtered/decimated data output
	long	*pDiff;			// Working ptr for lDiff
	long	*pDiff_end;		// End of lDiff[][]
	unsigned short	usDecimateCt;	// Downsampling counter
	unsigned short	usDecimate;	// Downsampling number
	unsigned char	ucChan;		// Channel number 
};
/******************************************************************************/
void cic_filter_l_N8_M3_f3_init (struct CICLN8M3 *pcic, unsigned short decimate);
/* @brief	: Setup the the struct with initial values
 * @param	: pcic = pointer to struct for this data stream
 * @param	: decimate = decimation number ("R")
*******************************************************************************/
unsigned short cic_filter_l_N8_M3_f3 (struct CICLN8M3 *pcic, long lval);
/* @brief	: Do three sinc filters* File Name  
 * @param       : pcic = pointer to struct with intermediate values and pointers
 * @param	: lval = output value of a new filtered output
 * @return	: 0 = filtered output not ready; 1 = new filtered output ready
*******************************************************************************/


#endif 
