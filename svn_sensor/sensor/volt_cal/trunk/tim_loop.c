/******************************************************************************
* File Name          : tim_loop.c
* Date First Issued  : 08/07/2015
* Board              : f103
* Description        : timer triggered polling loop
*******************************************************************************/
/*

*/
#include "tim_clk.h"
#include "triac_ctl.h"

extern void 	(*tim3_yog_ptr)(void);		// Address of function to call at timer complete tick
extern void 	(*tim3_yog_ll_ptr)(void);	// Low Level function call
void tim_loop_poll(void);

/******************************************************************************
 * void tim_loop_init(void);
 * @brief	: Initialize 
*******************************************************************************/
void tim_loop_init(void)
{
		/* Call back from tim3_yog. */
	tim3_yog_ptr = &tim_loop_poll;

}
/*#######################################################################################
 * From tim3_yog timer: high priority
 *####################################################################################### */
void tim_loop_poll(void)
{
	tim_clk_counter();	// Tick the time counters
	
	triac_ctl_poll();	// Update the PWM

	return;
}
