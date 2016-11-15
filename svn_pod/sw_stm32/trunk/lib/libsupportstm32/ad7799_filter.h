/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : ad7799_filter.h
* Hackeroos          : deh
* Date First Issued  : 07/18/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : ad7799 filtering
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AD7799_FILTER
#define __AD7799_FILTER

/******************************************************************************/
void ad7799_poll_rdy (void);
/* @brief	: Check /RDY and complete a ready when ready
*******************************************************************************/
void ad7799_filter (int nInput);
/* @brief	: Do three sinc filters
 * @param	: nInput = ad7799 reading, bipolar and adjusted
*******************************************************************************/

/* Address ad7799_filter go to upon completion (@6) */
extern void 	(*ad7799_filterdone_ptr)(void);		// Address of function to call to go to for further handling under RTC interrupt

/* Filtering of tension data (@6) */
extern long long	llAD7799_out;		// Filtered/decimated data output
extern unsigned short	usAD7799filterFlag;	// Filtered/decimated data ready: 0 = not ready, not zero = ready


#endif 
