/******************************************************************************
* File Name          : ad7799_vcal_filter.c
* Date First Issued  : 10/09/2015
* Board              : STM32F103VxT6_pod_mm
* Description        : ad7799 filtering--multiple channels
*******************************************************************************/
/*
This routine sequences the AD7799 through the channels.  Each channel is measured
in the continuous read mode and the result of each reading summed.  When the count
of readings is reached the next channel is initialized and continous readings
resume.  When all the channels have been read and summed a flag is set for the 
mainline and the readings saved in an array.  The whole cycle then repeats.

During the continuous reading mode, when the reading data is ready the ad7799 
DOUT line goes low and triggers an external interrupt.  The interrupt handling then
masks the external interrupt and executes the series of SPI write/reads to extract the
reading from the ad7799 data register.  When that is complete the external interrupt
mask is removed and the process repeats until the number of readings have been summed.
The sum is saved and a sequence of spi driven routines sets up the next ad7799 input
channel and places it in continuous read mode.  

Once started the cycle is entirely interrupt driven.  'main' monitors the flag for new
data.
*/
#include "PODpinconfig.h"
#include "ad7799_vcal_comm.h"
#include "ad7799_vcal_filter.h"
#include "spi1ad7799_vcal.h"
#include "libopenstm32/gpio.h"
#include "exti.h"

#include "libmiscstm32/DTW_counter.h"

#define NULL	0

/* Subroutine prototypes */
static void sp0(void);
static void sp1(void);
static void spx(void);

/* Pointers point to address upon completion of operation in 'spi1ad7799_vcal' */
extern void (*ad7799_vcal_comm_doneptr)(void);	// Address of function to call upon completion of write
extern void (*spi1_vcal_exti2ptr)(void);	// Low Level interrupt trigger function callback

/* Variables set by SPI1 operations.  See ../devices/ad7799_vcal_comm.c,.h */
extern volatile unsigned char 	ad7799_vcal_comm_busy;		// 0 = not busy with a sequence of steps, not zero = busy
extern union INTCHAR		ad7799_vcal_24bit;		// 24 bits read (high ord byte is set to zero)
extern volatile int		ad7799_vcal_last_good_reading;	// Last good 24 bit reading, bipolar, zero offset adjusted


/* Parameters for each channel. */
static struct AD7799CHAN* pchan; 		// Pointer to array of channel parameters (for interrupt use)
static struct AD7799CHAN* pchan_begin; 		// Pointer to array of channel parameters, beginning
static struct AD7799CHAN* pchan_end; 		// Pointer to array of channel parameters, end + 1

int32_t sum_save[AD7799NUMCHANNELS];

/* Index into channel map. */
static uint16_t idx;

/* When all channels have been read & summed, save in a buffer and set this flag. */
uint16_t ad7799_ready_flag;	// Flag showing new 'sum_save' data ready

/******************************************************************************
 * void ad7799_vcal_filter_init_param(struct AD7799CHAN* p[]);
 * @brief	: Initialize channel parameters
 * @param	: p = pointer to array of parameters for channels
*******************************************************************************/
void ad7799_vcal_filter_init_param(struct AD7799CHAN* p)
{
/* AD7799 usage
AIN1 +/- 10K/3.9K divider: .2798 ratio
AIN2 +/- White / Yellow
AIN3 +/- +2.5 volt ref / ground
*/
	int i;

	pchan = p;	// Save pointer for interrupt use
	pchan_begin = pchan;	// Save beginning
	pchan_end = pchan_begin + AD7799NUMCHANNELS;

	/* Mux channel number */
	(p + 0)->mux = CHANNELAIN_1;
	(p + 1)->mux = CHANNELAIN_2;
	(p + 2)->mux = CHANNELAIN_3;

	/* Mode register setup for continuous conversions. */
	(p + 0)->mode = (AD7799_CONT | AD7799_470SPS);
	(p + 1)->mode = (AD7799_CONT | AD7799_470SPS);
	(p + 2)->mode = (AD7799_CONT | AD7799_470SPS);

	/* Mode register setup for inernal zero calibration. */
	(p + 0)->calibzero = (AD7799_ZEROCALIB | AD7799_470SPS);
	(p + 1)->calibzero = (AD7799_ZEROCALIB | AD7799_470SPS);
	(p + 2)->calibzero = (AD7799_ZEROCALIB | AD7799_470SPS);


	/* Mode register setup for internal full-scale calibration. */
	(p + 0)->calibfs = (AD7799_FSCALIB | AD7799_470SPS);
	(p + 1)->calibfs = (AD7799_FSCALIB | AD7799_470SPS);
	(p + 2)->calibfs = (AD7799_FSCALIB | AD7799_470SPS);


	/* Configuration register setup. */
	(p + 0)->config = (AD7799_UNI|AD7799_1NO|AD7799_BUF_IN|AD7799_REF_DET | (p + 0)->mux );
	(p + 1)->config = (AD7799_UNI|AD7799_1NO|AD7799_BUF_IN|AD7799_REF_DET | (p + 1)->mux );
	(p + 2)->config = (AD7799_UNI|AD7799_1NO|AD7799_BUF_IN|AD7799_REF_DET | (p + 2)->mux );

	/* Count of number of readings to sum. */
	(p + 0)->ct = READSUMCT;
	(p + 1)->ct = READSUMCT;
	(p + 2)->ct = READSUMCT;

	/* Zero things that might need it. */
	for (i = 0; i < AD7799NUMCHANNELS; i++)
	{
		(p + i)->sum = 0;		// Summation of readings
		(p + i)->chanflag = 0; 		// Channel read complete flag
		(p + i)->ctr = 0;		// Working count
	}	
	
	return;
}

