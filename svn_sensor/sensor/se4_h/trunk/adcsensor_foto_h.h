/******************************************************************************
* File Name          : adcsensor_foto_h.h
* Date First Issued  : 01/31/2014
* Board              : RxT6
* Description        : ADC routines for f103 sensor--sensor board histogram collection
*******************************************************************************/
/*
11/11/2012 This is a hack of svn_pod/sw_stm32/trunk/devices/adcpod.h

Strategy--
Two photocell emitters are measured using on ADC3 and ADC2.  The threshold registers
are used to trigger an interrupt when there is a transition between states of the 
sensed photo reflections.

DMA stores readings for generation of a histogram.  Since DMA1 stores ADC2 in the high
half of the 32b ADC1 data register and the watchdog for ADC1 is 16b, the comparison
to the ADC1 DR no longer works.  Therefore, ADC1 is used for the DMA storing of ADC2, but
ADC2 and ADC3 are used for the watchdog detection.


11/15/2012 - POD prototype setup
Emitter resistors--3.3K
Diode resistors--330
ADC voltage applied--3.3v

*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADCSENSOR_FOTO_H
#define __ADCSENSOR_FOTO_H

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined
#include "libopenstm32/gpio.h"
#include "libopenstm32/spi.h"
#include "libopenstm32/rcc.h"
#include "libopenstm32/rtc.h"
#include "../../../../svn_common/trunk/common_can.h"
#include "common_misc.h"
#include "common_highflash.h"
//#include "../../../../../svn_discoveryf4/PC/sensor/cangate/trunk/tmpstruct.h"	// structs generated from .txt file for parameters, etc.
#include "../../../../svn_common/trunk/db/can_db.h"


#define DMA1_CH1_PRIORITY	0x80	// 
#define DMA2_CH5_PRIORITY	0x80	// 

#define NUMBERADCCHANNELS_FOTO	1	// Number of channels each ADC is to scan
#define DISCARD_FOTO		32	// Number of readings to discard before filtering starts
/* Decimation|CICSCALE (for unity scaling) 
    1    3
    2	 6
    4	 9
    8	12
   16	15
   32	18                                                                     */
#define DECIMATION_FOTO		4	// Decimation ratio
#define CICSCALE_FOTO		9	// Right shift count to scale result

#define ADCERRORCTRSIZE	4	// Number of error counters for adcsensor.c

/* These define the initial values for the ADC watchdog registers */
#define ADC3_HTR_INITIAL	900		// High threshold register setting, ADC3
#define ADC3_LTR_INITIAL	400		// Low  threshold register setting, ADC3
#define ADC2_HTR_INITIAL	1100		// High threshold register setting, ADC2
#define ADC2_LTR_INITIAL	700		// Low  threshold register setting, ADC2

/* Histogram additions */
union ADC12VAL		// In dual mode: DMA stores pairs of ADC readings, two 16b readings in one 32b word
{
	u32	ui;
	u16	us[2];
};

#define ADCVALBUFFSIZE	64	// Number of ADC readings in a *half* DMA buffer
#define ADCHISTOSIZE 	64	// Number of histogram bins (that accumulate counts)
#define THROTTLE	7	// Number of 1/2048 sec ticks between sending bin msgs
#define THROTTLE2	64	// Number of 1/2048 sec ticks between sending bin msgs
#define ADC3ADC2READCT	2048	// Number of readings output before shutting it off

/******************************************************************************/
void adc_init_sequence_foto_h(struct FLASHP_SE4* phfp);
/* @brief 	: Call this routine to do a timed sequencing of power up and calibration
 * @param	: phfp = pointer to struct stored in highflashp area
*******************************************************************************/
void adc_histo_cansend(struct CANRCVBUF* p);
/* @brief 	: Send data in response to a CAN msg
 * @param	: p = pointer to 'struct' with CAN msg
 * @return	: ? 
*******************************************************************************/


extern long speed_filteredA2;		// Most recent computed & filtered rpm
extern u32 encoder_ctrA2;		// Most recent encoder count
extern u32 adc_encode_time_prev2;	// Previous time at SYSTICK
extern s32 encoder_ctr_prev2;		// Previous encoder running count

/* Error counters */
extern u32 adcsensor_foto_err[ADCERRORCTRSIZE];	// Error counters



#endif 
