/******************************************************************************
* File Name          : can_log.c
* Date First Issued  : 05/10/2014
* Board              : Sensor board w flash genie SD module
* Description        : Setup and handling of socket switches module LED
*******************************************************************************/
/*
*/
/* 
PC4  grn LED (inverter drives RED/GRN LED pair hi/lo)
PC10 wht PRO write protect (not write protected, lo)
PC11 yel DET insertion sw  (inserted, lo)
*/

#include "libopenstm32/gpio.h"
#include "pinconfig_all.h"
#include "SENSORpinconfig.h"

static const struct PINCONFIGALL led = {(volatile u32 *)GPIOC,  4, OUT_PP, MHZ_2};
static const struct PINCONFIGALL det = {(volatile u32 *)GPIOC, 10, IN_FLT, 0};
static const struct PINCONFIGALL pro = {(volatile u32 *)GPIOC, 11, IN_FLT, 0};

/******************************************************************************
 * int SD_socket_init(void);
 * @brief 	: Setup pins for flash genie socket module connections to sensor board
 * @return	: 0 = OK, not zero for error.
*******************************************************************************/
int SD_socket_init(void)
{
	int err;
	/* LED driver for LED red/grn LED on flash genie socket module */
	err  = pinconfig_all( (struct PINCONFIGALL *)&led);
	err |= pinconfig_all( (struct PINCONFIGALL *)&det);
	err |= pinconfig_all( (struct PINCONFIGALL *)&pro);
	return err;
}
/******************************************************************************
 * void SD_socket_setled(int onoff);
 * @brief 	: Set
 * @param	: onoff: 0 set RED, not-zero set GREEN
*******************************************************************************/
void SD_socket_setled(int which)
{
	if (which == 0)
	{
		LED19RED_off; return;
	}
	LED19RED_on; return;
}
/******************************************************************************
 * int SD_socket_sw_status(int sw);
 * @brief 	: Get status of flash genie socket module switch
 * @param	: sw: 1 = insertion switch; 0 = write protection switch
 * @return	: 0 = switch closed; not zero = switch open; negative = bad 'sw' number
*******************************************************************************/
int SD_socket_sw_status(int sw)
{
	if (sw > 1) return -1;
	if (sw < 0) return -1;
	return ( GPIOC_IDR & (1 << (sw + 10)) );

}
/******************************************************************************
 * int SD_socket_sw_status_and_setlet(void);
 * @brief 	: Get status of flash genie socket module switch
 * @return	: 0 = OK (and LED set to GRN); not zero = SD is not inserted (led set to RED)
*******************************************************************************/
int SD_socket_sw_status_and_setlet(void)
{
	int ret = SD_socket_sw_status(0); // Get insertion switch status
	if (ret == 0) // Is it closed?
		SD_socket_setled(1); // Yes, set GRN
	else	
		SD_socket_setled(0); // No, set RED
	return ret;
}
