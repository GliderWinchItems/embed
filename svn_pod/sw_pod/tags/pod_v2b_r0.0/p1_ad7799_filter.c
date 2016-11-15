/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : ad7799_filter.c
* Hackerees          : deh
* Date First Issued  : 07/18/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : ad7799 filtering
*******************************************************************************/
/*
06/07/2012 deh
 Added check for out of range load-cell readings.  A shutdown and re-initialization is
 instituted of OOR readings persist.

*/

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
#include "p1_ad7799_filter.h"
#include "p1_common.h"		// Variables in common

#define AD7799DISCARDNUMBER	6	// Number of initial ad7799 readings to discard

static void ad7799_lockup (void);	//  Execute a sequence to shutdown and re-initialize the ad7799

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
unsigned char cADClockupflag;		// not zero = ad7799 re-initialization in progress

static unsigned int uiADCoorctr;	// Count of consecutive OOR readings
static unsigned short	usDecimateCt;		// Downsampling counter
#define AD7799DECIMATE	32		// Downsampling (decimation) count

/******************************************************************************
 * void ad7799_filter (int nInput);
 * @brief	: Do three sinc filters
 * @param	: nInput = ad7799 reading, bipolar and adjusted
*******************************************************************************/
static short usPhaseFlag;	// Flag for phasing filter to system time 

void ad7799_filter (int nInput)
{
long long	llX1,llX2;	// Intermediate differentiator value

  /* Synchronize filtering start of filtering on the even 1/64th sec tick */
  switch (usPhaseFlag)
  {
  case 0: /* Be sure filter starts on the even 1/64th sec tick */
     /* Look at low order bits of tick count. Skip filtering if not in sync. */
     if ( (strAlltime.SYS.ui[0] & 31)  != 1 )	break;
     usPhaseFlag = 1; 	
  case 1:

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
		usPhaseFlag = 0;			// Reset synchronizing flag
	}	
  
  } // End of switch (usPhaseFlag)
	return;
}
/******************************************************************************
 * void ad7799_poll_rdy (void);
 * @brief	: Check /RDY and start an spi read 24 bit register sequence
*******************************************************************************/
static void ad7799_lockup (void);	//  Execute a sequence to shutdown and re-initialize the ad7799

unsigned short	ad7799lockupctr;	// Allow for multiple timeout intervals
unsigned short	ad7799lockupseq;	// Next in sequence


