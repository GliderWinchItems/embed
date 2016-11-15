/******************** (C) COPYRIGHT 2010 **************************************
* File Name          : SYSTICK_getcount32.c
* Hackor             : deh
* Date First Issued  : 10/21/2010
* Description        : SYSTICK setup and related
*******************************************************************************/
#include "../libopenstm32/systick.h"
#include "../libmiscstm32/systick1.h"

/*******************************************************************************
 * u32 SYSTICK_getcount32(void);
 * @brief	: Gets 32 bit systick count
 * @param	: none
 * @return	: Get current 32 bit systick count
 *******************************************************************************/
u32 SYSTICK_getcount32(void)
{
	u32 temp;
	u8 hibyte;
	do
	{
		temp = STK_VAL;		// Get current reading
		hibyte = systick_hibyte;// Get extended byte
	}
	/* If 'systick_hibyte' changed, then there was a SYSTICK interrupt
	   between the above two instructions, so re-read */
	while (hibyte != systick_hibyte);
	
	return ((hibyte << 24) + temp);	
}

