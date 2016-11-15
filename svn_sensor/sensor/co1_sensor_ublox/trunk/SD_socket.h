/******************************************************************************
* File Name          : SD_socket.h
* Date First Issued  : 05/10/2014
* Board              : Sensor board w flash genie SD module
* Description        : Setup and handling of socket switches module LED
*******************************************************************************/
/* 

*/

#ifndef __SD_SOCKET
#define __SD_SOCKET

#include "common.h"	// Such as 'u32'
#include "common_can.h"	// CAN ID|mask layout and structs

/******************************************************************************/
int SD_socket_init(void);
/* @brief 	: Setup pins for flash genie socket module connections to sensor board
 * @return	: 0 = OK, not zero for error.
*******************************************************************************/
void SD_socket_setled(int which);
/* @brief 	: Set
 * @param	: which: 0 set RED, not-zero set GREEN
*******************************************************************************/
int SD_socket_sw_status(int sw);
/* @brief 	: Check flash genie socket module switch
 * @param	: sw: 0 = insertion switch; 1 = write protection switch
 * @return	: 0 = switch closed; not zero = switch open; negative = bad 'sw' number
*******************************************************************************/
int SD_socket_sw_status_and_setlet(void);
/* @brief 	: Get status of flash genie socket module switch
 * @return	: 0 = OK (and LED set to GRN); not zero = SD is not inserted (led set to RED)
*******************************************************************************/

#endif 



