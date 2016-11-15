/******************************************************************************
* File Name          : gps_1pps_se.h
* Date First Issued  : 01/05/2013
* Board              : STM32F103VxT6_pod_mm
* Description        : Routines that use PC13->PE9 for measuring 1 pps gps vs 32 KHz osc
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPS1PPS_CO
#define __GPS1PPS_CO

#include "irq_priorities_co1.h"

#include "p1_common.h"

//union TIMCAPTURE64
//{
//	unsigned short	us[4];
//	unsigned int	ui[2];
//	unsigned long long ll;
//};

//struct TIMCAPTRET32
//{
//	unsigned int 	ic;
//	unsigned int	flg;
//};



/******************************************************************************/
void Tim4_pod_init(void);
/* @brief	: Initialize Tim4 for input capture
******************************************************************************/
void Tim3_pod_init(void);
/* @brief	: Initialize Tim3 for input capture
******************************************************************************/
void p1_Tim2_pod_init(void);
/* @brief	: Initialize Tim2 for input capture
******************************************************************************/
void p1_GPS_1pps_init(void);
/* @brief	: Initialize PC13->PE9 RTC-to-timer connection
******************************************************************************/

/*******************************************************************************/
unsigned int p1_Tim1_gettime_ui(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
unsigned int p1_Tim2_gettime_ui(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
unsigned int Tim3_gettime_ui(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned int
*******************************************************************************/



/*******************************************************************************/
struct TIMCAPTRET32 p1_Tim1_inputcapture_ui(void);
/* @brief	: Retrieve the extended capture timer counter count
 * @brief	: Lock interrupts
 * @return	: Current timer count as an unsigned int
*******************************************************************************/
struct TIMCAPTRET32 p1_Tim2_inputcapture_ui(void);
/* @brief	: Retrieve the extended capture timer counter count and flag counter
 * @brief	: Lock interrupts
 * @return	: Current timer count and flag counter in a struct
*******************************************************************************/
struct TIMCAPTRET32 Tim3_inputcapture_ui(void);
/* @brief	: Retrieve the extended capture timer counter count and flag counter
 * @brief	: Lock interrupts
 * @return	: Current timer count and flag counter in a struct
*******************************************************************************/
struct TIMCAPTRET32 Tim4_inputcapture_ui(void);
/* @brief	: Retrieve the extended capture timer counter count and flag counter
 * @brief	: Lock interrupts
 * @return	: Current timer count and flag counter in a struct
*******************************************************************************/

/*******************************************************************************/
unsigned long long p1_Tim1_gettime_ll(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned long long
******************************************************************************/
unsigned long long Tim3_gettime_ll(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned long long
*******************************************************************************/
unsigned long long p1_Tim2_gettime_ull(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned long long
******************************************************************************/
unsigned long long Tim3_gettime_ll(void);
/* @brief	: Retrieve the extended timer counter count
 * @return	: Current timer count as an unsigned long long
*******************************************************************************/

/******************************************************************************/
struct CANRCVBUF* Tim4_pod_se_sync_msg_get(void);
/* @brief	: Get pointer to msg in a buffer
 * @return	: NULL = buffer empty, otherwise pointer to bu
*******************************************************************************/
void Tim4_pod_se_set_sync_msg(void);
/* @brief	: Set fixed info in time sync msg
*******************************************************************************/


/* Pointers to functions to be executed under a low priority interrupt */
extern void (*tim4ic_ptr)(void);	// Address of function to call forced interrupt
extern void (*tim2end_ptr)(void);	// Address of function to call forced interrupt
extern u32 tim2_cnt_at_tim4_ic;		// TIM2 counter at time of TIM4 CH1 input capture

extern s32	deviation_oneinterval;	// "whole.fraction" for interval adjustment for duration
extern s32	phasing_oneinterval;	// "whole.fraction" for interval adjustment for phasing

/* Running count of instances where measured tickspersec of clock is outside limits. */
extern u32	tim4_tickspersec_err;

/* Flag to tell 'can_log.c' that time & syncing is ready */
/* When 'tim4_readyforlogging == 0x3' then the time is stable */
extern u8	tim4_readyforlogging;

/* These count forced adjustments to the date/time at the tick level */
extern u16	tim4_64th_0_er;
extern u16	tim4_64th_19_er;

/* Index for double buffer that switches each second when GPS 1 PPS (ic) arrives. */
extern volatile u32 idx_cmd_n_ct;	// Index for double buffering can msg counts

#endif
