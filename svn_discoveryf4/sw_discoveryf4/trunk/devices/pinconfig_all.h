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
#include "common.h"	// Has things like 'u16' defined here
#include "gpio.h"

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

/* For the F4 series these are possible for output configurations.
These bits are written by software to configure the I/O output speed.
00: 2 MHz Low speed
01: 25 MHz Medium speed
10: 50 MHz Fast speed
11: 100 MHz High speed on 30 pF (80 MHz Output max speed on 15 pF) */
enum pin_out_speed	// Output pin speed
{
	MHZ_2		= 0x0,
	MHZ_25		= 0x1,
	MHZ_50		= 0x2,
	MHZ_100		= 0x3
};
/* Pull up/dn/none code */
enum pin_pull_code
{
	PULL_NONE	= 0x0,	// No pull up or down
	PULL_UP		= 0x1,	// Pull up
	PULL_DN		= 0x2	// Pull down
};

/* This struct is used to pass the setup parameters to the setup routine. */
struct PINCONFIGALL
{
	volatile u32 *port;	// Port--Example: GPIOA
	u8	pin;		// Pin number--Example: 14
	u8	usecode;	// Pin type of use--pin_use_code
	u8	speed;		// Speed (used for output only): Example: 50_MHZ
	u8	pullcode;	// Pull up/dn code: (0 = none)
	u8	afcode;		// Alternate function code (0 = none; see data sheet table)
};

/* Subroutines */
/* **************************************************************************************************************/
void pinconfig_all (struct PINCONFIGALL *p);
/* @brief	: configure a pin according to the parameters in the struct
 * @param	: p = pointer to struct (see .h file)
 * @return	: negative = error; 0 = OK
 ************************************************************************************************************** */


#endif 


