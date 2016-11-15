/******************************************************************************
* File Name          : SYSTICK_systickduration.c
* Date First Issued  : 10/21/2010
* Description        : SYSTICK setup and related
*******************************************************************************/
#include "../libopencm3/stm32/systick.h"
#include "../libmiscstm32f4/systick1.h"

/*******************************************************************************
 * u32 SYSTICK_systickduration(u32 old_systick);
 * @brief	: Read SYSTICK counter, and compute difference from old reading
 * @param	: none
 * @return	: Difference between current counter reading and previous saved reading
 *******************************************************************************/
u32 SYSTICK_systickduration(u32 old_systick)
{
	/* Compute difference between current count and old count */
	s32 temp = old_systick - STK_VAL;	

	if (temp > 0) return temp;	// No wrap.
	return (temp + STK_LOAD +1);	// Adjust for wrap=around
}

