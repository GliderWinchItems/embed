/******************** (C) COPYRIGHT 2012 **************************************
* File Name          : xb_api_atcmd.c
* Hackee             : deh
* Date First Issued  : 03/25/2012
* Board              : STM32F103VxT6_pod_mm with XBee
* Description        : In API mode send AT command to the module
*******************************************************************************/
#include <string.h>


#include "libusartstm32/usartallproto.h"
#include "libopenstm32/usart.h"
#include "xb_common.h"
#include "xb_api_tx.h"

/* ************************************************************
 * static int xb_api_at(char * q, unsigned char id);
 * @brief	: Prepare frame and send AT cmd, e.g. xb_api_atcmd("DL00000FFF");
 * @argument	: p = pointer to string with command
 * @argument	: id = 0x08 (AT command), 0x09 (queue command)
***************************************************************/
static int xb_api_at(char * q, unsigned char id)
{
#define BUFFSIZE	64
#define FRAMEID		0x00	// Frame id (arbitrary), 0 = no response

	char zz[BUFFSIZE];	// Frame to be sent
	char *p = &zz[0];	// Build frame on local stack	
	unsigned int checksum = (id + FRAMEID);
	int nLen = strlen(q);

	/* Check that length will fit within buffer */
	if ( (nLen >= (BUFFSIZE-5)) && (nLen > 0) )
		return -1;
	
	*p++ = (nLen + 2) >> 8;		// MSB of length
	*p++ = (nLen + 2) & 0xff;	// LSB of length
	*p++ = id;			// API identifier (AT command code)
	*p++ = FRAMEID;			// Frame identifier

	/* Copy command */	
	while (*p != 0) 
	{
		*p++ = *q;	// Copy command
		checksum += *q++;
	}

	/* Place checksum */
	*p++ = (0xff - (checksum & 0xff) );

	return 0;
}
/* ************************************************************
 * int xb_api_atcmd(char * q);
 * @brief	: Prepare frame and send AT cmd, e.g. xb_api_atcmd("DL00000FFF");
 * @argument	: q = pointer to string with command
 * @return	: 0 = OK; -1 = string too long
***************************************************************/
int xb_api_atcmd(char * q)
{
	return	xb_api_at(q,0x08);

}	
/* ************************************************************
 * int xb_api_queue(char * q);
 * @brief	: Prepare frame and queue AT cmd, e.g. xb_api_atcmd("DL00000FFF");
 * @argument	: q = pointer to string with command
 * @return	: 0 = OK; -1 = string too long
***************************************************************/
int xb_api_queue(char * q)
{
	return	xb_api_at(q,0x09);

}	



