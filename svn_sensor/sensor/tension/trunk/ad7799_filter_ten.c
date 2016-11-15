/******************************************************************************
* File Name          : ad7799_filter_ten.c
* Date First Issued  : 07/03/2015
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
@3 = svn_pod/sw_stm32/trunk/devices/ad7799_tencomm.h

*/

#include <stdint.h>
#include "tension_a_functionS.h"
#include "libopenstm32/gpio.h"
#include "ad7799_filter_ten.h"
#include "ad7799_ten_comm.h"
#include "spi1sad7799_ten.h"
#include "cic_filter_ll_N2_M3.h"

#define DECIMATESTAGE1	32	// Decimation count for stage 1 filtering
#define DECIMATESTAGE2	32	// Decimation count for stage 2 filtering
#define SCALESTAGE1to2  17	// Number of binary places to scale down stage 1 -> stage2

/* The first few ad7799 readings are radically large, so discard them when starting out */
#define AD7799DISCARDNUMBER	8	// Number of initial ad7799 readings to discard

/* Routine declarations and other annoucements that compilers enjoy. */
static void cic_init(struct CICLLN2M3 *pcic, unsigned short usDec);

/* Address this routine will go to upon completion */
void 	(*ad7799_ten_filterdone_ptr)(void);		// Address of function to call to go to for further handling under RTC interrupt

/* Variables set by SPI1 operations.  See ../devices/ad7799_tencomm.c,.h */
extern volatile unsigned char 	ad7799_ten_comm_busy;	// 0 = not busy with a sequence of steps, not zero = busy
extern union INTCHAR		ad7799_ten_24bit;	// 24 bits read (high ord byte is set to zero)

/* Variables contributed by this masterful set of routines */
static struct CICLLN2M3 cic1;		// Stage1: Filtering for control loop (fast)
long long	llAD7799_ten_out;	// Filtered/decimated data output
unsigned short	usAD7799_tenfilterFlag;	//Filtered/decimated data ready: 0 = not ready

static struct CICLLN2M3 cic2;		//  Stage2: Filtering of stage1 for printf displaying (slow)
long long	llAD7799_ten_out2;	// Filtered/decimated data output
unsigned short	usAD7799_tenfilterFlag2;// Filtered/decimated data ready: 0 = not ready

/* Filtering for two AD7799s, with two stages of CIC */
struct CICLL cic[NADCS][NSTAGES];
unsigned char aduse = 0;		// AD7799 switch: 0 = AD7799_1, 1 = AD7799_2

static uint8_t initflag = 0;	// OTO init structs flag

#define AD7799DECIMATE	32	

static unsigned short flagct;
int ad7799_ten_flag;	// Flag for flashing LED

static unsigned char readsw = 0;	// 1 = prior AD7799 check started a read of the conversion

/******************************************************************************
 * static void initit(void);
 * @brief	: Get the whole CIC struct mess init'd
*******************************************************************************/
static void initit(void)
{
	int i,j;
	for (i = 0; i < NADCS; i++)
	{
		for (j = 0; j < NSTAGES; j++)
		{
			cic_init(&cic[i][j].cic, DECIMATESTAGE1);
		}
	}
	cic_init(&cic1, DECIMATESTAGE1);
	cic_init(&cic2, DECIMATESTAGE2);
	return;
}
/******************************************************************************
 * static void filterit(struct CICLL *p, int reading);
 * @param	: p = pointer to first stage filtering struct
 * @param	: reading = AD7799 reading
 * @brief	: Run the data through the CIC filtering
*******************************************************************************/
static void filterit(struct CICLL *p, int reading)
{
	unsigned short usret;
	struct CICLL *p2;

	/* The input to the filter is the last good reading from the ADC */
	p->cic.nIn = reading;
	usret = cic_filter_ll_N2_M3 (&p->cic);	// Do three stage cascade integrator comb filter
	if (usret != 0)	// Did the first stage complete?
	{ // Yes, stage 1 completion	
			p->llout_save = p->cic.llout; // Save for somebody
			p->Flag += 1;	// JIC show somebody there's new data

			// Filter again (used for printf/display purposes)
			p2 = (p+1);
			p2->cic.nIn = (p->cic.llout / (1<<SCALESTAGE1to2));	
			usret = cic_filter_ll_N2_M3 (&(p2->cic));
			if (usret != 0)	// Stage 2 completion?
			{ // Yes.
				p2->llout_save = p2->cic.llout; // Save
				p2->Flag  += 1; // New data for showing
			}
	}
	return;
}
/******************************************************************************
 * void ad7799_poll_rdy_ten_both(void);
 * @brief	: Check /RDY and start an spi read 24 bit register sequence
*******************************************************************************/
int debugad7799;

