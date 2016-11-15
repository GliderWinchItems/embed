/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : RS232_ctl.h
* Author	     : deh
* Date First Issued  : 09/01/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Power up/down, initialization contorl for USART1, UART4
*******************************************************************************/

#ifndef __RS232_CTL
#define __RS232_CTL


/* Short for debugging (10 secs) */
//#define RS232TIMEOUT	10	// Seconds of no-activity for shutting down MAX232

/* More reasonable operatonal timeout (5 min) */
#define RS232TIMEOUT	300	// Seconds of no-activity for shutting down MAX232


/******************************************************************************/
int RS232_ctl(void);
/* @brief 	: Get started with the sequence when reset is comes out of STANDBY
 * @return	: 0 = Not ready, 1 = ready
*******************************************************************************/
void RS232_ctl_reset_timer(void);
/* @brief 	: Reset the no-acitivity shutdown timer
*******************************************************************************/


extern char cRS232_skip_flag;		// Debugging

#endif

