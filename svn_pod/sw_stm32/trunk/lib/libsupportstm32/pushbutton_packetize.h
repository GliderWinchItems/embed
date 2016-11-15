/******************** (C) COPYRIGHT 2011 **************************************
* File Name          : pushbutton_packetize.h
* Hacker	     : deh
* Date First Issued  : 09/06/2011
* Board              : STM32F103VxT6_pod_mm
* Description        : Handle the packeting of the pushbutton
*******************************************************************************/
#ifndef __PUSHBUTTON_PACKETIZE
#define __PUSHBUTTON_PACKETIZE


#define PKT_PUSHBUTTON_ID	ID_PUSHBUTTON	// ID for PUSHBUTTON packets
#define PUSHBUTTONOFFDELAY	7500 		// Switch debounce delay (0.1 millisecond)

#include "../../../../sw_pod/trunk/pod_v1/32KHz_p1.h"

struct PKT_PUSHBUTTON
{
	unsigned char	id;	// Packet ID
	union LL_L_S 	U;					// 64 bit Linux time
};

/******************************************************************************/
struct PKT_PTR  pushbutton_packetize_poll(void);
/* @brief	: Handle the pushbutton
 ******************************************************************************/

/* Pushbutton packeting (@12) */
extern struct PKT_PUSHBUTTON pkt_pushbutton_sd;	// Packet with time stamp for pushbutton
extern int pushbutton_sd_pkt_ctr;		// Ready flag, (increments)


#endif