void ad7799_poll_rdy_ten_both(void)
{
//	unsigned short usret;
	/* OTO init the structs before using */
	if (initflag == 0)
	{
		initflag = 1;
		initit();
	}
	
	/* This is for flashing a LED for the hapless Op (and puzzled programmer) */
	flagct += 1;
	if (flagct >= 2048)
	{
		flagct = 0;
		ad7799_ten_flag = 0;
	}	

	// Is some ad7799 communication sequence not in progress AND the /RDY line low?
	if  ( (ad7799_ten_comm_busy == 0) &&  ( (GPIO_IDR(GPIOA) & (1<<6) ) == 0) )	
	{ // Here, a conversion is ready to be read
		ad7799_ten_rd_24bit_reg_both (aduse);	// *Start* read of 24bit data register (macro in ad7799_tencomm.h (@3))
		readsw = 1;
	}
	else
	{ // No conversion ready for this AD7799.  Check the other AD7799.

		if (readsw != 0) // Did prior pass start a conversion register read?
		{ // Yes.  There is a new reading for this AD7799/
			readsw = 0;	// Reset switch
			/* Run reading through each iir filter for this AD7799 */
			iir_filter_l_do(&ten_f[aduse].iir_filtered[0], (int32_t*)&ad7799_ten_last_good_reading_both[aduse]);
			iir_filter_l_do(&ten_f[aduse].iir_filtered[1], (int32_t*)&ad7799_ten_last_good_reading_both[aduse]);
			
			/* Save last good reading along with other readings & parameters for the function. */
			ten_f[aduse].lgr = ad7799_ten_last_good_reading_both[aduse];
		}

		/* Switch the select to the other AD7799. */
		if (aduse == 0)
		{ // AD7799_1 currently enabled
			aduse = 1;		// Show AD7799_2 about to be enabled
			ad7799_2_select();	// Select/enable AD7799_2
		}
		else
		{ // AD7799_2 currently enabled
			aduse = 0;		// Show AD7799_1 about to be enabled
			ad7799_1_select();	// Select/enable AD7799_1
		}
	}		

	/* Throw out the initial readings so that it doesn't give gigantic impulse to the filter */
	// Note the discard count is based on timer entry ticks, not readings.
	if (cic[0][0].cic.usDiscard > 0) 
	{
		cic[0][0].cic.usDiscard -=1;
		return;
	}

	/* Filter the last good reading for each of the AD7799s */
	filterit(&cic[0][0], ad7799_ten_last_good_reading_both[0]);	
	filterit(&cic[1][0], ad7799_ten_last_good_reading_both[1]);	

	return;
}
/******************************************************************************
 * void ad7799_poll_rdy_ten(void);
 * @brief	: Check /RDY and start an spi read 24 bit register sequence
*******************************************************************************/
unsigned int debug9;

/* NOTE: This "poll" is done in the 'tim3_ten.c' interrupt chain */
void ad7799_poll_rdy_ten(void)
{
	unsigned short usret;
	/* OTO init the structs before using */
	if (initflag == 0)
	{
		initflag = 1;
		initit();
	}
	
	/* This is for flashing a LED to aid the hapless Op and puzzled programmer. */
	flagct += 1;
	if (flagct >= 2048)
	{
		flagct = 0;
		ad7799_ten_flag = 0;
	}

	/* Throw out the initial readings so that it doesn't give gigantic impulse to the filter */
	if (cic1.usDiscard > 0) 
	{
		cic1.usDiscard -=1;
		return;
	}

	// Here, start doing the filtering
	/* The input to the filter is the last good reading from the ADC */
	cic1.nIn = ad7799_ten_last_good_reading;
	usret = cic_filter_ll_N2_M3 (&cic1);	// Do three stage cascade integrator comb filter
	if (usret != 0)
	{ // stage 1 completion	
			llAD7799_ten_out = cic1.llout;	// Save for mainline
			usAD7799_tenfilterFlag += 1;	// Show somebody there's new data

			// Filter again (used for printf/display purposes
			cic2.nIn = (cic1.llout / (1<<SCALESTAGE1to2));	
			usret = cic_filter_ll_N2_M3 (&cic2);
			if (usret != 0)
			{
				llAD7799_ten_out2 = cic2.llout;	// Save
				usAD7799_tenfilterFlag2 += 1; // New data for showing
			}
	}

 	/* If the ad7799 has data available, *start* the reading (spiad7799.c).  When spi1 has
	read the 24 bit ad7799 data register it will adjust the result for bipolar and zero offset
	and store it in 'ad7799_tenlast_good_reading' as a 32 bit int */

	// Is some ad7799 communication sequence not in progress AND the /RDY line low?
	if  ( (ad7799_ten_comm_busy == 0) &&  ( (GPIO_IDR(GPIOA) & (1<<6) ) == 0) )	
	{ // Here, a conversion is ready to be read
		AD7799_RD_DATA_REG;		// *Start* read of 24bit data register (macro in ad7799_tencomm.h (@3))
	}

	/* Call another routine in the "chain" if the address has been set up */
	if (ad7799_ten_filterdone_ptr != 0)	// Skip subroutine call if pointer is NULL
		(*ad7799_ten_filterdone_ptr)();	// Go do something

	return;
}
/******************************************************************************
 * static void cic_init(struct CICLLN2M3 *pcic, unsigned short usDec);
 * @brief 	: Clear values
 * @param	: pcic = pointer to struct with intermediate values
 * @param	: usDec = decimation number
*******************************************************************************/
/*  FYI
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
*/
static void cic_init(struct CICLLN2M3 *pcic, unsigned short usDec)
{
	int j;

	pcic->usDecimateNum = usDec;	// Decimation number
	pcic->usDiscard = AD7799DISCARDNUMBER;	// Initial discard count
	pcic->usDecimateCt = 0;		// Decimation counter
	pcic->usFlag = 0;		// Data ready flag

	for (j = 0; j < 3; j++)
	{ // Very important that the integrators begin with zero.
		pcic->llIntegral[j] = 0;
		pcic->llDiff[j][0] = 0;
		pcic->llDiff[j][1] = 0;
	}	
	return;	
}

