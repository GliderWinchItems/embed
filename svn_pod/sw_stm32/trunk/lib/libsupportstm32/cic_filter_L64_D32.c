/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : cic_filter_L64_D32.c
* Author             : deh
* Date First Issued  : 09/08/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : general purpose adc filtering
*******************************************************************************/
/*
ad7799_filter.c implement a three Cascaded Integrator Comb (CIC) filter.  (See
_Digital Signal Processing in Communication Systems_, Marvin E. Frerking, (1994; Kluwer Academic Publishing),
page 200, Figure 5.33 for a two stage CIC filter).  

This filter is length 64 with down-sampling by 32.  The AD7799 is polled at 32768Hz divided by 16
(the TR_CLK rate), which is 2048 Hz.  The output of filter is therefore at 2048/32 -> 64 Hz.

With a down-sampling of 32 (5 bits), each integrator requires 5 more bits than the largest
data.  Three cascaded integrators add 15 bits to the 12 bit adc size.  Therefore, 32b 'ints'
are sufficient for the arithmetic.

The scale factor of this filter implementation is 2^18.  The scale factor is (R*N)^M, where R is the down-sampling
ratio (32 in this case), N is the number of delays in the differentiators (2 in this case) and
M is the number of cascaded sections (3 in this case); hence (32*2)^3 = 2^18. (See Frerking
p 202.)  If N is not a power of 2 then the scale factor will not be a power of two and
simple shifting of the output data will not de-scale the filter (if exact de-scaling is
important, which in this application it is not).

Since 4-5 effective bits are added by the low pass filtering, adjustment of the output by 2^18
could drop some useful bits, depending on the noise bits.

It is absolutely essential that the integrators be intialized to zero.  There is nothing
in the math to correct for a non-zero start up, and the error accumulates.

The three cascaded sections give an impulse response that is parabolic and therefore quite
close to Gaussian, which minimizes ringing.

*/

#include "cic_filter_L64_D32.h"

#define CICDECIMATE	32	

/******************************************************************************
 * void cic_L64_D32_filter (struct CICL64D32 * strP);
 * @brief	: Do three sinc filters on the data passed in in the struct
 * @param	: pointer to struct with input, intermediate data, and flag
*******************************************************************************/
void cic_L64_D32_filter (struct CICL64D32 * strP)
{

long	lX1,lX2;	// Intermediate differentiator value

	/* Three stages of integration */
	strP->lIntegral[0] += strP->nIn;		// Incoming data is 32 bits; add to 64 bits accumulator
	strP->lIntegral[1] += strP->lIntegral[0];		// 1st stage feeds 2nd stage
	strP->lIntegral[2] += strP->lIntegral[1];		// 2nd stage feeds 3rd stage
	
	/* Decimate (down-sample) */
	strP->usDecimateCt += 1;			// Decimation count
	if (strP->usDecimateCt >= CICDECIMATE) 	// Time to decimate?
	{ // Here, yes.  Do three stages of differentiation of the down sampled data
		strP->usDecimateCt = 0;		// Reset decimation counter
		lX1	           = strP->lIntegral[2] - strP->lDiff[0][1]; // 3rd stage integral minus 3rd stage value delayed by 2
		strP->lDiff[0][1] = strP->lDiff[0][0];	// Move the samples down one step
		strP->lDiff[0][0] = strP->lIntegral[2];	// Input goes into 1st delay of the two delays

		/* Repeat differentiator for 2nd stage */
		lX2               = lX1 - strP->lDiff[1][1]; // 1st stage diff- value delayed by 3
		strP->lDiff[1][1] = strP->lDiff[1][0];
		strP->lDiff[1][0] = lX1;

		/* Repeat for 3rd stage.  Output is the filtered/decimated output */
		strP->lout        = lX2  - strP->lDiff[2][1];
		strP->lDiff[2][1] = strP->lDiff[2][0];
		strP->lDiff[2][0] = lX2;

		strP->usFlag += 1;	// Flag mainline that there is new output data.

	}	
	return;
}

