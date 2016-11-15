/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : 1_pps_cal_filter.c
* Hackee             : deh
* Date First Issued  : 08/09/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Filter input capture time differences bewteen GPS 1_PPS & RTC 1 sec alarm
*******************************************************************************/
/*
1_pps_cal_filter.c implement a three Cascaded Integrator Comb (CIC) filter.  (See
_Digital Signal Processing in Communication Systems_, Marvin E. Frerking, (1994; Kluwer Academic Publishing),
page 200, Figure 5.33 for a two stage CIC filter).  

The three cascaded sections give an impulse response that is parabolic and therefore quite
close to Gaussian, which minimizes ringing.

This filter is length 32 with down-sampling by 16.  

With a down-sampling of 16 (4 bits), each integrator requires 4 more bits than the largest
data.  Three cascaded integrators add 12 bits to the 12 bit ADC register.  This fits into one 32 bit int.

The scale factor of this filter implementation is 2^15.  The scale factor is (R*N)^M, where R is the down-sampling
ratio (32 in this case), N is the number of delays in the differentiators (2 in this case) and
M is the number of cascaded sections (3 in this case); hence (16*2)^3 = 2^15. (See Frerking
p 202.)  If N is not a power of 2 then the scale factor will not be a power of two and
simple shifting of the output data will not de-scale the filter (if exact de-scaling is
important, which in this application it is not).


Initialization--

Put the 'struct' in static memory so that it is initialized to zeroes at startup.
It is absolutely essential that the integrators be intialized to zero.  There is nothing
in the math to correct for a non-zero start up, and the error accumulates.

Initial readings can be discarded by initializing 'sDiscard' in the 'struct' to the number
of readings to be skipped.

*/



#include "1_pps_cal_filter.h"
#include "libopenstm32/gpio.h"


/******************************************************************************
 * unsigned int pps_cal_filter (struct PPSCALFILT *strX, int nInput);
 * @brief	: Do three sinc filters
 * @param	: strX: filters registers
 * @param	: nInput = reading
 * @return	: Ready flag: 0 = no; 1 = yes
*******************************************************************************/
unsigned int pps_cal_filter (struct PPSCALFILT *strX, int nInput)
{
long	lX1,lX2;	// Intermediate differentiator value

	/* Skip the initial data...likely bogus */
	if (strX->sDiscard > 0)
	{
		strX->sDiscard -= 1;	return 0;
	}

	/* Three stages of integration */
	strX->lIntegral[0] += nInput;		// Incoming data is 32 bits; add to 64 bits accumulator
	strX->lIntegral[1] += strX->lIntegral[0];		// 1st stage feeds 2nd stage
	strX->lIntegral[2] += strX->lIntegral[1];		// 2nd stage feeds 3rd stage
	
	/* Decimate (down-sample) */
	strX->usDecimateCt += 1;			// Decimation count
	if (strX->usDecimateCt >= PPSDECIMATE) 	// Time to decimate?
	{ // Here, yes.  Do three stages of differentiation of the down sampled data
		strX->usDecimateCt = 0;		// Reset decimation counter
		lX1	     = strX->lIntegral[2] - strX->lDiff[0][1]; // 3rd stage integral minus 3rd stage value delayed by 2
		strX->lDiff[0][1] = strX->lDiff[0][0];	// Move the samples down one step
		strX->lDiff[0][0] = strX->lIntegral[2];	// Input goes into 1st delay of the two delays

		/* Repeat differentiator for 2nd stage */
		lX2         = lX1 - strX->lDiff[1][1]; // 1st stage diff- value delayed by 3
		strX->lDiff[1][1] = strX->lDiff[1][0];
		strX->lDiff[1][0] = lX1;

		/* Repeat for 3rd stage.  Output is the filtered/decimated output */
		strX->l1PPS_out = lX2  - strX->lDiff[2][1];
		strX->lDiff[2][1] = strX->lDiff[2][0];
		strX->lDiff[2][0] = lX2;

		strX->us1PPSfilterFlag += 1;	// Flag mainline that there is new output data.
		return 1;

	}	
	return 0;
}


