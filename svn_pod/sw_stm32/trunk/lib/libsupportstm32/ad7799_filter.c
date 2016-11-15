/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : ad7799_filter.c
* Hackerees          : deh
* Date First Issued  : 07/18/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : ad7799 filtering
*******************************************************************************/
/*
ad7799_filter.c implement a three Cascaded Integrator Comb (CIC) filter.  (See
_Digital Signal Processing in Communication Systems_, Marvin E. Frerking, (1994; Kluwer Academic Publishing),
page 200, Figure 5.33 for a two stage CIC filter).  

This filter is length 64 with down-sampling by 32.  The AD7799 is polled at 32768Hz divided by 16
(the TR_CLK rate), which is 2048 Hz.  The output of filter is therefore at 2048/32 -> 64 Hz.

With a down-sampling of 32 (5 bits), each integrator requires 5 more bits than the largest
data.  Three cascaded integrators add 15 bits to the 24 bit AD7799 data register size.  This
mandates 'long long' (64 bit) arithmetic.

The scale factor of this filter implementation is 2^18.  The scale factor is (R*N)^M, where R is the down-sampling
ratio (32 in this case), N is the number of delays in the differentiators (2 in this case) and
M is the number of cascaded sections (3 in this case); hence (32*2)^3 = 2^18. (See Frerking
p 202.)  If N is not a power of 2 then the scale factor will not be a power of two and
simple shifting of the output data will not de-scale the filter (if exact de-scaling is
important, which in this application it is not).

Since 4-5 effective bits are added by the low pass filtering, adjustment of the output by 2^18
could drop some useful bits, depending on the noise bits in the input data from AD7799.

It is absolutely essential that the integrators be intialized to zero.  There is nothing
in the math to correct for a non-zero start up, and the error accumulates.

The three cascaded sections give an impulse response that is parabolic and therefore quite
close to Gaussian, which minimizes ringing.

*/
/*
Subroutine call references shown as "@n"--
@1 = svn_pod/sw_stm32/trunk/lib/libsupport/gps_packetize.h
@2 = svn_pod/sw_stm32/trunk/devices/spi1ad7799.c
@3 = svn_pod/sw_stm32/trunk/devices/ad7799_comm.h

*/


#include "ad7799_comm.h"
#include "libopenstm32/gpio.h"

#define AD7799DISCARDNUMBER	6	// Number of initial ad7799 readings to discard

/* The first few ad7799 readings are radically large, so discard them when starting out */
static int nAD7799_Startup_discard = AD7799DISCARDNUMBER; // Discard initial ad7799 readings to avoid a filter impulse


/* Address this routine will go to upon completion */
void 	(*ad7799_filterdone_ptr)(void);		// Address of function to call to go to for further handling under RTC interrupt


/* Variables set by SPI1 operations.  See ../devices/ad7799_comm.c,.h */
extern volatile unsigned char 	ad7799_comm_busy;	// 0 = not busy with a sequence of steps, not zero = busy
extern union INTCHAR		ad7799_24bit;		// 24 bits read (high ord byte is set to zero)
extern volatile int		ad7799_last_good_reading;	// Last good 24 bit reading, bipolar, zero offset adjusted

/* Variables contributed by this masterful set of routines */
static long long 	llIntegral[3];		// Three stages of Z^(-1) integrators
static long long	llDiff[3][2];		// Three stages of Z^(-2) delay storage
long long	llAD7799_out;		// Filtered/decimated data output
unsigned short	usAD7799filterFlag;	// Filtered/decimated data ready: 0 = not ready, not zero = ready


static unsigned short	usDecimateCt;		// Downsampling counter
#define AD7799DECIMATE	32	

/******************************************************************************
 * void ad7799_filter (int nInput);
 * @brief	: Do three sinc filters
 * @param	: nInput = ad7799 reading, bipolar and adjusted
*******************************************************************************/
void ad7799_filter (int nInput)
{
long long	llX1,llX2;	// Intermediate differentiator value


	/* Three stages of integration */
	llIntegral[0] += nInput;		// Incoming data is 32 bits; add to 64 bits accumulator
	llIntegral[1] += llIntegral[0];		// 1st stage feeds 2nd stage
	llIntegral[2] += llIntegral[1];		// 2nd stage feeds 3rd stage
	
	/* Decimate (down-sample) */
	usDecimateCt += 1;			// Decimation count
	if (usDecimateCt >= AD7799DECIMATE) 	// Time to decimate?
	{ // Here, yes.  Do three stages of differentiation of the down sampled data
		usDecimateCt = 0;		// Reset decimation counter
		llX1	     = llIntegral[2] - llDiff[0][1]; // 3rd stage integral minus 3rd stage value delayed by 2
		llDiff[0][1] = llDiff[0][0];	// Move the samples down one step
		llDiff[0][0] = llIntegral[2];	// Input goes into 1st delay of the two delays

		/* Repeat differentiator for 2nd stage */
		llX2         = llX1 - llDiff[1][1]; // 1st stage diff- value delayed by 3
		llDiff[1][1] = llDiff[1][0];
		llDiff[1][0] = llX1;

		/* Repeat for 3rd stage.  Output is the filtered/decimated output */
		llAD7799_out = llX2  - llDiff[2][1];
		llDiff[2][1] = llDiff[2][0];
		llDiff[2][0] = llX2;

		usAD7799filterFlag += 1;	// Flag mainline that there is new output data.

	}	
	return;
}
/******************************************************************************
 * void ad7799_poll_rdy (void);
 * @brief	: Check /RDY and start an spi read 24 bit register sequence
*******************************************************************************/
/* NOTE: This "poll" is done in the RTC interrupt chain */
void ad7799_poll_rdy (void)
{

	/* Throw out the initial readings so that it doesn't give an impulse to the filter */
	if (nAD7799_Startup_discard > 0) 
	{
		nAD7799_Startup_discard -=1;
		return;
	}

	// Here, start doing the filtering
	/* The input to the filter is the last good reading from the ADC */
	ad7799_filter(ad7799_last_good_reading);	// Do three stage cascade integrator comb filter

 	/* If the ad7799 has data available, *start* the reading (spiad7799.c).  When spi1 has
	read the 24 bit ad7799 data register it will adjust the result for bipolar and zero offset
	and store it in 'ad7799_last_good_reading' as a 32 bit int */

	// Is: some ad7799 communication sequence not in progress? AND
	// the /RDY line low?
	if  ( (ad7799_comm_busy == 0) &&  ( (GPIO_IDR(GPIOA) & (1<<6) ) == 0) )	
	{ // Here, a conversion is ready to be read
		AD7799_RD_DATA_REG;		// *Start* read of 24bit data register (macro in ad7799_comm.h (@3))
	}

	/* Call another routine in the "chain" if the address has been set up */
	if (ad7799_filterdone_ptr != 0)	// Having no address for the following is bad.
		(*ad7799_filterdone_ptr)();	// Go do something (e.g. poll the AD7799)

	return;
}

