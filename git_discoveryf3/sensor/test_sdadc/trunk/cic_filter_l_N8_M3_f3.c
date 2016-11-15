/******************************************************************************
* File Name          : cic_filter_l_N8_M3_f3.c
* Date First Issued  : 03/27/2016
* Board              : STM32F373 filtering
* Description        : General purpose cic long filter for N(delays)=8, M(sections)=3
*******************************************************************************/
/*
Implement a three Cascaded Integrator Comb (CIC) filter.  (See _Digital Signal Processing in Communication Systems_,
 Marvin E. Frerking, (1994; Kluwer Academic Publishing), page 200, Figure 5.33 for a two stage CIC filter).   

This filter length is R*N, where R is the down-sampling as set in the struct and N = 2.

The number of bits required in each integrator requires three times (M=3) the number of bits
in the downsampling.  E.g. with a down-sampling of 32 (5 bits), each integrator requires 5 more bits 
than the largest data.  Three cascaded integrators add 15 bits.  If the largest input size is 24 bits
(e.g. the AD7799 register), 39 bits are required, therefore 'long long' (64 bit) arithmetic is needed.
  
The scale factor is (R*N)^M, where R is the down-sampling ratio (32 in this case), N is the number of delays 
in the differentiators (2 in this case) and M is the number of cascaded sections (3 in this case); 
hence (32*2)^3 = 2^18. (See Frerking p 202.)  If N is not a power of 2 then the scale factor will not be a 
power of two and simple shifting of the output data will not de-scale the filter (if exact de-scaling is
important, which in this application it is not).

Since effective bits are added by the low pass filtering, adjustment of the output by
could drop some useful bits.

It is absolutely essential that the integrators be intialized to zero.  There is nothing
in the math to correct for a non-zero start up, and the error accumulates.

The three cascaded sections give an impulse response that is parabolic and therefore quite
close to Gaussian, which minimizes ringing.

NOTE: 

1) The struct must be initialized with the downsampling count, and discard number (if any
initial readings are to be discarded).

2) The new reading to go into the filter is added to the struct before calling the routine,

3) The 'usFlag' is incremented when the filter has a new output.

*/
/*

*/


#include "cic_filter_l_N8_M3_f3.h"

/******************************************************************************
 * void cic_filter_l_N8_M3_f3_init (struct CICLN8M3 *pcic, unsigned short decimate);
 * @brief	: Setup the the struct with initial values
 * @param	: pcic = pointer to struct for this data stream
 * @param	: decimate = decimation number ("R")
*******************************************************************************/
void cic_filter_l_N8_M3_f3_init (struct CICLN8M3 *pcic, unsigned short decimate)
{
	int i,j;
	for (i = 0; i < CIC1M; i++)
	{
		pcic->lIntegral[i] = 0;
		for (j = 0; j < CIC1N; j++)
		{
			pcic->lDiff[j][i] = 0;
		}
	}
	pcic->usDecimateCt = 0;
	pcic->usDecimate = decimate;
	pcic->pDiff = &pcic->lDiff[0][0];
	pcic->pDiff_end = &pcic->lDiff[CIC1N][CIC1M];
	return;
}

/******************************************************************************
 * unsigned short cic_filter_l_N8_M3_f3 (struct CICLN8M3 *pcic, long lval);
 * @brief	: Do three sinc filters* File Name  
 * @param       : pcic = pointer to struct with intermediate values and pointers
 * @param	: lval = output value of a new filtered output
 * @return	: 0 = filtered output not ready; 1 = new filtered output ready
*******************************************************************************/
unsigned short cic_filter_l_N8_M3_f3 (struct CICLN8M3 *pcic, long lval)
{
	long	lX1,lX2;	// Temporary intermediate differentiator value
	long *pIntegral = &pcic->lIntegral[0];	// Integrators
	long *pd;	// Local differentiators

	/* Three stages of integration */
	*(pIntegral+0) += lval;		// Incoming data is 32 bits; add to 64 bit accumulator
	*(pIntegral+1) += *(pIntegral+0);	// 1st stage feeds 2nd stage
	*(pIntegral+2) += *(pIntegral+1);	// 2nd stage feeds 3rd stage
	
	/* Decimate (down-sample) */
	pcic->usDecimateCt += 1;		// Decimation count
	if (pcic->usDecimateCt >= pcic->usDecimate) // Time to decimate?
	{ // Here, yes.  Do three stages of differentiation of the down sampled data
		pcic->usDecimateCt = 0;	// Reset decimation counter
		pd = pcic->pDiff;	// Local pointer

		/* Output of 3rd integrator less delayed value. */
		lX1	= *(pcic->lIntegral+2)- *(pd+0);
		*(pd+0) = *(pcic->lIntegral+2);	// Save new value

		/* 2nd stage differentiator */
		lX2	= lX1 - *(pd+1); // Output of stage 1 less delayed value
		*(pd+1) = lX1;		// Save new value

		/* 3rd stage differentiator */
		pcic->lout = lX2 - *(pd+2); // Output of stage 2 less delayed value
		*(pd+2) = lX2;

		/* Advance pointer in delayed value circular buffer. */
		pcic->pDiff += 3;	// Step to now the next oldest set of three
		if (pcic->pDiff >= pcic->pDiff_end) pcic->pDiff = &pcic->lDiff[0][0];

		return 1;	
	}	
	return 0;
}

