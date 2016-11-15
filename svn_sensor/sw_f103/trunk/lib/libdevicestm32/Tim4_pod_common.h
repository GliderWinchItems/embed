/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : Tim4_pod_common.h
* Hackeroo           : deh
* Date First Issued  : 02/01/2013
* Board              : STM32F103VxT6_pod_mm
* Description        : Sensor board timing
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPS1PPS_SE_COMMON
#define __GPS1PPS_SE_COMMON

#include "common.h"


/* Note: The lower four bits of the priority byte are not used.
 The higher the priority number, the lower the priority */
#define TIM1CC_PRIORITY		0x30	// Interrupt priority for TIM1 interrupt
#define TIM1UP_PRIORITY		0x30	// Interrupt priority for TIM1 interrupt
#define TIM2_PRIORITY		0x10	// Interrupt priority for TIM2 interrupt
#define TIM3_PRIORITY		0x20	// Interrupt priority for TIM3 interrupt
#define NVIC_I2C2_EV_IRQ_PRIORITY_A 0xC0	// Post TIM4 input capture interrupt processing
#define NVIC_I2C2_ER_IRQ_PRIORITY 0xe0	// Post TIM2 interrupt processing

union TIMCAPTURE64
{
	unsigned short	us[4];
	unsigned int	ui[2];
	unsigned long long ll;
};

struct TIMCAPTRET32
{
	unsigned int 	ic;
	unsigned int	flg;
};



/******************************************************************************/
void Tim4_pod_init(void);
/* @brief	: Initialize Tim4 for input capture
******************************************************************************/
u32 Tim4_pseudoinputcapture(void);
/* @brief	: 
 * @brief	: Lock interrupts
 * @return	: Current timer count
*******************************************************************************/
void Tim4_gettime(union LL_L_S * ptime);


/*#######################################################################################*/
void CAN_sync(s32 fifo1cycnt);
/* entry is from low-level 'I2C1_EV_IRQHandler' following FIFO 1 interrupt
 * fifo1cycnt = DTW_CYCCNT (32b) processor cycle counter saved at FIFO 1 interrupt entry.
 * A FIFO 1 (time sync) msg causes interrupt to CAN_RX1_Handler, which saves the DTW_CYCCNT
 * count, then triggers 'I2C1_EV_IRQHandler' interrupt for low priority level handling, which
 * calls this routine, 'CAN_sync'.
 * This routine serves as a "pseudo" input capture caused by a CAN time sync msg
 *####################################################################################### */

/* Pointers to functions to be executed under a low priority interrupt */
extern void (*tim4ic_ptr)(void);	// Address of function to call forced interrupt
extern void (*tim2end_ptr)(void);	// Address of function to call forced interrupt

/* Pointer to directing end of 1/64th sec TIM4 output capture */
extern void 	(*tim4_end64th_ptr)(void);	// Address of function to call at end of 1/64th sec interval tick

extern u32 tim2_cnt_at_tim4_ic;		// TIM2 counter at time of TIM4 CH1 input capture

extern u8 	GPStimegood;		// 0 = waiting for good GPS; 1 = GPS time sentences good	
extern u32	tim4lowlevel_er1;	// Error count 1: bogus time per sec.
extern u32	tim4_pod_se_debug0; 	// Count of 1/64th out-of-sync's
extern s32	deviation_oneinterval;



#endif