/* NOTE: This "poll" is done in the RTC interrupt chain */
void ad7799_poll_rdy (void)
{

	/* Throw out the initial readings so that it doesn't give an impulse to the filter */
	if (nAD7799_Startup_discard > 0) 
	{
		nAD7799_Startup_discard -=1;
		return;
	}

	/* Skip doing anything with the AD7799 data if we are in a lockup condition */
	if (cADClockupflag == 0)
	{ // Here, the ad7799 shutdown & re-initialization process is not in progress

		/* Check if the reading is off-scale, either plus or minus.  If so, a transient or ESD may have locked
		   it up, in which case we will stop the polling of the AD7799, shut it down, and re-initialize it. */
		if ( (ad7799_last_good_reading > AD7799OORANGEHI) || (ad7799_last_good_reading < AD7799OORANGELO) )
		{ // Here, reading is too large to be reasonable, so we assume it has locked up
			/* Remember we are coming through this routine 2048 times per sec.  (The CIC filter puts up a flag 64 times/sec.) */
			uiADCoorctr += 1;	// Count number of consecutive bad readings
			if (uiADCoorctr >= AD7799OORCT)	// Too many consecutive OOR readings?
			{ // Here, yes.  Get the shutdown sequence started
				cADClockupflag = 1;	// Use this flag to stop the presses we are going to re-initialize
				ad7799lockupseq = 0;// Reset lockup state machine

				/* Set 'last_good_reading so that other routines that use it use this unique value */
				ad7799_last_good_reading = AD7799REINITVALUE;	// Show a value that identifies the lockup
			}
			else
			{ // Here, a OOR value, but the threshold for consecutive readings has not been passed
				/* The input to the filter is the last good reading from the ADC */
				ad7799_filter(ad7799_last_good_reading);	// Do three stage cascade integrator comb filter
	
				// Is: some ad7799 communication sequence not in progress? AND
				// the /RDY line low?
				if  ( (ad7799_comm_busy == 0) &&  ( (GPIO_IDR(GPIOA) & (1<<6) ) == 0) )	
				{ // Here, a conversion is ready to be read
					AD7799_RD_DATA_REG;		// *Start* read of 24bit data register (macro in ad7799_comm.h (@3))	
				}
			}
		}
		else
		{ // Here, data is reasonable start doing the filtering
			uiADCoorctr = 0;	// Rest count number of consecutive bad readings
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
		}
	}
	else
	{ // Here, the ad7799 shutdown & re-initialization process is in progress
		/* The input to the filter is the last good reading from the ADC */
		ad7799_filter(ad7799_last_good_reading);	// Do three stage cascade integrator comb filter
		ad7799_lockup();	// Poll the shutdown & re-initialization sequence
	}

	/* Call another routine in the "chain" if the address has been set up */
	if (ad7799_filterdone_ptr != 0)	// Having no address for the following is bad.
		(*ad7799_filterdone_ptr)();	// Go do something (e.g. poll the AD7799)

	return;
}
/******************************************************************************
 * static void ad7799_lockup (void);
 * @brief	: Execute a sequence to shutdown and re-initialize the ad7799
*******************************************************************************/
static void ad7799_lockup (void)
{
	switch (ad7799lockupseq)
	{
	case 0:	// Begin sequence
		/* Turn off power both power supplies to the ad7799 */
		STRAINGAUGEPWR_off;	// 5.0v strain gauge regulator (@1)
		ADC7799VCCSW_off;	// 3.3v digital switch (@1)
		/* The following time needs to elapse to assure the power supplies capacitors have discharged. */
		sys.t1 = SYSTICK_getcount32() - (POWERDELAY*(sysclk_freq/10000));	// Get time and compute ending time  (@3)		
		ad7799lockupctr = 0;	// Allow for multiple timeout intervals
		ad7799lockupseq = 1;	// Next in sequence

	/* The following implements the routine 'void complete_ad7799(void)' in 'p1_initialization.c'
	   but overlaps all the waits (instead of looping).  Remember this is being done under interrupt so we
	   cannot tarry waiting for things to complete. */

	case 1: // Waiting for capacitors on power supplies to discharge

		/* Check if the time has expired */
		if ( ( (int)(SYSTICK_getcount32() - sys.t1) ) > 0 ) break; // break if time has not expired
		/* The following provides for long timer intervals (if needed) */
		ad7799lockupctr += 1;	// Count number of timer intervals
		if (ad7799lockupctr >= AD7799SHUTDNCT) // Have we done enough intervals?
		{ // Here no.  Start another one
			sys.t1 = SYSTICK_getcount32() - (POWERDELAY*(sysclk_freq/10000));	// Set end time for interval  (@3)		
			break;
		}

		/* Turn on supplies */
		STRAINGAUGEPWR_on;	// 5.0v strain gauge regulator (@1)
		ADC7799VCCSW_on;	// 3.3v digital switch (@1)

		/* The following time needs to elapse before the adc and ad7799 power is stablized (about 500 ms) */
		sys.t1 = SYSTICK_getcount32() - (POWERUPDELAYR*(sysclk_freq/10000));	// Get time for this mess  (@3)
		ad7799lockupseq = 2;	// Next in sequence

	case 2:	// Waiting for power up time to expire
		if ( ( (int)(SYSTICK_getcount32() - sys.t1) ) > 0 ) break;	// Wait for regulators to stabilize

		ad7799_reset ();	// Send a reset sequence to the AD7799
		ad7799lockupseq = 3;	// Next in sequence		
		
	case 3: // Wait for reset to complete, then start ID reg read 
		if (ad7799_comm_busy != 0) break;	// Wait for write/read sequence to complete

		AD7799_RD_ID_REG;	// Read ID register (macro in ad7799_comm.h)
		ad7799lockupseq = 4;	// Next in sequence		

	case 4: // Wait for ID register read to complete, the start 'initA'
		if (ad7799_comm_busy != 0) break;		// Wait for mode register loading to complete
		ad7799lockupseq = 41;	// Next in sequence				

		/* Select channel with strain gauge, gain 128x, reference voltage detector on */
		//........................... channel........gain......reference detect...bipolar(default)
		ad7799_wr_configuration_reg (AD7799_CH_2 | AD7799_128 |AD7799_REF_DET );

	case 41:
		if (ad7799_comm_busy != 0) break;		// Wait for mode register loading to complete
		ad7799lockupseq = 42;	// Next in sequence		

		/* Set mode to execute the zero calibration */
		ad7799_wr_mode_reg(AD7799_ZEROCALIB | (AD7799_4p17SPS & 0x0f) );	// Calibrate chan with inputs internally shorted
		break;

	case 42:
		if (ad7799_comm_busy != 0) break;		// Wait for mode register loading to complete
		ad7799lockupseq = 43;	// Next in sequence		

		/* Set mode to continuous conversions */
		ad7799_wr_mode_reg(AD7799_SINGLE | (AD7799_4p17SPS & 0x0f) );
		break;

	case 43:
		if (ad7799_comm_busy != 0) break;	// Wait for write/read sequence to complete
		ad7799lockupseq = 5;	// Next in sequence		

		/* Setup: Zero calibration, sampling rate, and continuous conversions */
		ad7799_1_initA(AD7799_4p17SPS);	// Setup for strain gauge, (Arg = sampling rate)
		break;

	case 5:
		if (ad7799_comm_busy != 0) break;	// Wait for write/read sequence to complete
		ad7799lockupseq = 6;	// Next in sequence		

		AD7799_RD_CONFIGURATION_REG;	// Read configuration register (macro in ad7799_comm.h)

	case 6:
		if (ad7799_comm_busy != 0) break;	// Wait for write/read sequence to complete
		ad7799lockupseq = 7;	// Next in sequence		
		
		AD7799_RD_MODE_REG;		// Read mode register (macro in ad7799_comm.h)

	case 7:
		if (ad7799_comm_busy != 0) break;	// Wait for write/read sequence to complete
		ad7799lockupseq = 8;	// Next in sequence		

		AD7799_RD_OFFSET_REG;		// Read offset register (macro in ad7799_comm.h)
	case 8:
		if (ad7799_comm_busy != 0) break;	// Wait for write/read sequence to complete
		ad7799lockupseq = 9;	// Next in sequence		

		AD7799_RD_FULLSCALE_REG;	// Read full scale register (macro in ad7799_comm.h)

	case 9:
		if (ad7799_comm_busy != 0) break;	// Wait for write/read sequence to complete

		ad7799_wr_mode_reg(AD7799_CONT | AD7799_470SPS );	// Continuous conversion (see ad7799_comm.h)
		
		ad7799lockupseq = 0;	// Reset for the next big event	
		cADClockupflag = 0;	// Show that the re-initialization is completed
		uiADCoorctr = 0;	// Rest count number of consecutive bad readings

	}
	return;
}
