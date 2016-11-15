/******************************************************************************
* File Name          : ad7799_filter_ten2.c
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
#include "ad7799_filter_ten2.h"
#include "ad7799_ten_comm.h"
#include "spi1sad7799_ten.h"
#include "cic_filter_ll_N2_M3.h"
#include "DTW_counter.h"
#include "p1_initialization.h"
#include "tension_idx_v_struct.h"


#define DECIMATESTAGE1	32	// Decimation count for stage 1 filtering
#define DECIMATESTAGE2	32	// Decimation count for stage 2 filtering
#define SCALESTAGE1to2  17	// Number of binary places to scale down stage 1 -> stage2

#define TICKSPERUS	64	// Hard code the timer ticks per microsecond

/* The first few ad7799 readings are radically large, so discard them when starting out */
#define AD7799DISCARDNUMBER	8	// Number of initial ad7799 readings to discard at startup

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
unsigned char aduse2 = 0;		// AD7799 switch: 0 = AD7799_1, 1 = AD7799_2

static uint8_t initflag = 0;	// OTO init structs flag

#define AD7799DECIMATE	32	

unsigned short ad7799_filter_ten2_flagct;
unsigned int ad7799_ten2_flag;	// Flag for flashing LED

static char sw_zero[NADCS];	// 0 = no recalibration request; not zero = start zero calibration sequence
	char x_y[NADCS];		// When busy test result is yes: state for next operation
static char x_n[NADCS];		// When busy test result is no: state for next operation
static uint16_t tinc;		// Increment for next timer

uint32_t ad7799_filter_ten_argh_ct[NADCS]; // Error count of ARGH!!!

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

	for (i = 0; i < NADCS; i++)
	{
		/* State machine */
		x_y[i] = 1;
		x_n[i] = 0;

		/* IIR filtering offsets. */
		ten_f[i].iir_z_recal_w.pprm = &ten_f[i].ten_a.iir_z_recal;	// Pointer to constants	
		ten_f[i].iir_z_recal_w.sw = 0; // Switch to assure initialization
		
		/* Set next zero_recalibration. */
		if (ten_f[i].ten_a.z_recal_ct != 0) // Skip zero recalib for special case of zero
		{ // Here, add the number of conversions to the current conversion count.
			ten_f[i].z_recal_next = ten_f[i].ten_a.z_recal_ct + ten_f[i].readingsct;
		}
	}


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
				p2->Flag2 = 1;
			}
	}
	return;
}

/******************************************************************************
 * void ad7799_poll_rdy_ten2_cic(void);
 * @brief	: Filter with CIC. (Called by tim3_ten2.c)
 ******************************************************************************/
void ad7799_poll_rdy_ten2_cic(void)
{
	/* This is for flashing a LED to aid the hapless Op and puzzled programmer. */
	ad7799_filter_ten2_flagct += 1;
	if (ad7799_filter_ten2_flagct >= 2048)
	{
		ad7799_filter_ten2_flagct = 0;
		ad7799_ten2_flag = 1;
	}

	/* Throw out the initial readings so that it doesn't give gigantic impulse to the filter */
	// Note the discard count is based on timer entry ticks, not readings.
	if (cic[0][0].cic.usDiscard > 0) 
	{
		cic[0][0].cic.usDiscard -=1;
	}
	else
	{
		/* Filter the last good reading for each of the AD7799s */
		filterit(&cic[0][0], ad7799regs[0].lgr);
		if (cic[0][1].Flag2 != 0)
		{
			cic[0][1].Flag2 = 0;
			ten_f[0].cicave = cic[0][1].llout_save;
		}


		filterit(&cic[1][0], ad7799regs[1].lgr);
		if (cic[1][1].Flag2 != 0)
		{
			cic[1][1].Flag2 = 0;
			ten_f[1].cicave = cic[1][1].llout_save;
		}	
	}	
	
	return;
}
/******************************************************************************
 * void ad7799_poll_rdy_ten2_req_calib(uint16_t n);
 * @brief	: Request a re-calibration sequence
 * @param	: n = AD7799 number: 0 = AD7799_1, 1 = AD7799_2
*******************************************************************************/
void ad7799_poll_rdy_ten2_req_calib(uint16_t n)
{
	sw_zero[n & 0x3] = 1;
	return;
}
/******************************************************************************
 * int ad7799_poll_rdy_ten2_req_calib_busy(uint16_t n);
 * @brief	: Request a re-calibration sequence
 * @param	: n = AD7799 number: 0 = AD7799_1, 1 = AD7799_2
 * @param	: 0 = not busy; 1 = sequence is in progress;
*******************************************************************************/
int ad7799_poll_rdy_ten2_req_calib_busy(uint16_t n)
{
	return sw_zero[n & 0x3];
}
/******************************************************************************
 * static void filterandstore(void);
 * @brief	: Filter ad7799 data and store in various places and even more
*******************************************************************************/
static void filterandstore(void)
{
	int32_t tmp;

	/* Adjust data register for bipolar. */
	tmp = 0x800000 - ad7799regs[aduse2].data_reg;	// Bipolar offset

	/* Run reading through each iir filter for this AD7799 */
	iir_filter_l_do(&ten_f[aduse2].iir_filtered[0], (int32_t*)&tmp);
	iir_filter_l_do(&ten_f[aduse2].iir_filtered[1], (int32_t*)&tmp);

	/* Save last good reading for others to use. */
	ten_f[aduse2].lgr = tmp;
	ad7799regs[aduse2].lgr = tmp;

	/* Running count of readings for zero_recalibration, debugging and amazement. */
	ten_f[aduse2].readingsct += 1;
	return;
}
/******************************************************************************
 * uint16_t ad7799_poll_rdy_ten2_both(void);
 * @brief	: Check /RDY and start an spi read 24 bit register sequence
*******************************************************************************/
unsigned int debug9;
//unsigned int debug0;
//unsigned int debug1;

