/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : RS232_ctl.h
* Author             : deh
* Date First Issued  : 10/21/2011
* Board              : STM32F103VxT6_pod_mm (USART1)
* Description        : Pulse external LED
*******************************************************************************/

#ifndef __LED_CTL
#define __LED_CTL


/******************************************************************************/
void LED_ctl_turnon(unsigned int uiCt, unsigned int uiSpace, int nNum);
/* @param	: uiCt: count of ON time (ms) (limit to about 40,000)
 * @param	: uiSpace: count of space time (ms) between pulses 
 * @param	: uiNum: number of pulses 
 * @brief 	: Configure pin for output, turn LED on, configure back to input
*******************************************************************************/
void LED_ctl(void);
/* @brief 	: Polling loop checks to see if time to turn LED off
*******************************************************************************/
void LEDonboard_ctl_turnon_ct(unsigned int uiCtob, unsigned int uiSpaceob, short sCt);
/* @param	: uiCobt: count of ON time (ms) (limit to about 40,000)
 * @param	: uiSpaceob: count of space time (ms) between pulses 
 * @param	: uiCt: Number of quick flashes (1 = one flash; 2 = two, etc.)
 * @brief 	: Flash onboard LED & LED on interface (not LED on BOX)
*******************************************************************************/
#endif

