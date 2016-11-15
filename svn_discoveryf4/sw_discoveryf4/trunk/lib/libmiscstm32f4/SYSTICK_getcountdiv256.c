/******************************************************************************
* File Name          : SYSTICK_getcountdiv256.c
* Date First Issued  : 10/22/2010
* Description        : SYSTICK setup and related
*******************************************************************************/
/*
Get the SYSTICK count extended from the hardware 24 bits to 64 bits.
The 32 lower bits of the 64 gives a cycle time, ( one tick of the upper 32 bits)
of --
59.6523235556 seconds
(20.8 seconds short of a full minute)
The full 64 bit cycle is too long for mortals.
*/
#include "../libopencm3/stm32/systick.h"
#include "../libmiscstm32f4/systick1.h"

/*******************************************************************************
 * u32 SYSTICK_getcountdiv256(void);
 * @brief	: Gets 64 bit systick count shift right 8 bits
 * @param	: none
 * @return	: Get current systick/256
 *******************************************************************************/
u32 SYSTICK_getcountdiv256(void)
{
union USYS
{
	unsigned long long T;
	u32 temp[2];
} usys;

	do
	{
		usys.temp[0] = SYSTICK_getcount32();	// Get current reading
		usys.temp[1] = systick_u32hi;		// Get extended 32 bits
	}
	/* If 'systick_u32hi' changed, then re-read */
	while (usys.temp[1] != systick_u32hi);
	
	return (usys.T >> 8);
}