/******************************************************************************
 * void ad7799_vcal_filter_start(struct AD7799CHAN* p);
 * @brief	: Start interrupt driven ad7799 readings; sequence though channels
 * @param	: p = pointer to array of parameters for channels
*******************************************************************************/
void ad7799_vcal_filter_start(struct AD7799CHAN* p)
{
	/* Select AD7799_1 */
//	AD7799_1_CS_low		// Macro is in PODpinconfig.h

	/* Iniiialize the array of channel parameters. */
	ad7799_vcal_filter_init_param(p);

	/* Initializations applicable to all channels. */
	idx = 0;		// Channel sequence index

	/* Reset sequence. */
	ad7799_vcal_reset();	// JIC (in case cap held power during resent)

	/* Do calibration and setup for interrupt driven cycle. */ 
	sp0();

	/* At this point a series of routines are driven by spi1 completion and end
	   with the AD7799 in continuous read mode where a high->low on the AD7799 DOUT
	   causes an external interrupt and a subsequent entry into 'exti2_b'
 	   below. The cycle then repeats for another input channel.  */
	
	return;
}
/* ############################################################################
 * spi and exti2 sequence lower level operations
   ############################################################################ */
int dbg = 0;	// (Debug)
uint32_t adt0; 
uint32_t adt1; 
uint32_t adt2; 
uint32_t adt3; 

extern unsigned char ad7799_vcal_8bit_reg;	// Byte with latest status register from ad7799_vcal_1
extern union SHORTCHAR	ad7799_vcal_16bit;	// Result of 16b read
extern union INTCHAR	ad7799_vcal_24bit;	// Result of 24b read

// local save of reads
union SHORTCHAR	ad7799_vcal_16bit_save;		// Save (Debug)
uint16_t	ad7799_vcal_16bit_save2;	// Save (Debug)
uint8_t		ad7799_vcal_8bit_save;
uint8_t ad7799id;				// Save ID (Debug)

// ---------------------------------------------------------------------
static void sp10(void);

