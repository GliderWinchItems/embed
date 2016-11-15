/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_gps_1pps.h
* Hackeroo           : deh
* Date First Issued  : 07/22/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines that use PC13->PE9 for measuring 1 pps gps vs 32 KHz osc
*******************************************************************************//* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPS1PPS_P1
#define __GPS1PPS_P1
/*
10-11-2011: Modified for 'pod_v1' to allow for 64 bit tick counter
  and count interrupts for 1 sec intervals.
*/
/* Note: The lower four bits of the priority byte are not used.
 The higher the priority number, the lower the priority */
#define TIM1CC_PRIORITY		0x30	// Interrupt priority for TIM1 interrupt
#define TIM1UP_PRIORITY		0x30	// Interrupt priority for TIM1 interrupt
#define TIM2_PRIORITY		0x10	// Interrupt priority for TIM2 interrupt
#define TIM3_PRIORITY		0x20	// Interrupt priority for TIM3 interrupt



/******************************************************************************/
void p1_GPS_1pps_init(void);
/* @brief	: Initialize PC13->PE9 RTC-to-timer connection
******************************************************************************/
unsigned long long p1_Tim1_gettime_ll(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned long long
******************************************************************************/
unsigned long long Tim3_gettime_ll(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned long long
*******************************************************************************/
unsigned int p1_Tim1_gettime_ui(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
unsigned int Tim3_gettime_ui(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
struct TIMCAPTRET32 p1_Tim1_inputcapture_ui(void);
/* @brief	: Retrieve the extended capture timer counter count
 * @brief	: Lock interrupts
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
struct TIMCAPTRET32 Tim3_inputcapture_ui(void);
/* @brief	: Retrieve the extended capture timer counter count and flag counter
 * @brief	: Lock interrupts
 * @return	: Current timer count and flag counter in a struct
*******************************************************************************/

/******************************************************************************/
void Tim3_pod_init(void);
/* @brief	: Initialize Tim3 for input capture
******************************************************************************/
void p1_Tim2_pod_init(void);
/* @brief	: Initialize Tim2 for input capture
*******************************************************************************/
unsigned long long p1_Tim2_gettime_ull(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned long long
******************************************************************************/
unsigned int p1_Tim2_gettime_ui(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
struct TIMCAPTRET32 p1_Tim2_inputcapture_ui(void);
/* @brief	: Retrieve the extended capture timer counter count and flag counter
 * @brief	: Lock interrupts
 * @return	: Current timer count and flag counter in a struct
*******************************************************************************/

/* The readings and flag counters are updated upon each capture interrupt (@4) */
extern volatile unsigned short		p1_usTim2ch2_Flag;		// Incremented when a new capture interrupt serviced, TIM2CH1*

extern volatile unsigned int Debug_TIM1;
extern volatile unsigned int nTim1Debug0;
extern volatile unsigned int uiTim1onesec;


#endif
