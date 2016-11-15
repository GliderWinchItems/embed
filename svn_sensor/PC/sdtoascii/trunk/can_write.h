/*******************************************************************************
* File Name          : can_write.h
* Date First Issued  : 05/20/2014
* Board              : PC
* Description        : Convert and output SD card packets to CAN ascii/hex format msgs
*******************************************************************************/
#ifndef __CAN_WRITE
#define __CAN_WRITE

#include "packet_extract.h"
#include "packet_print.h"
#include <stdio.h>
/*******************************************************************************/
void can_write(FILE* fpOut, struct PKTP *pp);
/* @brief 	: Convert packet to CAN ascii/hex msg
 * @param	: pp--pointer (not a zero terminated string)
 * @return	: void
*******************************************************************************/
void can_write2(FILE* fpOut, struct PKTP *pp);
/* @brief 	: Convert packet to CAN ascii/hex msg
 * @param	: pp--pointer (not a zero terminated string)
 * @return	: void
*******************************************************************************/


#endif