static void sp11(void)
{ // Here, data register has been read into 'ad7799_vcal_last_good_reading'
dbg = 11;
	int i;
	/* Fix up result (reverse byte order, extend sign bit if bipolar mode bit on). */
	int32_t rd24b = ad7799_vcal_rd_24bit_polar(pchan->config & AD7799_UNI);

//if (rd24b == 0) while(1==1); // First reading zero trap

	pchan->sum += rd24b;	// Build long long sum of readings
	pchan->ctr += 1;				// Count # readings summed
	if (pchan->ctr >= pchan->ct) // Enough readings summed?
	{ // Yes, we are done with this channel
		pchan->ctr = 0;			// Reset summing counter
		pchan->sum_save = pchan->sum;	// Save channel sum for mainline use
		pchan->sum = 0;			// Reset sum for next time
		pchan->chanflag += 1;		// Show mainline new data ready
		pchan += 1;			// Next channel to setup
		if (pchan >= pchan_end) 	// Wraparound channel sequence
		{ // Here, all channels have been read
			pchan = pchan_begin;	// Start new channel sequence
			for (i = 0; i < AD7799NUMCHANNELS; i++) // Save all for mainline
			{
				sum_save[i] = (pchan + i)->sum_save; // Save all
			}
			ad7799_ready_flag += 1;	// Show all data ready for mainline
		}
		// Exit the continuous read mode and change channel
		spi1ad7799_vcal_push(&spx);	// Push next spi address
		ad7799_vcal_exit_continous_rd();
dbg = 12;
		return;
	}
	/* Here, continue summing readings in continuous mode */
	spi1ad7799_vcal_DOUT_ext(&sp10);	// Enable external interrupt
adt0 = DTWTIME - adt1;
adt2 = DTWTIME - adt1;
adt1 = DTWTIME;
	return;
}
// ---------------------------------------------------------------------
static void sp10(void)
{ // Here--continuous conversion in effect and data reg is ready to be read.
dbg = 10;
	spi1ad7799_vcal_push(&sp11);	// Address for following spi operation
	ad7799_vcal_rd_data_reg();	// Start read of data register
	return;
}
// ---------------------------------------------------------------------
static void sp10a(void)
{ //
dbg = 9;
	spi1ad7799_vcal_DOUT_ext(&sp10);// Enable external interrupt
	return;
}
// ---------------------------------------------------------------------
static void sp10b(void)
{ // Here--continuous conversion in effect and data reg is ready to be read.
dbg = 10;
	spi1ad7799_vcal_push(&sp10a);	// Address for following spi operation
	ad7799_vcal_rd_data_reg();	// Start read of data register
	return;
}
// ---------------------------------------------------------------------
static void sp10c(void)
{ //
dbg = 9;
	spi1ad7799_vcal_DOUT_ext(&sp10b);// Enable external interrupt
	return;
}
// ---------------------------------------------------------------------
static void sp7(void)
{ // 
dbg = 81;
ad7799_vcal_16bit_save2 = ad7799_vcal_rd_16bit_swab(); 	// Save mode reg read (debug)
	spi1ad7799_vcal_push(&sp10c);
	ad7799_vcal_set_continous_rd();	// Start continuous converison mode
	return;
}
// ---------------------------------------------------------------------
static void sp7a(void)
{ //
dbg = 7;
	spi1ad7799_vcal_DOUT_ext(&sp7);	// Enable external interrupt
	return;
}
// ---------------------------------------------------------------------
static void sp6(void)
{ // Readback for debugging
dbg = 8;
	spi1ad7799_vcal_push(&sp7);
	ad7799_vcal_rd_mode_reg(); // Read mode register just written
	return;
}
// ---------------------------------------------------------------------
static void sp6a(void)
{ // Set mode to continuous
dbg = 6;
	spi1ad7799_vcal_push(&sp6);
	ad7799_vcal_wr_mode_reg(pchan->mode);
	return;
}
// ---------------------------------------------------------------------
static void sp5c(void)
{ //
dbg = 7;
	spi1ad7799_vcal_DOUT_ext(&sp6a);	// Enable external interrupt
	return;
}
// ---------------------------------------------------------------------
static void sp5b(void)
{ // Start internal full-scale calibration
dbg = 8;
//	if ((pchan->calibfs &0x0ff0) == 0)
//	{ // Here, do internal fullscale calibration
		spi1ad7799_vcal_push(&sp5c);	
		ad7799_vcal_wr_mode_reg(pchan->calibfs);// Start fullscale calibration
//	}
//	else
//	{ // Skip fullscale calibration 
//		sp5();
//	}
	return;
}
// ---------------------------------------------------------------------
static void sp5a(void)
{ //
dbg = 1;
	spi1ad7799_vcal_DOUT_ext(&sp5b);	// Enable external interrupt

	return;
}
// ---------------------------------------------------------------------
static void sp3(void)
{ // Start internal zero calibration
dbg = 5;
// Save last 16b and last 8b reads for mainline display
ad7799_vcal_rd_16bit_swab();			// Bytes arrive in reverse order
ad7799_vcal_16bit_save = ad7799_vcal_16bit; 	// Save config reg read (debug)
ad7799_vcal_8bit_save = pchan->config & 0xf; //ad7799_vcal_8bit_reg; // Save ID (for debug & checking)

//ad7799_vcal_16bit_save.us = (int16_t)pchan->config;

//	if ((pchan->calibzero &0x0ff0) == 0)
//	{ // Here, do internal zero calibration
		spi1ad7799_vcal_push(&sp5a);	// Wait for calib to complete
		ad7799_vcal_wr_mode_reg(pchan->calibzero);	// Start zero calibration
//	}
//	else
//	{ // Skip zero calibration 
//		sp5();
//	}
	return;
}
// ---------------------------------------------------------------------
static void sp3b(void)
{ // Read status register for debugging/test
dbg = 41;
	spi1ad7799_vcal_push(&sp3);
	ad7799_vcal_rd_status_reg();
	return;
}
// ---------------------------------------------------------------------
static void sp2(void)
{ // Readback config register for debugging/test
dbg = 4;
	spi1ad7799_vcal_push(&sp3b);
	ad7799_vcal_rd_configuration_reg ();
	return;
}
// ---------------------------------------------------------------------
static void sp1(void)
{ // Write config register: select mux
dbg = 3;
ad7799id = ad7799_vcal_8bit_reg; // Save ID (for debug & checking)
	spi1ad7799_vcal_push(&sp2);
	ad7799_vcal_wr_configuration_reg (pchan->config); // Select channel et al.
	return;
}
// ---------------------------------------------------------------------
static void sp0(void)
{ // Start here from initial reset
dbg = 2;
	spi1ad7799_vcal_push(&sp1);
	ad7799_vcal_rd_ID_reg();	// Read ad7799 ID
	return;
}
// ---------------------------------------------------------------------
static void spx(void)
{ // Here: exit continuous mode has been sent.  Wait for DOUT interrupt
dbg = 1;
	spi1ad7799_vcal_DOUT_ext(&sp0);	// Enable external interrupt
	return;
}

