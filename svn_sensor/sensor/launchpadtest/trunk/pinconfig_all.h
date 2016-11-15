/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : pinconfig_all.h
* Hackeroo           : deh
* Date First Issued  : 01/21/2013
* Board              : STM32F103xx
* Description        : Configure gpio port pins
*******************************************************************************/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PODPIN_ALL
#define __PODPIN_ALL

/* Includes ------------------------------------------------------------------*/
#include "libopenstm32/common.h"	// Has things like 'u16' defined here
#include "libopenstm32/gpio.h"

/* RM0090 rev 3 section 7.3 p 185
● Input floating
● Input pull-up
● Input-pull-down
● Analog
● Output open-drain with pull-up or pull-down capability
● Output push-pull with pull-up or pull-down capability
● Alternate function push-pull with pull-up or pull-down capability
● Alternate function open-drain with pull-up or pull-down capability

RM00008 rev 14 section 9.1 p 154
● Input floating
● Input pull-up
● Input-pull-down
● Analog
● Output open-drain
● Output push-pull
● Alternate function push-pull
● Alternate function open-drain

1. GP = general-purpose, PP = push-pull, PU = pull-up, PD = pull-down, OD = open-drain, AF = alternate
   function.

*/
/* The following list is the 8 possible ways a pin can be configured. */
enum pin_use_code
{
	// The following is in the order listed in RM0008 rev 14, p 156, table 20
	OUT_PP		= 0,	// Output push-pull with pull-up or pull-down capability
	OUT_OD		= 1,	// Output open-drain with pull-up or pull-down capability
	OUT_AF_PP	= 2,	// Alternate function push-pull with pull-up or pull-down capability
	OUT_AF_OD	= 3,	// Alternate function open-drain with pull-up or pull-down capability
	IN_ANALOG	= 4,	// Analog
	IN_FLT		= 5,	// Input floating
	IN_PD		= 6,	// Input-pull-down
	IN_PU		= 7	// Input pull-up
};

/* For the F103 series these three speeds are possible for output configurations. */
enum pin_out_speed	// Output pin speed
{
	MHZ_10		= 0x01,
	MHZ_2		= 0x02,
	MHZ_50		= 0x03
};

/* This struct is used to pass the setup parameters to the setup routine. */
struct PINCONFIGALL
{
	volatile u32 *port;	// Port--Example: GPIOA
	u8	pin;		// Pin number--Example: 14
	u8	usecode;	// Pin type of use--pin_use_code
	u8	speed;		// Speed (used for output only): Example: 50_MHZ
};

/* Subroutines */
/* **************************************************************************************************************/
int pinconfig_all (struct PINCONFIGALL *p);
/* @brief	: configure a pin according to the parameters in the struct
 * @param	: p = pointer to struct (see .h file)
 * @return	: negative = error; 0 = OK
 ************************************************************************************************************** */


#endif 


