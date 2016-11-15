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

#define AD7799REINITVALUE	-337096		// Value we insert when doing a re-initialization of the AD7799 (approx -50 kg)
#define AD7799OORANGEHI		 8000000	// Above this value is an Out Of Range value (approx 2,281 kg)
#define AD7799OORANGELO		-8000000	// Below this value is an Out Of Range value
#define	AD7799SHUTDNCT		15		// Number of shutdown intervals for power supply shutdown delay

/* Note: the ad7799 is polled at 2048 /sec, but it only produces new readings at about 470 /sec, so the 
 last_good_reading can only change every 4+ polls.  "1024" is 1/2 sec of OOR values */
#define AD7799OORCT		1024		// Number of consecutive OOR cases before doing a shutdowbn/re-initializatin sequence

#define POWERDELAY	1000 	// Powerup delay interval, 100 ms (count in terms of 0.1 millisecond)
#define POWERUPDELAYR	5000 	// Powerup delay (0.1 millisecond) AD7799
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
