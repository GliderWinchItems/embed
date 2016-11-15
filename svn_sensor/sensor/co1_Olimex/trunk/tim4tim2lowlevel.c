/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : tim4tim2lowlevel.c
* Hacker	     : deh
* Date First Issued  : 01/29/2013
* Board              : Olimex
* Description        : Low level interrupt processing for Tim4_pod_se and Tim2_pod_se
*******************************************************************************/
/*
01/29/2013 21:04 Rev 112 before big changes to Tim4, Tim2 timing scheme

*/
#include "common.h"
#include "tim4tim2lowlevel.h"
#include "gps_1pps_se.h"


/* The following variables are used by peripherals in their 'init' routines to set dividers 	*/
/* (see 'lib/libmiscstm32/clockspecifysetup.c') */
extern u32	pclk1_freq;	/*	SYSCLKX/PCLK1DIV	E.g. 72000000 	*/
extern u32	sysclk_freq;	/* 	SYSCLK freq		E.g. 72000000	*/

static u32	tickspersecHI;	// TIM4 clock ticks for one sec: high limit
static u32	tickspersecLO;	// TIM4 clock ticks for one sec: low limit

static struct TIMCAPTRET32 strTim4prev;
static u32 tickspersec;
static u32 Tim2ticksatonesec;
static s16 tickspersecrunning[TICKSRUNNINGSUM];	// Running sum history
static u16 tickspersecidx = 0;	// Index into tickspersecrunning
s32 tickspersecsum;		// Running sum of tickspersec deviation
u32 tim4lowlevel_erct;		// Error count: out-of-range tickspersec

/*#################### TIM4_CH1 -> I2C2_EV_IRQHandle ##########################
 * void tim4lowlevel(void);
 * @brief	: Input capture of GPS 1 PPS low level interrupt processing
 * @return	: 
 * NOTE: this is executed under interrupt.
 #############################################################################*/
/*
The purpose is to compute an average to adjusting the TIM2 counts so that TIM2 
accurately generates 64 ticks per second AND synchronizes those ticks to the
beginning of GPS 1 PPS, which is 1/64th sec tick 0.
*/

void tim4lowlevel(void)
{ // Here, TIM4_CH1 had an input capture due to the GPS 1 PPS 


	return;		
}
/*#################### TIM2_CH2 -> I2C2_ER_IRQHandle ##########################
 * void tim2lowlevel_init(void);
 * @brief	: TIM2 end of one second interrupt
 * @return	: 
 #############################################################################*/
void tim2lowlevel(void)
{
	return;
}
/******************************************************************************
 * void tim4tim2lowlevel_init(void);
 * @brief	: Setup for low level Tim4_pod_se.c and Tim2_pod_se.c routines
 * @return	: 
 ******************************************************************************/
/* This routine is in the main polling loop */
void tim4tim2lowlevel_init(void)
{
	unsigned int temp = (pclk1_freq/10);	// Allow +/- 10% range on ticks per sec

	/* Set limits for valid processor ticks between 1 PPS events */
	tickspersecHI = pclk1_freq + temp;
	tickspersecLO = pclk1_freq - temp;

	/* Set low level interrupt dispatch pointers. */
	tim4ic_ptr  = &tim4lowlevel;	// See 'Tim4_pod_se.c'
	tim2end_ptr = &tim2lowlevel;	// See 'Tim2_pod_se.c'

	return;
}

