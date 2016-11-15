/******************************************************************************
* File Name          : ad7799_filter_ten2.h
* Date First Issued  : 07/03/2015
* Board              : STM32F103VxT6_pod_mm
* Description        : ad7799 filtering
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AD7799_FILTER_TEN2
#define __AD7799_FILTER_TEN2

#include "cic_filter_ll_N2_M3.h"
#include "iir_filter_l.h"

#define NADCS	2	// Number of AD7799s
#define NSTAGES	2	// Number of cic stages for one data stream

struct CICLL	// Basic long long struct for one CIC stage
{
	struct CICLLN2M3 cic;		// Values for one cic stage
	long long llout_save;		// Buffered/saved output
	unsigned short Flag;		// Stage complete flag 
	unsigned short Flag_prev;	// Previous flag count
	unsigned short Flag2;		// 2nd flag for interrupt routine
};

/******************************************************************************/
void ad7799_poll_rdy_ten2 (void);
/* @brief	: Check /RDY and complete a ready when ready
*******************************************************************************/
uint16_t ad7799_poll_rdy_ten2_both(void);
/* @brief	: Check /RDY and start an spi read 24 bit register sequence
*******************************************************************************/
void ad7799_filter_ten2 (int nInput);
/* @brief	: Do three sinc filters
 * @param	: nInput = ad7799 reading, bipolar and adjusted
*******************************************************************************/
void ad7799_poll_rdy_ten2_cic(void);
/* @brief	: Filter with CIC. (Called by tim3_ten2.c)
 ******************************************************************************/
void ad7799_poll_rdy_ten2_req_calib(uint16_t n);
/* @brief	: Request a re-calibration sequence
 * @param	: n = AD7799 number: 0 = AD7799_1, 1 = AD7799_2
*******************************************************************************/
int ad7799_poll_rdy_ten2_req_calib_busy(uint16_t n);
/* @brief	: Request a re-calibration sequence
 * @param	: n = AD7799 number: 0 = AD7799_1, 1 = AD7799_2
 * @param	: 0 = not busy; 1 = sequence is in progress;
*******************************************************************************/


/* Address ad7799_tenfilter go to upon completion (@6) */
extern void 	(*ad7799_ten_filterdone_ptr)(void);		// Address of function to call to go to for further handling under RTC interrupt

/* Filtering of tension data (@6) */
// 1st stage
extern long long	llAD7799_ten_out;		// Filtered/decimated data output
extern unsigned short	usAD7799_tenfilterFlag;	// Filtered/decimated data ready: 0 = not ready, not zero = ready
// 2nd stage
extern long long	llAD7799_ten_out2;		// Filtered/decimated data output
extern unsigned short	usAD7799_tenfilterFlag2;	// Filtered/decimated data ready: 0 = not ready, not zero = ready

extern struct CICLL cic[NADCS][NSTAGES];	// Two AD7799 filtering, output buffer, flags
extern struct IIRFILTERL iir_filtered[NADCS][NADCS];	// Two AD7799 readings after iir filter

extern unsigned int ad7799_ten2_flag;	// Flag for flashing LED

extern unsigned short ad7799_filter_ten2_flagct;

extern struct IIR_L_PARAM iirparam_offset_const;

#endif 