int16_t state;
int32_t tmp;

/* NOTE: This "poll" is done in the 'tim3_ten.c' interrupt chain */
uint16_t ad7799_poll_rdy_ten2_both(void)
{
	/* OTO init the structs before using */
	if (initflag == 0)
	{
		initflag = 1;
		initit();
	}

	// Busy?  Is some ad7799 communication sequence not in progress AND the /RDY line low?
	if  ( (ad7799_ten_comm_busy == 0) &&  ( (GPIO_IDR(GPIOA) & (1<<6) ) == 0) )	
	{ // Here, yes.  It is not busy. "y" = yes it is ready to do next operation
		state = x_y[aduse2]; // Select operation for this AD7799 is ready
	}
	else
	{ // Here, yes it is busy. "n" = no it is not ready
		state = x_n[aduse2];
	
	}
	/* Go to next operation for "this" (aduse2) AD7799. */
	switch (state)
	{
	case 10:
		tinc = 5 * TICKSPERUS;
		break;

	case 0:	// Merely and merrily switch to other AD7799
		/* Switch the select to the other AD7799. */
		if (AD7799_num == 1)
		{ // Here, just one AD7799 successfully initialized ('p1_initialization.c')
			tinc = 5 * TICKSPERUS;	// Time delay for /CS to take effect
			break;
		}
		/* Here, we have two AD7799s, so switch back and forth between them. */
		if (aduse2 == 0)
		{ // AD7799_1 currently enabled
			aduse2 = 1;		// Show AD7799_2 about to be enabled
			ad7799_2_select();	// Select/enable AD7799_2
		}
		else
		{ // AD7799_2 currently enabled
			aduse2 = 0;		// Show AD7799_1 about to be enabled
			ad7799_1_select();	// Select/enable AD7799_1
		}
		tinc = 20 * TICKSPERUS;	// Time delay for /CS to take effect
		break;

	case 1: // Conversion complete.  Start 24 bit data register read or recal
		ad7799_ten_rd_data_reg(aduse2);	// Start data register read
		/* Is there a request to do a zero calibration? */
		if ( ((int)(ten_f[aduse2].readingsct - ten_f[aduse2].z_recal_next) >= 0) \
			&& (ten_f[aduse2].ten_a.z_recal_ct != 0) )
		{ // Here, yes.  Send command to stop continuous read mode.
			x_y[aduse2] = 25;	// Next if it is not busy
			x_n[aduse2] = 0;	// Go switch if busy (?...why should it be busy)
			tinc = 80 * TICKSPERUS;
			break;
		}
		x_y[aduse2] = 2;
		x_n[aduse2] = 0;
		tinc = 50 * TICKSPERUS;
		break;

	case 2:	// Data register read complete.  Save & filter the data register.
		filterandstore();	// Filter and store data
		x_y[aduse2] = 1; // Next time not busy, start data register read
		x_n[aduse2] = 0; // Next time busy, switch to the other AD7799
		tinc = 5 * TICKSPERUS;
		break;

	case 25: // 
		filterandstore();	// Filter and store data
		ad7799_ten_exit_continous_rd (aduse2); // Send stop continuous conversions
		x_y[aduse2] = 3;	// Next if it is not busy
		x_n[aduse2] = 0;	// Go switch if busy (?...why should it be busy)
		tinc = 50 * TICKSPERUS;
		break;

	case 3: // Stop continuous read mode sending is complete.  Start zero calib
		/* Set mode to execute the zero calibration */
		ad7799_ten_wr_mode_reg( (AD7799_ZEROCALIB | AD7799_470SPS) );
		x_y[aduse2] = 4;	// Next it is not busy
		x_n[aduse2] = 0;	
		tinc = 50 * TICKSPERUS;
		break;

	case 4: // End of zero calibration.  Read reg (just to check)
		/* Read (new) offset (for monitoring the change). */
		ad7799_ten_rd_offset_reg(aduse2); // Send command to read offset register
		x_y[aduse2] = 5;
		x_n[aduse2] = 0;
		tinc = 50 * TICKSPERUS;
		break;

	case 5: // End of read offset reg.
		tmp = ad7799regs[aduse2].offset_reg;	// Get offset register reading
		ten_f[aduse2].offset_reg = tmp; 	// Save offset register
		tmp -= 0x800000;			// Offset adjustment
		iir_filter_l_do(&ten_f[aduse2].iir_z_recal_w, &tmp); // IIR filter offset
		tmp = ten_f[aduse2].iir_z_recal_w.z / (ten_f[aduse2].iir_z_recal_w.pprm->scale);// Scale
		tmp += 0x800000;			// Offset re-adjustment
		ten_f[aduse2].offset_reg_filt = tmp;	// Save filtered offset
		ad7799_ten_wr_offset_reg (tmp);		// Store filtered value in register
		ten_f[aduse2].offset_ct	+= 1;		// Running ct of updates
		x_y[aduse2] = 51;
		x_n[aduse2] = 10;
		tinc = 8 * TICKSPERUS;
		break;	

	case 51: // End of zero calibration.  Read reg (just to check that write worked OK)
		/* Read (new) offset (for monitoring the change). */
		ad7799_ten_rd_offset_reg(aduse2); // Send command to read offset register
		x_y[aduse2] = 52;
		x_n[aduse2] = 0;
		tinc = 50 * TICKSPERUS;
		break;

	case 52: // End of read offset reg.
		tmp = ad7799regs[aduse2].offset_reg;
		ten_f[aduse2].offset_reg_rdbk = tmp; 	// Save offset register

	case 55:
		ad7799_ten_wr_mode_reg(AD7799_CONT | AD7799_470SPS); // Start write mode
		x_y[aduse2] = 6;
		x_n[aduse2] =10;
		tinc = 6 * TICKSPERUS;

	case 6: 
		/* Place back in continuous mode. */
		ad7799_ten_set_continous_rd (aduse2);
		x_y[aduse2] = 7;
		x_n[aduse2] = 10;
		tinc = 8 * TICKSPERUS;
		break;
	
	case 7:	// 
debug9 += 1;
//if (aduse2 == 0) debug1 = DTWTIME;
		// Set next conversion count when a zero recalibration is again started.
		ten_f[aduse2].z_recal_next = ten_f[aduse2].ten_a.z_recal_ct + ten_f[aduse2].readingsct;
		ten_f[aduse2].zero_flag = 1;	// Show zero'ing complete
		ad7799_ten_rd_data_reg(aduse2);	// Start data register read
		x_y[aduse2] = 1;
		x_n[aduse2] = 0;
		sw_zero[aduse2] = 0;	// Set zero calibration request switch off
//		if (sw_zero[(aduse2 ^0x1)] == 0) // Is the other AD7799 the process of doing a re-calib?
			tinc = 125 * TICKSPERUS;	// Slow if both are not in zero-calib process
//		else	
//			tinc = 20 * TICKSPERUS;	// Fast so the other guy gets done quickly
		break;
					
	default: // ARGH!!!
		ad7799_filter_ten_argh_ct[aduse2] += 1; // Error count of ARGH!!!	
		break;
	}
	
	return tinc;
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

