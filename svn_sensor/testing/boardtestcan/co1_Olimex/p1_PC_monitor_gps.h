/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : p1_PC_monitor_gps.h
* Author             : deh
* Date First Issued  : 03/25/2013
* Board              : Olimex CO board (USART1)
* Description        : Output current gps to PC
*******************************************************************************/
/*


*/
#ifndef __P1_MONITOR_GPS
#define __P1_MONITOR_GPS

/******************************************************************************/
void p1_PC_monitor_gps(void);
/* @brief	: Output current GPS & tick ct to PC
 ******************************************************************************/

extern u16 monitor_gps_state;
extern u16 k_gps_ctr;


#endif

