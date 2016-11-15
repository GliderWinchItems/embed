/******************************************************************************
* File Name          : timer_thread.h
* Date First Issued  : 12/03/2014
* Board              : PC
* Description        : timer
*******************************************************************************/


#ifndef __TIMER_THREAD_CG
#define __TIMER_THREAD_CG

/******************************************************************************/
int timer_thread_init(void* func, int rate);
/* @brief 	: Start new thread if not started and set select timeout 
 * @param	: func(void) = Pointer to function to call upon select timeout
 * @param	: rate = timeout count (microsecs)
*******************************************************************************/
void timer_thread_shutdown(void);
/* @brief 	: Shutdown timer and cancel thread
*******************************************************************************/

extern unsigned int timer_thread_ctr;	// Count select timeouts

#endif

