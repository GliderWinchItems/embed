/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : p1_get_reset_mode.h
* Hackeroos          : caw, deh
* Date First Issued  : 08/30/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Sort out type of reset and some other related things ;)
*******************************************************************************/
/*
08-30-2011


*/
#ifndef __P1_GET_RESET_MODE
#define __P1_GET_RESET_MODE

#define RESET_MODE_NORMAL	0
#define RESET_MODE_COLD		1


/******************************************************************************/
int p1_get_reset_mode(void);
/*@brief 	: Get started with the sequence when reset is comes out of STANDBY
 * @return	: Reset mode type
*******************************************************************************/


#endif

