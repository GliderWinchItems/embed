/******************************************************************************
* File Name          : ad7799_vcal_filter.h
* Date First Issued  : 10/09/2015
* Board              : STM32F103VxT6_pod_mm
* Description        : ad7799 filtering--multiple channels
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AD7799_FILTER
#define __AD7799_FILTER

#include <stdint.h>

#define AD7799NUMCHANNELS	3	// Number of channels
#define CHANNELAIN_1	0	// Channel number for first array position
#define CHANNELAIN_2	1	// Channel number for second array position
#define CHANNELAIN_3	2	// Channel number for third array position
#define READSUMCT	96	// Number of readings to sum

/* Parameters for reading a mux channel.  (There WILL be three of these.) */
struct AD7799CHAN
{
	uint16_t mode;		// Mode register setting (for readings)
	uint16_t calibzero;	// Calibration (mode) setting zero (0x0ff0 = skip)
	uint16_t calibfs;	// Calibration (mode) setting fullscale (0x0ff0 = skip)
	uint16_t config;	// Configuraton register setting
	uint16_t ct;		// Number of readings to sum (0 = skip channel)
	uint16_t ctr;		// Number of readings summed (working counter)
	int32_t  sum;		// Summation of readings, working (for interrupt)
	int32_t  sum_save;	// Summation saved (for mainline)
	uint16_t chanflag;	// Channel flag (increment when data ready)
	uint8_t	 mux;		// Mux channel number
};

/******************************************************************************/
void ad7799_vcal_poll_rdy (void);
/* @brief	: Check /RDY and complete a ready when ready
*******************************************************************************/
void ad7799_vcal_filter (int nInput);
/* @brief	: Do three sinc filters
 * @param	: nInput = ad7799 reading, bipolar and adjusted
*******************************************************************************/
void ad7799_vcal_filter_start(struct AD7799CHAN* p);
/* @brief	: Start interrupt driven ad7799 readings; sequence though channels
 * @param	: p = pointer to array of parameters for channels
*******************************************************************************/
void ad7799_vcal_filter_init_param(struct AD7799CHAN* p);
/* @brief	: Initialize channel parameters
 * @param	: p = pointer to array of parameters for channels
*******************************************************************************/

/* When all channels have been read & summed, save in buffer and set this flag. */
extern uint16_t ad7799_ready_flag;	// Flag showing new 'sum_save' data ready

/* Address this routine will go to upon completion of 'comm calls. */
extern void 	(*ad7799_vcal_filterdone_ptr)(void);

/* Completed sequence of channel readings saved for mainline */
extern int32_t sum_save[AD7799NUMCHANNELS];

#endif 
