/******************** (C) COPYRIGHT 2013 **************************************
* File Name          : packet_print.h
* Writer	     : deh
* Date First Issued  : 06/17/2013
* Board              : any
* Description        : Print svn_sensor packets
*******************************************************************************/
#ifndef __PACKET_PRINT
#define __PACKET_PRINT

#include "packet_extract.h"
#define PODTIMEEPOCH	1318478400	// Offset from Linux epoch to save bits

/*******************************************************************************/
void packet_print(struct PKTP *pp);
/* @brief 	: printf the packet
 * @param	: pblk--pointer (not a zero terminated string)
 * @return	: void
*******************************************************************************/
void packet_print_hex(unsigned char * p);
/* @brief 	: printf the block in hex
 * @param	: p = pointer to packet (not a zero terminated string)
 * @return	: void
*******************************************************************************/
void packet_print_date1(unsigned char * p);
/* @brief 	: printf the block with date/time/tick
 * @param	: p = pointer to packet (not a zero terminated string)
 * @return	: void
*******************************************************************************/
void packet_changelines(unsigned char * p);
/* @brief 	: printf lines of the block where there is a change
 * @param	: p = pointer to packet (not a zero terminated string)
 * @return	: void
*******************************************************************************/


#endif

