/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : gps_1pps.h
* Hackeroo           : deh
* Date First Issued  : 01/05/2013
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines that use PC13->PE9 for measuring 1 pps gps vs 32 KHz osc
*******************************************************************************//* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPS1PPS
#define __GPS1PPS

/* Note: The lower four bits of the priority byte are not used.
 The higher the priority number, the lower the priority */
#define TIM1CC_PRIORITY		0x40	// Interrupt priority for TIM1 interrupt
#define TIM1UP_PRIORITY		0x50	// Interrupt priority for TIM1 interrupt
#define TIM2_PRIORITY		0x20	// Interrupt priority for TIM2 interrupt
#define TIM3_PRIORITY		0x40	// Interrupt priority for TIM3 interrupt

union TIMCAPTURE64
{
	unsigned short	us[4];
	unsigned int	ui[2];
	unsigned long long ll;
};

struct TIMCAPTRET32
{
	unsigned int 	ic;
	unsigned short	flg;
	unsigned int	cnt;	// RTC_CNT (in memory mirror)
};



/******************************************************************************/
void GPS_1pps_init(void);
/* @brief	: Initialize PC13->PE9 RTC-to-timer connection
******************************************************************************/
unsigned long long Tim1_gettime_ll(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned long long
******************************************************************************/
unsigned long long Tim3_gettime_ll(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned long long
*******************************************************************************/
unsigned int Tim1_gettime_ui(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
unsigned int Tim3_gettime_ui(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
struct TIMCAPTRET32 Tim1_inputcapture_ui(void);
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
void Tim2_pod_init(void);
/* @brief	: Initialize Tim2 for input capture
*******************************************************************************/
unsigned long long Tim2_gettime_ll(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned long long
******************************************************************************/
unsigned int Tim2_gettime_ui(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
struct TIMCAPTRET32 Tim2_inputcapture_ui(void);
/* @brief	: Retrieve the extended capture timer counter count and flag counter
 * @brief	: Lock interrupts
 * @return	: Current timer count and flag counter in a struct
*******************************************************************************/

/* The readings and flag counters are updated upon each capture interrupt (@4) */
//extern volatile unsigned short		usTim2ch2_Flag;		// Incremented when a new capture interrupt serviced, TIM2CH1*
//extern volatile unsigned int	uiRTCsystemcounterTim2IC;	// Save RTC count upon each Tim2 IC




#endif
