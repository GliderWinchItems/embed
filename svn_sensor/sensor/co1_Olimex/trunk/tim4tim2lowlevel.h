/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : tim4tim2lowlevel.h
* Hacker	     : deh
* Date First Issued  : 01/29/2013
* Board              : Olimex
* Description        : Low level interrupt processing for Tim4_pod_se and Tim2_pod_se
*******************************************************************************/

#ifndef __TIM4TIM2LOWLEVEL
#define __TIM4TIM2LOWLEVEL

#define TICKSRUNNINGSUM	8	// Number of events in running sum of deviations

#include "common.h"

/******************************************************************************/
void tim4tim2lowlevel_init(void);
/* @brief	: Setup for low level Tim4_pod_se.c and Tim2_pod_se.c routines
 * @return	: 
 ******************************************************************************/


extern s32 tickspersecsum;	// Running sum of tickspersec deviation


#endif


